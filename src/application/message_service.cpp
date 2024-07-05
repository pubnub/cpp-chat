#include "message_service.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"
#include "nlohmann/json.hpp"
#include "presentation/message.hpp"

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

MessageEntity MessageService::create_domain_from_presentation_data(String timetoken, ChatMessageData &presentation_data)
{
    MessageEntity new_message_entity;
    new_message_entity.timetoken = timetoken;
    new_message_entity.type = presentation_data.type;
    new_message_entity.text = presentation_data.text;
    new_message_entity.channel_id = presentation_data.channel_id;
    new_message_entity.user_id = presentation_data.user_id;
    new_message_entity.meta = presentation_data.meta;
    new_message_entity.message_actions = presentation_data.message_actions;

    return new_message_entity;
}

// TODO: this is domain...
MessageEntity MessageService::create_domain_from_message_json(String message_json, String channel_id)
{
    json message_data_json = json::parse(message_json);;

    if(message_data_json.is_null())
    {
        throw std::runtime_error("Failed to construct message, message_json is invalid Json");
    }

    if(!message_data_json.contains("timetoken") && !message_data_json["timetoken"].is_null())
    {
        throw std::runtime_error("Failed to construct message, message_json doesn't have  timetoken field");
    }

    MessageEntity new_message_entity;

    new_message_entity.timetoken = message_data_json["timetoken"].dump();
    new_message_entity.timetoken.erase(0, 1);
    new_message_entity.timetoken.erase(new_message_entity.timetoken.length() - 1, 1);

    new_message_entity.channel_id = channel_id;

    if(message_data_json.contains("message") && !message_data_json["message"].is_null())
    {
        new_message_entity.text = message_data_json["message"]["text"].dump();
        new_message_entity.type = chat_message_type_from_string(message_data_json["message"]["type"].dump());
    }
    if(message_data_json.contains("actions") && !message_data_json["actions"].is_null())
    {
        json message_action_types_json = message_data_json["actions"];
        for (json::iterator message_action_type = message_action_types_json.begin(); message_action_type != message_action_types_json.end(); ++message_action_type) 
        {
            json message_actions_json = message_action_type.value();
            String message_actions_json_string = message_actions_json.dump();
            for (json::iterator message_action = message_actions_json.begin(); message_action != message_actions_json.end(); ++message_action) 
            {
                json single_message_action_json = message_action.value();
                String single_message_action_json_string = single_message_action_json.dump();
                MessageAction message_action_data;
                message_action_data.type = message_action_type_from_string(message_action_type.key());
                message_action_data.value = message_action.key();
                message_action_data.timetoken = single_message_action_json[0]["actionTimetoken"].dump();
                message_action_data.user_id = single_message_action_json[0]["uuid"].dump();
                new_message_entity.message_actions.push_back(message_action_data);
            }
        }
    }

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

Pubnub::Message MessageService::create_message_object(std::pair<Pubnub::String, MessageEntity> message_data)
{
    if (auto chat = this->chat_service.lock()) {
        this->entity_repository
            ->get_message_entities()
            .update_or_insert(message_data.first, message_data.second);

        return Pubnub::Message(message_data.first, chat, shared_from_this());
    }

    throw std::runtime_error("Can't create message, chat service pointer is invalid");
}
