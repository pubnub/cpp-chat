#include "message_service.hpp"
#include "application/subscription.hpp"
#include "chat_service.hpp"
#include "enum_converters.hpp"
#include "domain/message_entity.hpp"
#include "domain/quotes.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"
#include "nlohmann/json.hpp"
#include "message.hpp"
#include "thread_message.hpp"
#include "draft_service.hpp"
#include "message_action.hpp"
#include "message_draft.hpp"
#include "message_draft_config.hpp"
#include "chat_helpers.hpp"
#include "dao/message_dao.hpp"
#include "callback_service.hpp"
#include <memory>
#include <string>
#include <sstream>

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

    auto new_message_entity = entity.delete_message(Quotes::remove(deleted_value), Quotes::remove(action_timetoken));

    return this->create_message_object(std::make_pair(timetoken, new_message_entity));
}

bool MessageService::delete_message_hard(const Pubnub::Message& message) const {

    if(message.has_thread())
    {
        message.get_thread().delete_channel();
    }

    auto start_timetoken_int = std::stoull(message.timetoken().to_std_string()) + 1;
    String start_timetoken = std::to_string(start_timetoken_int);

    bool message_deleted = [this, message, start_timetoken] {
        auto pubnub_handle = pubnub->lock();
        return pubnub_handle->delete_messages(message.message_data().channel_id, start_timetoken, message.timetoken());
    }();

    return message_deleted;
}

bool MessageService::deleted(const MessageDAO& message) const {
    return message.get_entity().is_deleted();
}

Pubnub::Message MessageService::restore(const MessageDAO& message, const Pubnub::String& timetoken) const
{
    auto entity = message.to_entity();
    if(!entity.is_deleted())
    {
        throw std::invalid_argument("This message has not been deleted");
    }

    std::vector<MessageAction> new_message_actions;
    {
        auto pubnub_handle = pubnub->lock();
        for(auto action : entity.message_actions)
        {
            if(action.type == PMAT_Deleted)
            {
                String timetoken_no_quotes = Quotes::remove(action.timetoken);
                pubnub_handle->remove_message_action(entity.channel_id, timetoken, timetoken_no_quotes);
            }
            else
            {
                new_message_actions.push_back(action);
            }
        }
    }

    entity.message_actions = new_message_actions;

    return this->create_message_object(std::make_pair(timetoken, entity));
}

Message MessageService::get_message(const String& timetoken, const String& channel_id) const {
    if(timetoken.empty())
    {
        throw std::invalid_argument("Can't get message, timetoken is empty");
    }

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
    auto new_entity = message_data.add_user_reaction(current_user_id, reaction, Quotes::remove(action_timetoken));

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
    // Shouldn't fail as it can be run only if the Channel is valid
    return MessageDraft(channel, message_draft_config, chat_service.lock()->channel_service, chat_service.lock()->user_service);
}


std::shared_ptr<Subscription> MessageService::stream_updates(Pubnub::Message calling_message, std::function<void(const Pubnub::Message)> message_callback) const
{
    auto subscription = this->pubnub->lock()->subscribe(calling_message.message_data().channel_id);

    auto callback_service = this->chat_service.lock()->callback_service;
    subscription->add_message_update_listener(
            callback_service->to_c_message_update_callback(calling_message, this->shared_from_this(), message_callback));

    return subscription;
}


std::shared_ptr<Subscription> MessageService::stream_updates(Pubnub::ThreadMessage calling_message, std::function<void(const Pubnub::ThreadMessage)> message_callback) const
{
    auto subscription = this->pubnub->lock()->subscribe(calling_message.message_data().channel_id);

    auto callback_service = this->chat_service.lock()->callback_service;
    subscription->add_thread_message_update_listener(
            callback_service->to_c_thread_message_update_callback(calling_message, this->shared_from_this(), message_callback));

    return subscription;
}

std::shared_ptr<SubscriptionSet> MessageService::stream_updates_on(Pubnub::Message calling_message, const std::vector<Pubnub::Message>& messages, std::function<void(std::vector<Pubnub::Message>)> message_callback) const
{
    if(messages.empty())
    {
        throw std::invalid_argument("Cannot stream message updates on an empty list");
    }

    std::vector<Pubnub::String> channels_ids;

    std::transform(
            messages.begin(),
            messages.end(),
            std::back_inserter(channels_ids),
            [](const Pubnub::Message& message){
                return message.message_data().channel_id;
            });

    auto subscription = this->pubnub->lock()->subscribe_multiple(channels_ids);

    auto callback_service = this->chat_service.lock()->callback_service;
    subscription->add_thread_message_update_listener(
            callback_service->to_c_messages_updates_callback(messages, shared_from_this(), message_callback));

    return subscription;
}

std::shared_ptr<SubscriptionSet> MessageService::stream_updates_on(Pubnub::ThreadMessage calling_message, const std::vector<Pubnub::ThreadMessage>& messages, std::function<void(std::vector<Pubnub::ThreadMessage>)> message_callback) const
{
    if(messages.empty())
    {
        throw std::invalid_argument("Cannot stream message updates on an empty list");
    }

    std::vector<Pubnub::String> channels_ids;

    std::transform(
            messages.begin(),
            messages.end(),
            std::back_inserter(channels_ids),
            [](const Pubnub::ThreadMessage& message){
                return message.message_data().channel_id;
            });

    auto subscription = this->pubnub->lock()->subscribe_multiple(channels_ids);

    auto callback_service = this->chat_service.lock()->callback_service;
    subscription->add_thread_message_update_listener(
            callback_service->to_c_thread_messages_updates_callback(messages, shared_from_this(), message_callback));

    return subscription;
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

Pubnub::String MessageService::get_phrase_to_look_for(const Pubnub::String& look_text) const
{
    std::string text = look_text.to_std_string();
    size_t lastAtIndex = text.find_last_of('@');
    
    // If there is no '@' or if there are fewer than 3 characters after the last '@'
    if (lastAtIndex == std::string::npos || text.size() - lastAtIndex - 1 < 3) {
        return "";
    }

    // Get the substring after the last '@'
    std::string charactersAfterAt = text.substr(lastAtIndex + 1);

    // Split the substring by spaces
    std::istringstream iss(charactersAfterAt);
    std::vector<std::string> splitWords;
    std::string word;
    while (iss >> word) {
        splitWords.push_back(word);
    }

    // If there are more than 2 words after the '@', return null
    if (splitWords.size() > 2) {
        return "";
    }

    // Combine the first two words (if they exist) with a space in between
    std::string result = splitWords[0];
    if (splitWords.size() > 1) {
        result += " " + splitWords[1];
    }

    return String(result);
}

Pubnub::String MessageService::get_channel_phrase_to_look_for(const Pubnub::String& look_text) const
{
    std::string text = look_text.to_std_string();
    size_t lastAtIndex = text.find_last_of('#');
    
    // If there is no '@' or if there are fewer than 3 characters after the last '#'
    if (lastAtIndex == std::string::npos || text.size() - lastAtIndex - 1 < 3) {
        return "";
    }

    // Get the substring after the last '#'
    std::string charactersAfterAt = text.substr(lastAtIndex + 1);

    // Split the substring by spaces
    std::istringstream iss(charactersAfterAt);
    std::vector<std::string> splitWords;
    std::string word;
    while (iss >> word) {
        splitWords.push_back(word);
    }

    // If there are more than 2 words after the '#', return null
    if (splitWords.size() > 2) {
        return "";
    }

    // Combine the first two words (if they exist) with a space in between
    std::string result = splitWords[0];
    if (splitWords.size() > 1) {
        result += " " + splitWords[1];
    }

    return String(result);
}

Pubnub::Message MessageService::update_message_with_base(const Pubnub::Message& message, const Pubnub::Message& base_message) const {
    MessageEntity base_entity = MessageDAO(base_message.message_data()).to_entity();
    MessageEntity message_entity = MessageDAO(message.message_data()).to_entity();

    return this->create_message_object(
            {message.timetoken(), MessageEntity::from_base_and_updated_message(base_entity, message_entity)});

}

Pubnub::ThreadMessage MessageService::update_thread_message_with_base(const Pubnub::ThreadMessage& message, const Pubnub::ThreadMessage& base_message) const {
    MessageEntity base_entity = MessageDAO(base_message.message_data()).to_entity();
    MessageEntity message_entity = MessageDAO(message.message_data()).to_entity();

    return this->create_thread_message_object(
            {message.timetoken(), MessageEntity::from_base_and_updated_message(base_entity, message_entity)},
            base_message.parent_channel_id());
}
