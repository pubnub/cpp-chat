#include "callback_service.hpp"
#include "infra/serialization.hpp"
#include "infra/sync.hpp"
#include "nlohmann/json.hpp"
#include "domain/parsers.hpp"
#include "presentation/message.hpp"

using json = nlohmann::json;

CallbackService::CallbackService(ThreadSafePtr<PubNub> pubnub, milliseconds wait_interval) :
    pubnub(pubnub),
    thread_run_flag(true) 
{
    this->callback_thread = std::thread([this, wait_interval]() {
        while (this->thread_run_flag.load()) {
            this->resolve_callbacks();
            // TODO: should we sleep in tests?
            std::this_thread::sleep_for(wait_interval);
        }
    });
}

void CallbackService::resolve_callbacks() {
    auto messages = [this]{
        auto guard = this->pubnub->lock();
        return guard->fetch_messages();
    }();

    for (auto message : messages) {
    }
}

void CallbackService::broadcast_callbacks_from_message(pubnub_v2_message message)
{
    if(!message.payload.ptr || !message.channel.ptr)
    {
        throw std::runtime_error("received message is invalid");
    }

    Pubnub::String message_string = Pubnub::String(message.payload.ptr, message.payload.size);
    Pubnub::String message_channel_string = Pubnub::String(message.channel.ptr, message.channel.size);

    if(message_string.empty())
    {
        throw std::runtime_error("message is empty");
    }

//    json message_json = json::parse(message_string);
//    
//    if(message_json.is_null())
//    {
//        throw std::runtime_error("Failed to parse message into json");
//    }

    if (Parsers::PubnubJson::is_message(message_string)) {
        auto maybe_callback = this->callbacks.get_message_callbacks().get(message_channel_string);
        if (maybe_callback.has_value()) {
            auto parsed_message = Parsers::PubnubJson::to_message(message);
            if (auto service = this->message_service.lock()) {
                auto message = service->create_message(parsed_message);
                maybe_callback.value()(message);
            }
        }
    }
    
//    //Handle channel updates
//    if(Deserialization::is_channel_update_message(message_string))
//    {
//        if(this->channel_callbacks_map.find(message_channel_string) != this->channel_callbacks_map.end())
//        {
//            this->channel_callbacks_map[message_channel_string](
//                    Deserialization::pubnub_message_to_chat_channel(this->chat_obj, message));
//        }
//    }
//
//    //Handle user updates
//    if(Deserialization::is_user_update_message(message_string))
//    {
//        if(this->user_callbacks_map.find(message_channel_string) != this->user_callbacks_map.end())
//        {
//            this->user_callbacks_map[message_channel_string](
//                    Deserialization::pubnub_message_to_chat_user(this->chat_obj, message));
//        }
//    }
//
//    //Handle events
//    if(Deserialization::is_event_message(message_string))
//    {
//        if(this->event_callbacks_map.find(message_channel_string) != this->event_callbacks_map.end())
//        {
//            //Get event type from callback
//            Pubnub::pubnub_chat_event_type event_type;
//            std::function<void(Pubnub::String)> callback;
//            std::tie(event_type, callback) = this->event_callbacks_map[message_channel_string];
//
//            //only send callback if event types ara matching
//            if(Pubnub::chat_event_type_from_string(message_json["type"].dump()) == event_type)
//            {
//                callback(message_string);
//            }
//        }
//    }
//
//    //Handle presence
//    if(Deserialization::is_presence_message(message_string))
//    {
//        //get channel name without -pnpres as all presence messages are on channels with -pnpres
//        Pubnub::String normal_channel_name = message_channel_string;
//        normal_channel_name.erase(message_channel_string.length() - 7, 7);
//
//        if(this->channel_presence_callbacks_map.find(normal_channel_name) != this->channel_presence_callbacks_map.end())
//        {
//            std::vector<Pubnub::String> current_users = chat_obj.who_is_present(normal_channel_name);
//            this->channel_presence_callbacks_map[message_channel_string](current_users);
//        }
//    }
//
//    //Handle message updates
//    if(Deserialization::is_message_update_message(message_string))
//    {
//        Pubnub::String message_timetoken = message_json["data"]["messageTimetoken"].dump();
//        if (message_timetoken.front() == '"' && message_timetoken.back() == '"')
//        {
//            message_timetoken.erase(0, 1);
//            message_timetoken.erase(message_timetoken.length() - 1, 1);
//        }
//
//        if(this->message_update_callbacks_map.find(message_timetoken) != this->message_update_callbacks_map.end())
//        {
//            Pubnub::String message_channel;
//            std::function<void(Pubnub::Message)> callback;
//            std::tie(message_channel, callback) = this->message_update_callbacks_map[message_timetoken];
//            // TODO: this should already give message with this new update, make sure it really does.pubnub.cpp
//            Pubnub::Message message_obj = chat_obj.get_channel(message_channel).get_message(message_timetoken);
//            callback(message_obj);
//        }
//    }
//
//    //Handle message updates
//    if(Deserialization::is_membership_update_message(message_string))
//    {
//        // TODO: All dump() calls should be replaced with unified function that removes quotes from the string
//        Pubnub::String dumped = message_json["data"]["channel"]["id"].dump();
//        Pubnub::String membership_channel = Pubnub::String(&dumped.c_str()[1], dumped.length() - 2); 
//
//        if(this->membership_callbacks_map.find(membership_channel) != this->membership_callbacks_map.end())
//        {
//            Pubnub::String membership_user;
//            std::function<void(Pubnub::Membership)> callback;
//            std::tie(membership_user, callback) = this->membership_callbacks_map[membership_channel];
//
//            //Make sure this message is related to the user that we are streaming updates for
//            Pubnub::String user_from_message = message_json["data"]["uuid"]["id"].dump();
//            Pubnub::String user_from_message_cleaned = Pubnub::String(&user_from_message.c_str()[1], user_from_message.length() - 2);
//            if(user_from_message_cleaned == membership_user)
//            {
//                auto custom_field = Pubnub::String(message_json["custom"].dump());
//                auto custom_field_cleaned = Pubnub::String(&custom_field.c_str()[1], custom_field.length() - 2);
//                Pubnub::Membership membership_obj = Pubnub::Membership(chat_obj, chat_obj.get_channel(membership_channel), chat_obj.get_user(membership_user), custom_field_cleaned);
//                callback(membership_obj);
//            }
//        }
//    }
}


