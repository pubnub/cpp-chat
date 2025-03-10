#include "infra/pubnub.hpp"
#include "infra/serialization.hpp"
#include "chat.hpp"
//#include "message.hpp"
//#include "channel.hpp"
//#include "user.hpp"
//#include "membership.hpp"
#include "nlohmann/json.hpp"
#include <thread>
#include <vector>
#include <iostream>
extern "C" {
#include <pubnub_coreapi_ex.h>
#include <pubnub_alloc.h>
#include <pubnub_pubsubapi.h>
#include <pubnub_ntf_sync.h>
#include <pubnub_helper.h>
#include <pubnub_objects_api.h>
#include <pubnub_blocking_io.h>
#include <pubnub_coreapi.h>
#include <pubnub_api_types.h>
#include <pubnub_subscribe_v2.h>
#include <pubnub_fetch_history.h>
#include <pubnub_actions_api.h>
#include <pubnub_advanced_history.h>
#include <pubnub_grant_token_api.h>
#include <pubnub_crypto.h>
}

using json = nlohmann::json;

PubNub::PubNub(const Pubnub::String publish_key, const Pubnub::String subscribe_key, const Pubnub::String user_id, const Pubnub::String auth_key):
    publish_key(publish_key),
    subscribe_key(subscribe_key),
    user_id(user_id),
    auth_key(auth_key),
    main_context(pubnub_alloc(), pubnub_free),
    long_poll_context(pubnub_alloc(), pubnub_free)
{
    pubnub_init(this->main_context.get(), this->publish_key.c_str(), this->subscribe_key.c_str());
    pubnub_init(this->long_poll_context.get(), this->publish_key.c_str(), this->subscribe_key.c_str());

    pubnub_set_user_id(this->main_context.get(), this->user_id.c_str());
    pubnub_set_user_id(this->long_poll_context.get(), this->user_id.c_str());

    pubnub_set_blocking_io(this->main_context.get());
    pubnub_set_non_blocking_io(this->long_poll_context.get());

    if (!auth_key.empty()) {
        pubnub_set_auth_token(this->main_context.get(), this->auth_key.c_str());
        pubnub_set_auth_token(this->long_poll_context.get(), this->auth_key.c_str());
    }
}

Pubnub::String PubNub::publish(const Pubnub::String channel, const Pubnub::String message, const Pubnub::String metadata, const bool store_in_history, const bool send_by_post)
{
    auto publish_options = pubnub_publish_defopts();
    publish_options.store = store_in_history;
    publish_options.method = send_by_post ? pubnubSendViaPOST : pubnubSendViaGET;
    metadata.empty() ?  publish_options.meta = NULL : publish_options.meta = metadata;

    auto result = pubnub_publish_ex(main_context.get(), channel.c_str(), message.c_str(), publish_options);

    this->await_and_handle_error(result);

    // TODO: this is hacky as fuck...
    //       We should add the posibility to retrieve timetoken from c-core in proper way...
    return Pubnub::String(&pubnub_last_publish_result(main_context.get())[8], 17);
}

Pubnub::String PubNub::signal(const Pubnub::String channel, const Pubnub::String message)
{
    auto result = pubnub_signal(main_context.get(), channel.c_str(), message.c_str());

    this->await_and_handle_error(result);

    // TODO: this is hacky as fuck...
    //       We should add the posibility to retrieve timetoken from c-core in proper way...
    return Pubnub::String(&pubnub_last_publish_result(main_context.get())[8], 17);
}

std::vector<pubnub_v2_message> PubNub::subscribe_to_channel_and_get_messages(const Pubnub::String channel)
{
    return this->subscribe_to_multiple_channels_and_get_messages({channel});
}

std::vector<pubnub_v2_message> PubNub::subscribe_to_multiple_channels_and_get_messages(const std::vector<Pubnub::String> channels)
{
    bool new_channels = false;

    for (const auto& channel : channels) {
        if (!this->is_subscribed_to_channel(channel)) {
            this->subscribed_channels.push_back(channel);
            new_channels = true;
        }
    }

    if (!new_channels) {
        return {};
    }

    std::vector<pubnub_v2_message> messages;

    if (this->is_subscribed) {
        messages = this->pause_subscription_and_get_messages();
    }

    this->call_handshake();
    this->is_subscribed = true;
    this->call_subscribe();

    return messages;
}

// TODO: s that even needed?
std::vector<Pubnub::String> PubNub::subscribe_to_channel_and_get_messages_as_strings(const Pubnub::String channel)
{
    std::vector<pubnub_v2_message> pubnub_messages = this->subscribe_to_channel_and_get_messages(channel);

    std::vector<Pubnub::String> messages;
    for(pubnub_v2_message& message : pubnub_messages)
    {
        messages.push_back(Deserialization::pubnub_message_to_string(message));
    }

    return messages;
}

std::vector<pubnub_v2_message> PubNub::fetch_messages()
{
    if (!this->is_subscribed) {
        return {};
    }

    auto result = pubnub_last_result(this->long_poll_context.get());
    if (PNR_OK != result && PNR_STARTED != result && PNR_TIMEOUT != result) {
        throw std::runtime_error(
                std::string("Failed to fetch messages: ")
                + pubnub_res_2_string(result)
                );
    }

    if (PNR_STARTED == result || PNR_TIMEOUT == result) {
        if (PNR_TIMEOUT == result) {
            this->call_subscribe();
        }
        return {};
    }

    std::vector<pubnub_v2_message> messages;
    
    for (
            pubnub_v2_message message = pubnub_get_v2(this->long_poll_context.get());
            message.payload.ptr != NULL;
            message = pubnub_get_v2(this->long_poll_context.get())
        ) {
        messages.push_back(message);
   };
    
    this->call_subscribe();

    return messages;
}

// TODO: s that even needed?
std::vector<Pubnub::String> PubNub::fetch_messages_as_strings()
{
    std::vector<pubnub_v2_message> pubnub_messages = this->fetch_messages();

    std::vector<Pubnub::String> messages;
    for(pubnub_v2_message& message : pubnub_messages)
    {
        messages.push_back(Deserialization::pubnub_message_to_string(message));
    }

    return messages;
}

std::vector<pubnub_v2_message> PubNub::pause_subscription_and_get_messages()
{
    if (this->subscribed_channels.empty() || !this->is_subscribed) {
        return {};
    }

    this->is_subscribed = false;
    this->cancel_previous_subscription();

    std::vector<pubnub_v2_message> messages;

    for (
            pubnub_v2_message message = pubnub_get_v2(this->long_poll_context.get());
            message.payload.ptr != NULL;
            message = pubnub_get_v2(this->long_poll_context.get())
        ) {
        messages.push_back(message);
    };

    return messages;
}

std::vector<pubnub_v2_message> PubNub::unsubscribe_from_channel_and_get_messages(Pubnub::String channel) {
    auto messages = this->pause_subscription_and_get_messages();
    std::vector<Pubnub::String> new_channels;
    for (auto& channel : this->subscribed_channels)
    {
        if (channel != channel)
        {
            new_channels.push_back(channel);
        }
    }

    this->subscribed_channels = new_channels;

    //This didn't work, maybe because of StringParser
    //this->subscribed_channels.erase(
    //        std::remove(
    //            this->subscribed_channels.begin(),
    //            this->subscribed_channels.end(),
    //            channel
    //        ),
    //        this->subscribed_channels.end()
    //    );

    if (this->subscribed_channels.empty()) {
        this->is_subscribed = false;

        return messages;
    }

    this->call_handshake();
    this->call_subscribe();

    return messages;
}

void PubNub::resume_subscription()
{
    if (this->is_subscribed) {
        return;
    }

    this->call_subscribe();
    this->is_subscribed = true;
}

void PubNub::set_channel_metadata(const Pubnub::String channel, const Pubnub::String metadata)
{
    auto result = pubnub_set_channelmetadata(
            this->main_context.get(),
            channel,
            "custom",
            metadata
    );

    this->await_and_handle_error(result);
}

void PubNub::remove_channel_metadata(const Pubnub::String channel)
{
    auto result = pubnub_remove_channelmetadata(
            this->main_context.get(),
            channel
    );

    this->await_and_handle_error(result);
}

Pubnub::String PubNub::get_channel_metadata(const Pubnub::String channel)
{
    auto result = pubnub_get_channelmetadata(
            this->main_context.get(),
            "custom",
            channel
    );

    this->await_and_handle_error(result);

    return pubnub_get(this->main_context.get());
}

Pubnub::String PubNub::get_all_channels_metadata(
        const Pubnub::String include,
        const int limit,
        const Pubnub::String filter,
        const Pubnub::String sort,
        const Pubnub::String page_next,
        const Pubnub::String page_prev
) {
    pubnub_getall_metadata_opts opt = pubnub_getall_metadata_defopts();
    opt.count = pubnub_tribool::pbccTrue;
    opt.filter = filter.c_str();
    opt.include = include.c_str();
    opt.limit = limit;
    opt.sort = sort.c_str();
    opt.page.next = page_next.c_str();
    opt.page.prev = page_prev.c_str();
    auto result = pubnub_getall_channelmetadata_ex(this->main_context.get(), opt);

    this->await_and_handle_error(result);

    return pubnub_get(this->main_context.get());
}

Pubnub::String PubNub::get_channel_members(
        const Pubnub::String channel,
        const Pubnub::String include,
        const int limit,
        const Pubnub::String filter,
        const Pubnub::String sort,
        const Pubnub::String page_next,
        const Pubnub::String page_prev
) {
    pubnub_members_opts opt = pubnub_members_opts();
    opt.count = pubnub_tribool::pbccTrue;
    opt.filter = filter.empty() ? NULL : filter.c_str();
    opt.include = include.empty() ? NULL : include.c_str();
    opt.limit = limit;
    opt.sort = sort.empty() ? NULL : sort.c_str();
    opt.page.next = page_next.empty() ? NULL : page_next.c_str();
    opt.page.prev = page_prev.empty() ? NULL : page_prev.c_str();


    auto result = pubnub_get_members_ex(
            this->main_context.get(), channel, opt);

    this->await_and_handle_error(result);

    return pubnub_get(this->main_context.get());
}

void PubNub::remove_members(const Pubnub::String channel, const Pubnub::String members_object)
{
    auto result = pubnub_remove_members(
            this->main_context.get(),
            channel,
            NULL,
            members_object
    );

    this->await_and_handle_error(result);
}

Pubnub::String PubNub::set_members(const Pubnub::String channel, const Pubnub::String members_object, const Pubnub::String include)
{
    auto result = pubnub_set_members(
            this->main_context.get(),
            channel,
            include,
            members_object
    );

    this->await_and_handle_error(result);

    return pubnub_get(this->main_context.get());
}

void PubNub::set_user_metadata(const Pubnub::String user_id, const Pubnub::String metadata)
{
    auto result = pubnub_set_uuidmetadata(
            this->main_context.get(),
            user_id,
            "custom",
            metadata
    );

    this->await_and_handle_error(result);
}

Pubnub::String PubNub::get_user_metadata(const Pubnub::String user_id)
{
    auto result = pubnub_get_uuidmetadata(
            this->main_context.get(),
            "custom",
            user_id
    );

    this->await_and_handle_error(result);

    return pubnub_get(this->main_context.get());
}

Pubnub::String PubNub::get_all_user_metadata(
        const Pubnub::String include,
        const int limit,
        const Pubnub::String filter,
        const Pubnub::String sort,
        const Pubnub::String page_next,
        const Pubnub::String page_prev
) {
    pubnub_getall_metadata_opts opt = pubnub_getall_metadata_defopts();
    opt.count = pubnub_tribool::pbccTrue;
    opt.filter = filter.c_str();
    opt.include = include.c_str();
    opt.limit = limit;
    opt.sort = sort.c_str();
    opt.page.next = page_next.c_str();
    opt.page.prev = page_prev.c_str();

    auto result = pubnub_getall_uuidmetadata_ex(this->main_context.get(), opt);

    this->await_and_handle_error(result);

    return pubnub_get(this->main_context.get());
}

void PubNub::remove_user_metadata(const Pubnub::String user_id)
{
    auto result = pubnub_remove_uuidmetadata(
            this->main_context.get(),
            user_id
    );

    this->await_and_handle_error(result);
}

Pubnub::String PubNub::get_memberships(
        const Pubnub::String user_id,
        const Pubnub::String include,
        const int limit,
        const Pubnub::String filter,
        const Pubnub::String sort,
        const Pubnub::String page_next,
        const Pubnub::String page_prev
) {
    pubnub_membership_opts opt = pubnub_membership_opts();
    opt.count = pubnub_tribool::pbccTrue;
    opt.filter = filter.c_str();
    opt.include = include.c_str();
    opt.limit = limit;
    opt.sort = sort.c_str();
    opt.page.next = page_next.c_str();
    opt.page.prev = page_prev.c_str();
    opt.uuid = user_id.c_str();

    auto result = pubnub_get_memberships_ex(this->main_context.get(), opt);

    this->await_and_handle_error(result);

    return pubnub_get(this->main_context.get());
}

Pubnub::String PubNub::set_memberships(const Pubnub::String user_id, const Pubnub::String memberships_object, const Pubnub::String include)
{
    auto result = pubnub_set_memberships(
            this->main_context.get(),
            user_id,
            include,
            memberships_object
    );

    this->await_and_handle_error(result);

    return pubnub_get(this->main_context.get());
}

void PubNub::remove_memberships(const Pubnub::String user_id, const Pubnub::String memberships_object)
{
    auto result = pubnub_remove_memberships(
            this->main_context.get(),
            user_id,
            NULL,
            memberships_object
    );

    this->await_and_handle_error(result);
}

Pubnub::String PubNub::where_now(const Pubnub::String user_id)
{
    auto result = pubnub_where_now(this->main_context.get(), user_id);

    this->await_and_handle_error(result);

    return pubnub_get(this->main_context.get());
}

Pubnub::String PubNub::here_now(const Pubnub::String channel)
{
    auto result = pubnub_here_now(this->main_context.get(), channel, NULL);

    this->await_and_handle_error(result);

    return pubnub_get(this->main_context.get());
}

Pubnub::String PubNub::get_user_id()
{
    return this->user_id;
}

Pubnub::String PubNub::fetch_history(
        const Pubnub::String channel,
        const Pubnub::String start_timetoken,
        const Pubnub::String end_timetoken,
        const int limit
) {
    auto options = pubnub_fetch_history_defopts();
    options.max_per_channel = limit;
    options.start = start_timetoken;
    options.end = end_timetoken;
    options.include_message_actions = true;
    options.include_meta = true;
    options.include_user_id = true;
    options.include_message_type = true;

    auto result = pubnub_fetch_history(this->main_context.get(), channel, options);

    this->await_and_handle_error(result);

    auto fetch_history_response = pubnub_get_fetch_history(this->main_context.get());

    if(!fetch_history_response.ptr)
    {
        throw std::runtime_error("Fetch history response is invalid");
    }

    return fetch_history_response.ptr;
}
Pubnub::String PubNub::fetch_history(
        const std::vector<Pubnub::String> channels,
        const Pubnub::String start_timetoken,
        const Pubnub::String end_timetoken,
        const int limit
) 
{
    return fetch_history(get_comma_sep_string_from_vector(channels), start_timetoken, end_timetoken, limit);
}

Pubnub::String PubNub::add_message_action(const Pubnub::String channel, const Pubnub::String message_time_token, const Pubnub::String message_action_type, const Pubnub::String value)
{
    Pubnub::String json_safe_value;;
    if ((value.front() != '\"' && value.back() != '\"') && (value.front() != '{' && value.back() != '}')) {
        // we asume it is a string and we need to add quotes 
        json_safe_value = Pubnub::String("\"") + value + Pubnub::String("\"");
    } else {
        json_safe_value = value;
    }    

    auto result = pubnub_add_message_action_str(this->main_context.get(), channel, message_time_token, message_action_type, json_safe_value);

    this->await_and_handle_error(result);

    pubnub_chamebl_t add_action_response = pubnub_get_message_action_timetoken(this->main_context.get());
    
    if(!add_action_response.ptr)
    {
        return "";
    }

    return Pubnub::String(add_action_response.ptr, add_action_response.size);
}

void PubNub::remove_message_action(const Pubnub::String channel, const Pubnub::String message_timetoken, const Pubnub::String action_timetoken)
{
    //TODO:: pubnub_str_2_chamebl_t could be used instead but it gives Linker error. There should be an easier way to achieve this
    
    Pubnub::String msg_timetoken_with_quotes = message_timetoken;
    if ((message_timetoken.front() != '\"' && message_timetoken.back() != '\"'))
    {
        msg_timetoken_with_quotes = Pubnub::String("\"") + message_timetoken + Pubnub::String("\"");
    }
    Pubnub::String action_timetoken_with_quotes = action_timetoken;
    if ((action_timetoken.front() != '\"' && action_timetoken.back() != '\"'))
    {
        action_timetoken_with_quotes = Pubnub::String("\"") + action_timetoken + Pubnub::String("\"");
    }
  
    // Allocate memory for message_timetoken_char and copy the content
    char* message_timetoken_char = new char[msg_timetoken_with_quotes.length() + 1];
    std::strcpy(message_timetoken_char, msg_timetoken_with_quotes.c_str());

    pubnub_chamebl_t message_timetoken_chamebl;
    message_timetoken_chamebl.ptr = message_timetoken_char;
    message_timetoken_chamebl.size = msg_timetoken_with_quotes.length();

    // Allocate memory for action_timetoken_char and copy the content
    char* action_timetoken_char = new char[action_timetoken_with_quotes.length() + 1];
    std::strcpy(action_timetoken_char, action_timetoken_with_quotes.c_str());

    pubnub_chamebl_t action_timetoken_chamebl;
    action_timetoken_chamebl.ptr = action_timetoken_char;
    action_timetoken_chamebl.size = action_timetoken_with_quotes.length();

    // Perform the action
    auto result = pubnub_remove_message_action(this->main_context.get(), channel.c_str(), message_timetoken_chamebl, action_timetoken_chamebl);
    this->await_and_handle_error(result);

    // Clean up allocated memory
    delete[] message_timetoken_char;
    delete[] action_timetoken_char;
}

std::map<Pubnub::String, int, Pubnub::StringComparer> PubNub::message_counts(const std::vector<Pubnub::String> channels, const std::vector<Pubnub::String> timestamps)
{
    std::map<Pubnub::String, int, Pubnub::StringComparer> final_map;
    auto result = pubnub_message_counts(this->main_context.get(), get_comma_sep_string_from_vector(channels).c_str(), get_comma_sep_string_from_vector(timestamps).c_str());
    this->await_and_handle_error(result);

    int size = channels.size();
    std::vector<int> MessageCountsReturn(size);

    int get_response = pubnub_get_message_counts(this->main_context.get(), get_comma_sep_string_from_vector(channels).c_str(), MessageCountsReturn.data());

    for(int i = 0; i < channels.size(); i++)
    {
        final_map[channels[i]] = get_response >= 0 ? MessageCountsReturn[i] : 0;
    }
    return final_map;
}

bool PubNub::delete_messages(const Pubnub::String channel, const Pubnub::String start, const Pubnub::String end)
{
    auto opt = pubnub_delete_messages_defopts();
    opt.start = start.c_str();
    opt.end = end.c_str();

    auto result = pubnub_delete_messages(this->main_context.get(), channel.c_str(), opt);

    try {
        this->await_and_handle_error(result);
    }
    catch(...)
    {
        return false;
    }

    return true;
}

void PubNub::await_and_handle_error(pubnub_res result)
{
    if (PNR_OK != result && PNR_STARTED != result) {
        auto error = pubnub_last_publish_result(this->main_context.get());
        ;
        throw std::runtime_error(pubnub_res_2_string(result));
    }

    if (PNR_STARTED == result) {
        pubnub_res await_result = pubnub_await(main_context.get());
        if (PNR_OK != await_result) {
            throw std::runtime_error(pubnub_res_2_string(result));
        }
    }
}

bool PubNub::is_subscribed_to_channel(const Pubnub::String channel)
{
    for (auto subscribed_channel : this->subscribed_channels)
    {
        if (subscribed_channel == channel)
        {
            return true;
        }
    }

    return false;

    //TODO:: use this when we solve string coparison for vectors
    /*
    return std::find(
            this->subscribed_channels.begin(),
            this->subscribed_channels.end(),
            channel
    ) != this->subscribed_channels.end();*/
}

void PubNub::cancel_previous_subscription()
{
    auto cancel_result = pubnub_cancel(this->long_poll_context.get());
    if (PN_CANCEL_FINISHED != cancel_result) {
        auto await_result = pubnub_await(this->long_poll_context.get());
        if (PNR_OK != await_result) {
            throw std::runtime_error(
                    std::string("Failed to cancel previous subscription: ")
                    + pubnub_res_2_string(
                        pubnub_last_result(this->long_poll_context.get())
                    )
                );
        }
    }

    // TODO: Should I always leave?
    auto result = pubnub_leave(this->long_poll_context.get(), get_comma_sep_channels_to_subscribe(), NULL);
    if (PNR_OK != result) {
        if (PNR_STARTED == result && PNR_OK != pubnub_await(this->long_poll_context.get())) {
            throw std::runtime_error(
                    std::string("Failed to leave previous subscription: ")
                    + pubnub_res_2_string(
                        pubnub_last_result(this->long_poll_context.get())
                    )
                );
        }
    }
}

void PubNub::call_subscribe()
{
    auto result = pubnub_subscribe_v2(this->long_poll_context.get(), get_comma_sep_channels_to_subscribe(), pubnub_subscribe_v2_defopts());
    if (PNR_OK != result && PNR_STARTED != result) {
        throw std::runtime_error(
                std::string("Failed to subscribe to channel: ")
                + pubnub_res_2_string(result)
            );
    }
}

void PubNub::call_handshake()
{
    auto result = pubnub_subscribe_v2(this->long_poll_context.get(), get_comma_sep_channels_to_subscribe(), pubnub_subscribe_v2_defopts());

    if (PNR_OK != result) {
        if (PNR_STARTED == result) {
            result = pubnub_await(this->long_poll_context.get());
        }

        if (PNR_OK != result) {
            throw std::runtime_error(
                    std::string("Failed to subscribe to channel: ")
                    + pubnub_res_2_string(result)
                    );

        }
    }
}

Pubnub::String PubNub::get_comma_sep_channels_to_subscribe()
{
    Pubnub::String current_channels;
    for (const auto& channel : this->subscribed_channels) {
        current_channels += channel + "," + channel + "-pnpres,";
    }
    if(!current_channels.empty())
    {
        current_channels.erase(current_channels.length() - 1, 1);
    }
    
    return current_channels;
}

Pubnub::String PubNub::get_comma_sep_string_from_vector(std::vector<Pubnub::String> vector_of_strings)
{
    Pubnub::String final_string;
    for (const auto& element : vector_of_strings) 
    {
        final_string += element + ",";
    }
    if(!final_string.empty())
    {
        final_string.erase(final_string.length() - 1, 1);
    }
    
    return final_string;
}

Pubnub::String PubNub::parse_token(const Pubnub::String auth_key) 
{
    return pubnub_parse_token(this->main_context.get(), auth_key.c_str());
}

Pubnub::String PubNub::get_current_auth_token() 
{
    return this->auth_key;
}

void PubNub::set_auth_token(const Pubnub::String token) 
{
    auth_key = token;
    pubnub_set_auth_token(this->main_context.get(), auth_key.c_str());
    pubnub_set_auth_token(this->long_poll_context.get(), auth_key.c_str());
}

int PubNub::set_pubnub_origin(const Pubnub::String origin) 
{
    custom_origin = origin;
    return pubnub_origin_set(this->main_context.get(), custom_origin.c_str());
    return pubnub_origin_set(this->long_poll_context.get(), custom_origin.c_str());
}

Pubnub::String PubNub::grant_token(const Pubnub::String permission_object)
{
	auto result = pubnub_grant_token(this->main_context.get(), permission_object.c_str());

    this->await_and_handle_error(result);

	pubnub_chamebl_t grant_token_resp = pubnub_get_grant_token(this->main_context.get());
	if(!grant_token_resp.ptr)
	{
		return "";
	}

    return grant_token_resp.ptr;
}

void PubNub::set_secret_key(const Pubnub::String key) 
{
    this->secret_key = key;
    pubnub_set_secret_key(this->main_context.get(), secret_key.c_str());
}