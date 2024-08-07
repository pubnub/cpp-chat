#include "message_service.hpp"
#include "chat_service.hpp"
#include "domain/message_entity.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"
#include "nlohmann/json.hpp"
#include "message.hpp"
#include "thread_message.hpp"
#include "message_action.hpp"
#include "message_draft.hpp"
#include "message_draft_config.hpp"
#include "chat_helpers.hpp"
#include "dao/message_dao.hpp"
#include "callback_service.hpp"

using namespace Pubnub;
using json = nlohmann::json;

MessageService::MessageService(ThreadSafePtr<PubNub> pubnub, std::weak_ptr<ChatService> chat_service):
    pubnub(pubnub),
    chat_service(chat_service)
{}

Pubnub::Message MessageService::edit_text(const Pubnub::String& timetoken, const MessageDAO& message_data, const Pubnub::String& new_text) const {
    if(new_text.empty())
    {
        throw std::invalid_argument("Failed to edit text, new_text is empty");
    }

    pubnub_message_action_type edited_action_type = pubnub_message_action_type::PMAT_Edited;
    auto entity = message_data.get_entity();

    //Message action value sent to server has to be in quotation marks
    String new_text_with_quotations = "\"" + new_text + "\"";

    auto action_timetoken = [this, entity, timetoken, edited_action_type, new_text_with_quotations] {
        auto pubnub_handle = pubnub->lock();
        return pubnub_handle->add_message_action(
                entity.channel_id, timetoken, message_action_type_to_string(edited_action_type), new_text_with_quotations);
    }();
    
    //But we store message text without quotations marks
    
    auto new_message_entity = entity.edit_text(new_text, action_timetoken);

    return this->create_message_object(std::make_pair(timetoken, new_message_entity));
}

String MessageService::text(const MessageDAO& message) const {
    return message.get_entity().current_text();
}

Pubnub::Message MessageService::delete_message(const MessageDAO& message, const Pubnub::String& timetoken) const {
    pubnub_message_action_type deleted_action_type = pubnub_message_action_type::PMAT_Deleted;
    String deleted_value = "\"deleted\"";
   
    auto entity = message.get_entity();

    auto action_timetoken = [this, entity, timetoken, deleted_action_type, deleted_value] {
        auto pubnub_handle = pubnub->lock();
        return pubnub_handle->add_message_action(entity.channel_id, timetoken, message_action_type_to_string(deleted_action_type), deleted_value);
    }();

    auto new_message_entity = entity.delete_message(deleted_value, action_timetoken);

    return this->create_message_object(std::make_pair(timetoken, new_message_entity));
}

bool MessageService::deleted(const MessageDAO& message) const {
    return message.get_entity().is_deleted();
}

Message MessageService::get_message(const String& timetoken, const String& channel_id) const {
    auto start_timetoken_int = std::stoull(timetoken.to_std_string()) + 1;
    String start_timetoken = std::to_string(start_timetoken_int);
    auto chat_service_shared = chat_service.lock();
    std::vector<Message> messages = chat_service_shared->channel_service->get_channel_history(channel_id, start_timetoken, timetoken, 1);
    if(messages.size() == 0)
    {
        throw std::runtime_error("can't get message, there is no message with this timestamp");
    }

    return messages[0];
}

std::vector<MessageAction> MessageService::get_message_reactions(const MessageDAO& message) const {
    return message.get_entity().get_reactions();
}

Message MessageService::toggle_reaction(const Pubnub::String& timetoken, const MessageDAO& message, const String& reaction) const {
    auto current_reactions = this->get_message_reactions(message);
    auto pubnub_handle = pubnub->lock();
    String current_user_id = pubnub_handle->get_user_id();
    pubnub_message_action_type action_type = pubnub_message_action_type::PMAT_Reaction;

    auto message_data = message.get_entity();
    auto timetoken_to_remove = message_data.get_user_reaction_timetoken(current_user_id, reaction);

    if (timetoken_to_remove.has_value()) {
        pubnub_handle->remove_message_action(message_data.channel_id, timetoken, timetoken_to_remove.value());
        auto new_entity = message_data.remove_user_reaction(timetoken_to_remove.value());

        return this->create_message_object(std::make_pair(timetoken, new_entity));
    } 

    String action_timetoken = pubnub_handle->add_message_action(
            message_data.channel_id, timetoken, message_action_type_to_string(action_type), reaction);
    auto new_entity = message_data.add_user_reaction(current_user_id, reaction, action_timetoken);

    return this->create_message_object(std::make_pair(timetoken, new_entity));
}

void MessageService::forward_message(const Message& message, const String& channel_id) const { 
    if(channel_id.empty())
    {
        throw std::invalid_argument("can't forward message, channel_id is empty");
    }


    String current_metadata = message.message_data().meta.empty() ? "{}" : message.message_data().meta;

    json meta_json = json::parse(current_metadata);

    // TODO: domain or not domain
    meta_json["originalPublisher"] = message.message_data().user_id;
    meta_json["originalChannelId"] = message.message_data().channel_id;

    auto pubnub_handle = pubnub->lock();
    pubnub_handle->publish(channel_id, chat_message_to_publish_string(message.text(), message.type()), meta_json.dump());
}

MessageDraft MessageService::create_message_draft(const Channel& channel, const MessageDraftConfig& message_draft_config) const
{
    return MessageDraft(channel, message_draft_config, shared_from_this());
}


std::function<void()> MessageService::stream_updates(Pubnub::Message calling_message, std::function<void(const Pubnub::Message)> message_callback) const
{

    auto pubnub_handle = this->pubnub->lock();

    auto chat = this->chat_service.lock();

    std::vector<String> messages_ids;
    std::function<void(Message)> final_message_callback = [=](Message message){
        MessageEntity calling_message_entity = MessageDAO(calling_message.message_data()).to_entity();
        MessageEntity message_entity = MessageDAO(message.message_data()).to_entity();
        std::pair<String, MessageEntity> pair = std::make_pair(message.timetoken(), MessageEntity::from_base_and_updated_message(calling_message_entity, message_entity));
        auto updated_message = create_message_object(pair);
        
        message_callback(updated_message);
    };
    
    auto messages = pubnub_handle->subscribe_to_multiple_channels_and_get_messages({calling_message.timetoken()});
    chat->callback_service->broadcast_messages(messages);
    chat->callback_service->register_message_callback(calling_message.timetoken(), final_message_callback);

    //stop streaming callback
    std::function<void()> stop_streaming = [=](){
        chat->callback_service->remove_message_callback(calling_message.timetoken());
    };

    return stop_streaming;

}

std::function<void()> MessageService::stream_updates_on(Pubnub::Message calling_message, const std::vector<Pubnub::Message>& messages, std::function<void(std::vector<Pubnub::Message>)> message_callback) const
{
    if(messages.empty())
    {
        throw std::invalid_argument("Cannot stream message updates on an empty list");
    }

    auto pubnub_handle = this->pubnub->lock();

    auto chat = this->chat_service.lock();
    std::vector<String> messages_ids;

    std::function<void(Message)> single_message_callback = [=](Message message){
        
        std::vector<Pubnub::Message> updated_messages; 

        for(int i = 0; i < messages.size(); i++)
        {
            //Find message that was updated and replace it in Entity stream messages
            auto stream_message = messages[i];

            if(stream_message.timetoken() == message.timetoken())
            {
                MessageEntity stream_message_entity = MessageDAO(stream_message.message_data()).to_entity();
                MessageEntity message_entity = MessageDAO(message.message_data()).to_entity();
                std::pair<String, MessageEntity> pair = std::make_pair(message.timetoken(), MessageEntity::from_base_and_updated_message(stream_message_entity, message_entity));
                auto updated_message = create_message_object(pair);
                updated_messages.push_back(updated_message);
            }
            else
            {
                updated_messages.push_back(messages[i]);
            }
        }
        message_callback(updated_messages);

    };
    
    for(auto message : messages)
    {
        messages_ids.push_back(message.timetoken());
        chat->callback_service->register_message_callback(message.timetoken(), single_message_callback);
    }
    

    auto subscribe_messages = pubnub_handle->subscribe_to_multiple_channels_and_get_messages(messages_ids);
    chat->callback_service->broadcast_messages(subscribe_messages);

    //stop streaming callback
    std::function<void()> stop_streaming = [=, &messages_ids](){
        for(auto id : messages_ids)
        {
            chat->callback_service->remove_message_callback(id);
        }
    };

    return stop_streaming;
}

Message MessageService::create_message_object(std::pair<String, MessageEntity> message_data) const {
    if (auto chat = this->chat_service.lock()) {
        return Pubnub::Message(
                message_data.first,
                chat,
                shared_from_this(),
                chat->channel_service,
                chat->restrictions_service,
                std::make_unique<MessageDAO>(message_data.second)
        );
    }

    throw std::runtime_error("Can't create message, chat service pointer is invalid");
}


Pubnub::ThreadMessage MessageService::create_thread_message_object(std::pair<Pubnub::String, MessageEntity> message_data, Pubnub::String parent_channel_id) const
{
    return ThreadMessage(create_message_object(message_data), parent_channel_id);
}

Pubnub::ThreadMessage MessageService::create_thread_message_object(const Pubnub::Message& base_message, const Pubnub::String& parent_channel_id) const
{
    return Pubnub::ThreadMessage(base_message, parent_channel_id);
}

