#include "membership_service.hpp"
#include "application/chat_service.hpp"
#include "application/dao/membership_dao.hpp"
#include "application/dao/user_dao.hpp"
#include "application/user_service.hpp"
#include "application/channel_service.hpp"
#include "application/access_manager_service.hpp"
#include "chat_helpers.hpp"
#include "domain/access_manager.hpp"
#include "domain/membership_entity.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"
#include "membership.hpp"
#include "nlohmann/json.hpp"
#include "callback_service.hpp"
#include "application/dao/channel_dao.hpp"
#include "domain/json.hpp"
#include <iostream>
#include <memory>
#include <chrono>

using namespace Pubnub;
using json = nlohmann::json;

MembershipService::MembershipService(ThreadSafePtr<PubNub> pubnub, std::weak_ptr<const ChatService> chat_service):
    pubnub(pubnub),
    chat_service(chat_service)
{}

std::vector<Membership> MembershipService::get_channel_members(const String& channel_id, const ChannelDAO& channel_data, int limit, const String& start_timetoken, const String& end_timetoken) const {
    String include_string = "custom,channel,totalCount,customChannel";

    auto get_channel_members_response = [this, channel_id, include_string, limit, start_timetoken, end_timetoken] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_channel_members(channel_id, include_string, limit, start_timetoken, end_timetoken);
    }();

    Json response_json = Json::parse(get_channel_members_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get members, response is incorrect");
    }

    Json users_array_json = response_json["data"];

    auto chat_service_shared = chat_service.lock();

    // TODO: domain
    std::vector<Membership> memberships;
    for (auto element : users_array_json)
    {
        UserEntity user_entity = UserEntity::from_json(element["uuid"].dump());
        String user_id = String(element["uuid"]["id"]);
        User user = chat_service_shared->user_service->create_user_object({user_id, user_entity});
        //We don't need to add channel to entity repository, as this whole function is called from a channel object - it has to already exist
        Channel channel = chat_service_shared->channel_service->create_channel_object({channel_id, channel_data.to_entity()});
        
        // TODO: no custom data?
        Membership membership = this->create_membership_object(user, channel);
        memberships.push_back(membership);
    }

    return memberships;
}

std::vector<Membership> MembershipService::get_user_memberships(const String& user_id, const UserDAO& user_data, int limit, const String& start_timetoken, const String& end_timetoken) const {
    String include_string = "totalCount,custom,channel,customChannel,channelType,status,channelStatus";

    auto get_memberships_response = [this, user_id, include_string, limit, start_timetoken, end_timetoken] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_memberships(user_id, include_string, limit, start_timetoken, end_timetoken);
    }();

    json response_json = json::parse(get_memberships_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get memberships, response is incorrect");
    }

    json channels_array_json = response_json["data"];

    auto chat_service_shared = chat_service.lock();

    std::vector<Membership> memberships;
    for (auto& element : channels_array_json)
    {
        //Create channel entity, as this channel maight be not in the repository yet. If it already is there, it will be updated
        ChannelEntity channel_entity = ChannelEntity::from_json(String(element.dump()));
        String channel_id = String(element["channel"]["id"]);

        Channel channel = chat_service_shared->channel_service->create_channel_object({channel_id, channel_entity});

        //We don't need to add user to entity repository, as this whole function is called from a user object - it has to already exist
        User user = chat_service_shared->user_service->create_user_object({user_id, user_data.to_entity()});

        Membership membership = this->create_membership_object(user, channel);
        memberships.push_back(membership);
    }

    return memberships;
}

Membership MembershipService::invite_to_channel(const String& channel_id, const ChannelDAO& channel_data, const User& user) const {
    auto chat_service_shared = chat_service.lock();

    Channel channel = chat_service_shared->channel_service->create_channel_object({channel_id, channel_data.to_entity()});

    if(channel.channel_data().type == String("public"))
    {
        throw std::runtime_error("Channel invites are not supported in Public chats");
    }

    //TODO:: check here if user already is on that channel. Requires C-Core filtering


    String include_string = "custom,channel,totalCount,customChannel";
    String set_memeberships_obj = create_set_memberships_object(channel_id, "");

    auto user_id = user.user_id();
    auto memberships_response = [this, user_id, set_memeberships_obj, include_string] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->set_memberships(user_id, set_memeberships_obj, include_string);
    }();
    
    json memberships_response_json = json::parse(memberships_response);

    String channel_data_string = memberships_response_json["data"][0].dump();

    String event_payload = "{\"channelType\": \"" + channel.channel_data().type + "\", \"channelId\": \"" + channel_id + "\"}";
    chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_INVITE, user.user_id(), event_payload);

    //This channel is updated, so we need to update it in entity repository as well
    ChannelEntity channel_entity = ChannelEntity::from_json(channel_data_string);
    
    // TODO: no custom data?
    Membership membership_object = this->create_membership_object(user, channel);
    membership_object.set_last_read_message_timetoken(get_now_timetoken());
    return membership_object;
}

std::vector<Membership> MembershipService::invite_multiple_to_channel(const String& channel_id, const ChannelDAO& channel_data, const std::vector<User>& users) const
{
    auto chat_service_shared = chat_service.lock();

    Channel channel = chat_service_shared->channel_service->create_channel_object({channel_id, channel_data.to_entity()});

    if(channel.channel_data().type == String("public"))
    {
        throw std::runtime_error("Channel invites are not supported in Public chats");
    }

    //TODO:: check here if users already are on that channel. Requires C-Core filtering


    std::vector<String> filtered_users_ids;

    for(auto &user : users)
    {
        filtered_users_ids.push_back(user.user_id());
    }

    String include_string = "custom,channel,totalCount,customChannel";
    String set_memebers_obj = create_set_members_object(filtered_users_ids, "");

    auto set_members_response = [this, channel_id, set_memebers_obj, include_string] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->set_members(channel_id, set_memebers_obj, include_string);
    }();
    
    std::vector<Membership> invitees_memberships;

    json memberships_response_json = json::parse(set_members_response);
    json memberships_data_array = memberships_response_json["data"];

    String test = memberships_data_array.dump();
    
    for (json::iterator single_data_json = memberships_data_array.begin(); single_data_json != memberships_data_array.end(); ++single_data_json) 
    {
        // TODO: @kamil - check if this assumption is correct
        auto user = std::find_if(users.begin(), users.end(), [single_data_json](const User& user) {
            return user.user_id() == String(single_data_json.value()["uuid"]["id"]);
        });

        if(user == users.end()) {
            continue;
        }

        // TODO: no custom data?
        Membership membership = this->create_membership_object(*user, channel);
        membership.set_last_read_message_timetoken(get_now_timetoken());
        invitees_memberships.push_back(membership);

        String event_payload = "{\"channelType\": \"" + channel.channel_data().type + "\", \"channelId\": \"" + channel_id + "\"}";
        chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_INVITE, user->user_id(), event_payload);
    }

    return invitees_memberships;
}

Membership MembershipService::update(const User& user, const Channel& channel, const String& custom_object_json) const {
    String custom_object_json_string;
    custom_object_json.empty() ? custom_object_json_string = "{}" : custom_object_json_string = custom_object_json;

    json response_json = json::parse(custom_object_json_string);
    
    if(response_json.is_null() || !response_json.is_object())
    {
        throw std::invalid_argument("Can't update membership, custom_object_json is not valid json object");
    }

	String set_memberships_string = String("[{\"channel\": {\"id\": \"") + channel.channel_id() + String("\"}, \"custom\": ") + custom_object_json_string + String("}]");

    {
        auto pubnub_handle = pubnub->lock();
        pubnub_handle->set_memberships(user.user_id(), set_memberships_string);
    }

    return create_membership_object(user, channel, create_domain_membership(custom_object_json_string));
}

String MembershipService::last_read_message_timetoken(const Membership& membership) const {
    String custom_data = membership.custom_data();
    if(custom_data.empty())
    {
        return String();
    }
    
    Json custom_data_json = Json::parse(custom_data);

    return custom_data_json.get_string("lastReadMessageTimetoken").value_or(String(""));
}

Pubnub::Membership MembershipService::set_last_read_message_timetoken(const Membership& membership, const String& timetoken) const {
    String custom_data = membership.custom_data().empty() ? "{}" : membership.custom_data();

    Json custom_data_json = Json::parse(custom_data);
    custom_data_json.insert_or_update("lastReadMessageTimetoken", timetoken);
    Pubnub::Membership new_membership = membership.update(custom_data_json.dump());
    
    //TODO:: in js chat here is check in access manager if event can be sent

    auto chat_service_shared = this->chat_service.lock();

    auto can_i_emit = chat_service_shared->access_manager_service->can_i(
            AccessManager::Permission::WRITE, AccessManager::ResourceType::CHANNELS, membership.channel.channel_id());

    if(can_i_emit) {
        String event_payload = "{\"messageTimetoken\": " + timetoken +", \"userId\": \"" + membership.user.user_id() + "\"}";
        chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_RECEPIT, membership.channel.channel_id(), event_payload);
    } else {
        // TODO: right now we don't have a logger yet
        std::cerr << "Can't emit chat event, user doesn't have permissions" << std::endl;
    }

    return new_membership;
}

int MembershipService::get_unread_messages_count_one_channel(const Membership& membership) const {
    auto pubnub_handle = pubnub->lock();
    String last_read_timetoken = this->last_read_message_timetoken(membership).empty() ? EMPTY_TIMETOKEN : this->last_read_message_timetoken(membership);
    auto message_counts_map =  pubnub_handle->message_counts({membership.channel.channel_id()}, {last_read_timetoken});

    return message_counts_map[membership.channel.channel_id()];
}

std::vector<std::tuple<Pubnub::Channel, Pubnub::Membership, int>> MembershipService::get_all_unread_messages_counts(const Pubnub::String& start_timetoken, const Pubnub::String& end_timetoken, const Pubnub::String& filter, int limit) const {
    auto chat_service_shared = chat_service.lock();
    std::vector<std::tuple<Pubnub::Channel, Pubnub::Membership, int>> return_tuples;

    User current_user = chat_service_shared->user_service->get_current_user();
    //TODO:: pass filter here when it will be supported in C-Core
    auto user_memberships = current_user.get_memberships(limit, start_timetoken, end_timetoken);

    if(user_memberships.size() == 0)
    {
        return return_tuples;
    }

    std::vector<String> channels;
    std::vector<String> timetokens;

    for(auto &membership : user_memberships)
    {
        channels.push_back(membership.channel.channel_id());
        String last_timetoken = this->last_read_message_timetoken(membership);
        if(last_timetoken.empty())
        {
            //Message counts requires a valid timetoken, so this one will be like "0", from beginning of the channel
            last_timetoken = EMPTY_TIMETOKEN;
        }
        timetokens.push_back(last_timetoken);
    }

    auto messages_counts_response = [this, channels, timetokens] {
        auto pubnub_handle = pubnub->lock();
        return pubnub_handle->message_counts(channels, timetokens);
    }();

    for(auto &membership : user_memberships)
    {
        if(messages_counts_response.find(membership.channel.channel_id()) != messages_counts_response.end())
        {
            std::tuple<Pubnub::Channel, Pubnub::Membership, int> return_tuple(membership.channel, membership, messages_counts_response[membership.channel.channel_id()]);
            return_tuples.push_back(return_tuple);
        }
    }

    return return_tuples;
}

std::tuple<Pubnub::Page, int, int, std::vector<Pubnub::Membership>> MembershipService::mark_all_messages_as_read(const Pubnub::String &filter, const Pubnub::String &sort, int limit, const Pubnub::Page& page) const
{
    auto chat_service_shared = chat_service.lock();
    std::tuple<Pubnub::Page, int, int, std::vector<Pubnub::Membership>> return_tuple;

    User current_user = chat_service_shared->user_service->get_current_user();
    //TODO:: pass filter here when it will be supported in C-Core
    auto user_memberships = current_user.get_memberships(limit, page.next, page.prev);

    if(user_memberships.size() == 0)
    {
        return return_tuple;
    }

    std::vector<String> relevant_channel_ids;
    String now_timetoken = get_now_timetoken();

    //For all memberships get custom data, add current timetoken as lastReadMessageTimetoken and combine everything to SetMembers payload
    String set_memberships_string = String("[");
    for(auto membership : user_memberships)
    {
        relevant_channel_ids.push_back(membership.channel.channel_id());
        
        String custom_object_json_string = membership.custom_data().empty() ? String("{}") : membership.custom_data();
        json custom_object_json = json::parse(custom_object_json_string);
        custom_object_json["lastReadMessageTimetoken"] = now_timetoken.c_str();

        set_memberships_string += String("{\"channel\": {\"id\": \"") + membership.channel.channel_id() + String("\"}, \"custom\": ") + custom_object_json.dump() + String("},");
    }

    set_memberships_string.erase(set_memberships_string.length() - 1, 1);
    set_memberships_string += String("]");

    /*  TODO:: this is disabled, because it looks like pubnub_fetch_history doesn't support getting messages for multiple channels
    auto fetch_messages_response = [this, relevant_channel_ids] {
        auto pubnub_handle = pubnub->lock();
        return pubnub_handle->fetch_history("relevant_channel_ids", (char*)NULL, (char*)NULL, 1);
    }();
    */


    auto memberships_response = [this, current_user, set_memberships_string] {
        auto pubnub_handle = pubnub->lock();
        String include_string = "custom,channel,totalCount,customChannel";
        return pubnub_handle->set_memberships(current_user.user_id(), set_memberships_string, include_string);
    }();


    Json response_json = Json::parse(memberships_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't mark all messages as read, set memberships response is incorrect");
    }

    Json channels_array_json = response_json["data"];

    //Create new memberships with updated values from response
    std::vector<Membership> memberships;
    for (auto element : channels_array_json)
    {
        String channel_id = String(element["channel"]["id"]);

        //Skip channels that are not in original request. TODO:: make this easier with filter
        bool is_in_channels = false;
        for (auto relevant_channel : relevant_channel_ids)
        {
            if (channel_id == relevant_channel)
            {
                is_in_channels = true;
                break;
            }
        }
        if (!is_in_channels)
        {
            continue;
        }

        ChannelEntity channel_entity = ChannelEntity::from_json(String(element.dump()));

        Channel channel = chat_service_shared->channel_service->create_channel_object({channel_id, channel_entity});

        Membership membership = this->create_membership_object(current_user, channel);
        memberships.push_back(membership);

        //Emit events for updated memberships
        String event_payload = "{\"messageTimetoken\": " + now_timetoken +", \"userId\": \"" + membership.user.user_id() + "\"}";
        chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_RECEPIT, channel_id, event_payload);
    }
    int total = response_json.get_int("totalCount").value_or(0);
    int status = response_json.get_int("status").value_or(0);
    Page page_response;
    page_response.next = response_json.get_string("next").value_or(String(""));
    page_response.prev = response_json.get_string("prev").value_or(String(""));

    return std::tuple<Pubnub::Page, int, int, std::vector<Pubnub::Membership>>(page_response, total, status, memberships);
}

void MembershipService::stream_updates_on(const std::vector<Membership>& memberships, std::function<void(const Membership&)> membership_callback) const
{
    if(memberships.empty())
    {
        throw std::invalid_argument("Cannot stream memberships updates on an empty list");
    }

    auto pubnub_handle = pubnub->lock();

#ifndef PN_CHAT_C_ABI
    if (auto chat = this->chat_service.lock()) {
#endif
        for(auto& membership : memberships)
        {
            auto messages = pubnub_handle->subscribe_to_channel_and_get_messages(membership.channel.channel_id());

#ifndef PN_CHAT_C_ABI
            chat->callback_service->broadcast_messages(messages);
            chat->callback_service->register_membership_callback(
                    membership.channel.channel_id(), membership.user.user_id(), membership_callback);
        }
#endif
    }
}

Membership MembershipService::create_membership_object(const User& user, const Channel& channel) const {
    return this->create_membership_object(user, channel, MembershipEntity{channel.channel_data().custom_data_json});
}

Membership MembershipService::create_membership_object(const User& user, const Channel& channel, const MembershipEntity& membership_entity) const {
    if (auto chat = this->chat_service.lock()) {
        return Membership(user, channel, chat, shared_from_this(), std::make_unique<MembershipDAO>(membership_entity));
    }

    throw std::runtime_error("Can't create membership, chat service pointer is invalid");
}

MembershipEntity MembershipService::create_domain_membership(const String& custom_object_json) const {
    MembershipEntity new_membership_entity;
    new_membership_entity.custom_field = custom_object_json;
    return new_membership_entity;
    
}
