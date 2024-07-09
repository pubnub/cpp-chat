#include "message_service.hpp"
#include "application/chat_service.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"
#include "nlohmann/json.hpp"
#include "presentation/message.hpp"
#include "presentation/message_action.hpp"
#include "presentation/message_draft.hpp"
#include "presentation/message_draft_config.hpp"
#include "chat_helpers.hpp"

using namespace Pubnub;
using json = nlohmann::json;

MessageService::MessageService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository, std::weak_ptr<ChatService> chat_service):
    pubnub(pubnub),
    entity_repository(entity_repository),
    chat_service(chat_service)
{}

std::vector<Message> MessageService::get_channel_history(Pubnub::String channel_id, String start_timetoken, String end_timetoken, int count)
{
    auto pubnub_handle = this->pubnub->lock();

    String fetch_history_response = pubnub_handle->fetch_history(channel_id, start_timetoken, end_timetoken, count);

    json response_json = json::parse(fetch_history_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get history, response is incorrect");
    }

    json messages_array_json = response_json["channels"][channel_id];


    std::vector<Message> messages;

    for (auto& element : messages_array_json)
    {
        MessageEntity new_message_entity = create_domain_from_message_json(String(element.dump()), channel_id);

        messages.push_back(
                this->create_message_object(std::make_pair(String(element["id"]), new_message_entity))
        );
    }

    return messages;
}

Message MessageService::get_message(String timetoken, Pubnub::String channel_id)
{
    auto start_timetoken_int = std::stoull(timetoken.to_std_string()) + 1;
    String start_timetoken = std::to_string(start_timetoken_int);
    std::vector<Message> messages = this->get_channel_history(channel_id, start_timetoken, timetoken, 1);
    if(messages.size() == 0)
    {
        throw std::runtime_error("can't get message, there is no message with this timestamp");
    }

    return messages[0];
}

std::vector<MessageAction> MessageService::get_message_reactions(Message message)
{
    std::vector<MessageAction> message_actions;
    for(auto message_action : message.message_data().message_actions)
    {
        if(message_action.type == pubnub_message_action_type::PMAT_Reaction)
        {
            message_actions.push_back(message_action);
        }
    }
    return message_actions;
}

void MessageService::toggle_reaction(Message message, String reaction)
{
    auto current_reactions = this->get_message_reactions(message);
    auto pubnub_handle = pubnub->lock();
    String current_user_id = pubnub_handle->get_user_id();
    pubnub_message_action_type action_type = pubnub_message_action_type::PMAT_Reaction;

    ChatMessageData final_message_data = message.message_data();

    bool reaction_removed = false;
    //If this user already gave reaction to this message, remove it firstly
    for(int i = 0; i < final_message_data.message_actions.size(); i++)
    {
        MessageAction current_meesage_action = final_message_data.message_actions[i];
        if(current_meesage_action.type == action_type && current_meesage_action.user_id == current_user_id)
        {
            pubnub_handle->remove_message_action(message.message_data().channel_id, message.timetoken(), current_meesage_action.timetoken);
            final_message_data.message_actions.erase(final_message_data.message_actions.begin() + i);
            reaction_removed = true;
            break;
        }
    }

    //It reaction wasn't removed, add reaction as new message action
    if(!reaction_removed)
    {
        String action_timetoken = pubnub_handle->add_message_action(message.message_data().channel_id, message.timetoken(), message_action_type_to_string(action_type), reaction);
        MessageAction new_message_action = {action_type, reaction, action_timetoken, current_user_id};
        final_message_data.message_actions.push_back(new_message_action);
    }

    //Update entity repository with updated message
    entity_repository->get_message_entities().update_or_insert(message.timetoken(), create_domain_from_presentation_data(message.timetoken(), final_message_data));
}

void MessageService::forward_message(Message message, String channel_id)
{  
    
    if(channel_id.empty())
    {
        throw std::invalid_argument("can't forward message, channel_id is empty");
    }

    auto pubnub_handle = pubnub->lock();

    String current_metadata = message.message_data().meta.empty() ? "{}" : message.message_data().meta;

    json meta_json = json::parse(current_metadata);

    meta_json["originalPublisher"] = message.message_data().user_id;
    meta_json["originalChannelId"] = message.message_data().channel_id;

    pubnub_handle->publish(channel_id, chat_message_to_publish_string(message.text(), message.type()), meta_json.dump());
}

MessageDraft MessageService::create_message_draft(Channel channel, MessageDraftConfig message_draft_config)
{
    return MessageDraft(channel, message_draft_config, shared_from_this());
}

void MessageService::stream_updates_on(std::vector<Message> messages, std::function<void(Message)> message_callback)
{
    if(messages.empty())
    {
        throw std::invalid_argument("Cannot stream message updates on an empty list");
    }

    auto pubnub_handle = pubnub->lock();

    for(auto message : messages)
    {
        //TODO:: CALLBACK register message update callback here
        pubnub_handle->subscribe_to_channel(message.message_data().channel_id);
    }
}

Message MessageService::create_message_object(std::pair<String, MessageEntity> message_data)
{
    if (auto chat = this->chat_service.lock()) {
        this->entity_repository
            ->get_message_entities()
            .update_or_insert(message_data.first, message_data.second);

        return create_presentation_object(message_data.first);
    }

    throw std::runtime_error("Can't create message, chat service pointer is invalid");
}

Message MessageService::create_presentation_object(String timetoken)
{
    if(auto chat_service_shared = this->chat_service.lock())
    {
        return Message(timetoken, chat_service_shared, shared_from_this(), chat_service_shared->channel_service, chat_service_shared->restrictions_service);
    }
    
    throw std::runtime_error("Can't create message, chat service pointer is invalid");
}

MessageEntity MessageService::create_domain_from_presentation_data(String timetoken, ChatMessageData &presentation_data)
{
    MessageEntity new_message_entity;
    new_message_entity.type = presentation_data.type;
    new_message_entity.text = presentation_data.text;
    new_message_entity.channel_id = presentation_data.channel_id;
    new_message_entity.user_id = presentation_data.user_id;
    new_message_entity.meta = presentation_data.meta;
    new_message_entity.message_actions = presentation_data.message_actions;

    return new_message_entity;
}

ChatMessageData MessageService::presentation_data_from_domain(MessageEntity &message_entity)
{
    ChatMessageData message_data;
    message_data.type = message_entity.type;
    message_data.text = message_entity.text;
    message_data.channel_id = message_entity.channel_id;
    message_data.user_id = message_entity.user_id;
    message_data.meta = message_entity.meta;
    message_data.message_actions = message_entity.message_actions;

    return message_data;
}

