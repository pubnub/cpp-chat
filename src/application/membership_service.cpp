#include "membership_service.hpp"
#include "application/chat_service.hpp"
#include "application/dao/membership_dao.hpp"
#include "application/dao/user_dao.hpp"
#include "application/user_service.hpp"
#include "application/channel_service.hpp"
#include "application/access_manager_service.hpp"
#include "chat_helpers.hpp"
#include "domain/access_manager_logic.hpp"
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

std::tuple<std::vector<Pubnub::Membership>, Pubnub::Page, int, Pubnub::String> MembershipService::get_channel_members(const String& channel_id, const ChannelDAO& channel_data, const Pubnub::String &filter, const Pubnub::String &sort, int limit, const Pubnub::Page &page) const {

    if(limit < 0 || limit > PN_MAX_LIMIT)
    {
        throw std::invalid_argument("can't get members, limit has to be within 0 - " + std::to_string(PN_MAX_LIMIT) + " range");
    }

    String include_string = "totalCount,custom,status,type,uuid,uuid.custom,uuid.status,uuid.type";

    auto get_channel_members_response = [this, channel_id, include_string, limit, filter, sort, page] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_channel_members(channel_id, include_string, limit, filter, sort, page.next, page.prev);
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
    for (Json::Iterator single_data_json = users_array_json.begin(); single_data_json != users_array_json.end(); ++single_data_json)
    {
        UserEntity user_entity = UserEntity::from_json(single_data_json.value()["uuid"].dump());
        String user_id = String(single_data_json.value()["uuid"]["id"]);
        User user = chat_service_shared->user_service->create_user_object({user_id, user_entity});
        //We don't need to add channel to entity repository, as this whole function is called from a channel object - it has to already exist
        Channel channel = chat_service_shared->channel_service->create_channel_object({channel_id, channel_data.to_entity()});
        
        MembershipEntity membership_entity = MembershipEntity::from_json(single_data_json.value());
        Membership membership = this->create_membership_object(user, channel, membership_entity);
        memberships.push_back(membership);
    }

    int total_count = response_json.get_int("totalCount").value_or(0);
    Page page_response({response_json.get_string("next").value_or(String("")), response_json.get_string("prev").value_or(String(""))});
    String status = response_json.get_string("status").value_or(String(""));
    std::tuple<std::vector<Pubnub::Membership>, Pubnub::Page, int, Pubnub::String> return_tuple = std::make_tuple(memberships, page_response, total_count, status);

    return return_tuple;
}

std::tuple<std::vector<Pubnub::Membership>, Pubnub::Page, int, Pubnub::String> MembershipService::get_user_memberships(const String& user_id, const UserDAO& user_data, const Pubnub::String &filter, const Pubnub::String &sort, int limit, const Pubnub::Page &page) const {
    if(limit < 0 || limit > PN_MAX_LIMIT)
    {
        throw std::invalid_argument("can't get memberships, limit has to be within 0 - " + std::to_string(PN_MAX_LIMIT) + " range");
    }

    String include_string = "totalCount,custom,channel,channel.custom,type,channel.type,status,channel.status";

    auto get_memberships_response = [this, user_id, include_string, filter, sort,  limit, page] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_memberships(user_id, include_string, limit, filter, sort, page.next, page.prev);
    }();

    Json response_json = Json::parse(get_memberships_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get memberships, response is incorrect");
    }

    Json channels_array_json = response_json["data"];

    auto chat_service_shared = chat_service.lock();

    std::vector<Membership> memberships;
    
    for (Json::Iterator single_data_json = channels_array_json.begin(); single_data_json != channels_array_json.end(); ++single_data_json)
    {
        //Create channel entity, as this channel maight be not in the repository yet. If it already is there, it will be updated
        ChannelEntity channel_entity = ChannelEntity::from_json(String(single_data_json.value().dump()));
        String channel_id = String(single_data_json.value()["channel"]["id"]);

        Channel channel = chat_service_shared->channel_service->create_channel_object({channel_id, channel_entity});

        //We don't need to add user to entity repository, as this whole function is called from a user object - it has to already exist
        User user = chat_service_shared->user_service->create_user_object({user_id, user_data.to_entity()});

        MembershipEntity membership_entity = MembershipEntity::from_json(single_data_json.value());
        Membership membership = this->create_membership_object(user, channel, membership_entity);
        memberships.push_back(membership);
    }

    int total_count = response_json.get_int("totalCount").value_or(0);
    Page page_response({response_json.get_string("next").value_or(String("")), response_json.get_string("prev").value_or(String(""))});
    String status = response_json.get_string("status").value_or(String(""));
    std::tuple<std::vector<Pubnub::Membership>, Pubnub::Page, int, Pubnub::String> return_tuple = std::make_tuple(memberships, page_response, total_count, status);

    return return_tuple;
}

Membership MembershipService::invite_to_channel(const String& channel_id, const ChannelDAO& channel_data, const User& user) const {
    auto chat_service_shared = chat_service.lock();

    Channel channel = chat_service_shared->channel_service->create_channel_object({channel_id, channel_data.to_entity()});

    //Check if user is not a member of the channel that he is invited to
    Pubnub::String filter = Pubnub::String("uuid.id == \"") + user.user_id() + Pubnub::String("\"");
    auto members = std::get<0>(this->get_channel_members(channel_id, channel_data, filter));
    if(!members.empty())
    {
        //Already a member, just return current membership
        return members[0];
    }

    String include_string = "custom,channel,channel.custom,type,channel.type,status,channel.status";
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
    
    //This is newly created membership, so membership data is empty
    MembershipEntity membership_entity;
    Membership membership_object = this->create_membership_object(user, channel, membership_entity);
    membership_object.set_last_read_message_timetoken(get_now_timetoken());
    return membership_object;
}

std::vector<Membership> MembershipService::invite_multiple_to_channel(const String& channel_id, const ChannelDAO& channel_data, const std::vector<User>& users) const
{
    auto chat_service_shared = chat_service.lock();

    Channel channel = chat_service_shared->channel_service->create_channel_object({channel_id, channel_data.to_entity()});

    std::vector<String> users_ids;
    String filter;

    for(int i = 0; i < users.size(); i++)
    {
        users_ids.push_back(users[i].user_id());
        filter += Pubnub::String("uuid.id == \"") + users[i].user_id() + Pubnub::String("\"");
        if(i < users.size() - 1)
        {
            filter += Pubnub::String(" || ");
        }
    }

    String include_string = "custom,type,status";
    String set_memebers_obj = create_set_members_object(users_ids, "");

    auto set_members_response = [this, channel_id, set_memebers_obj, include_string, filter] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->set_members(channel_id, set_memebers_obj, include_string, filter);
    }();
    
    std::vector<Membership> invitees_memberships;

    json memberships_response_json = json::parse(set_members_response);
    json memberships_data_array = memberships_response_json["data"];
    
    for (json::iterator single_data_json = memberships_data_array.begin(); single_data_json != memberships_data_array.end(); ++single_data_json) 
    {
        auto user = std::find_if(users.begin(), users.end(), [single_data_json](const User& user) {
            return user.user_id() == String(single_data_json.value()["uuid"]["id"]);
        });

        if(user == users.end()) {
            continue;
        }

        MembershipEntity membership_entity = MembershipEntity::from_json(single_data_json.value());
        Membership membership = this->create_membership_object(*user, channel, membership_entity);
        membership.set_last_read_message_timetoken(get_now_timetoken());
        invitees_memberships.push_back(membership);

        String event_payload = "{\"channelType\": \"" + channel.channel_data().type + "\", \"channelId\": \"" + channel_id + "\"}";
        chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_INVITE, user->user_id(), event_payload);
    }

    return invitees_memberships;
}

Membership MembershipService::update(const User& user, const Channel& channel, MembershipDAO membership_data) const {
    auto entity = membership_data.to_entity();

    String custom_object_json_string;
    entity.custom_field.empty() ? custom_object_json_string = "{}" : custom_object_json_string = entity.custom_field;

    json response_json = json::parse(custom_object_json_string);
    
    if(response_json.is_null() || !response_json.is_object())
    {
        throw std::invalid_argument("Can't update membership, custom_object_json is not valid json object");
    }

    {
        auto pubnub_handle = pubnub->lock();
        pubnub_handle->set_memberships(user.user_id(), entity.get_set_memberships_json_string(channel.channel_id()));
    }

    return create_membership_object(user, channel, entity);
}

String MembershipService::last_read_message_timetoken(const Membership& membership) const {
    String custom_data = membership.membership_data().custom_data_json;
    if(custom_data.empty())
    {
        return String();
    }
    
    Json custom_data_json = Json::parse(custom_data);

    return custom_data_json.get_string("lastReadMessageTimetoken").value_or(String(""));
}

Pubnub::Membership MembershipService::set_last_read_message_timetoken(const Membership& membership, const String& timetoken) const {
    
    String custom_data = membership.membership_data().custom_data_json.empty() ? "{}" : membership.membership_data().custom_data_json;

    Json custom_data_json = Json::parse(custom_data);
    custom_data_json.insert_or_update("lastReadMessageTimetoken", timetoken);
    Pubnub::Membership new_membership = membership.update(custom_data_json.dump());
    
    //TODO:: in js chat here is check in access manager if event can be sent

    auto chat_service_shared = this->chat_service.lock();

    auto can_i_emit = chat_service_shared->access_manager_service->can_i(
            AccessManager::Permission::WRITE, AccessManager::ResourceType::CHANNELS, membership.channel.channel_id());

    if(can_i_emit) {
        String event_payload = "{\"messageTimetoken\": \"" + timetoken + "\"}";
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

std::vector<std::tuple<Pubnub::Channel, Pubnub::Membership, int>> MembershipService::get_all_unread_messages_counts(const Pubnub::String &filter, const Pubnub::String &sort, int limit, const Pubnub::Page &page) const {
    
    if(limit < 0 || limit > PN_MAX_LIMIT)
    {
        throw std::invalid_argument("can't get unread messages counts, limit has to be within 0 - " + std::to_string(PN_MAX_LIMIT) + " range");
    }
    
    auto chat_service_shared = chat_service.lock();
    std::vector<std::tuple<Pubnub::Channel, Pubnub::Membership, int>> return_tuples;

    User current_user = chat_service_shared->user_service->get_current_user();

    auto user_memberships = current_user.get_memberships(filter, sort, limit, page).memberships;

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
    if(limit < 0 || limit > PN_MAX_LIMIT)
    {
        throw std::invalid_argument("can't mark all messages as read, limit has to be within 0 - " + std::to_string(PN_MAX_LIMIT) + " range");
    }

    auto chat_service_shared = chat_service.lock();
    std::tuple<Pubnub::Page, int, int, std::vector<Pubnub::Membership>> return_tuple;

    User current_user = chat_service_shared->user_service->get_current_user();
    auto user_memberships = current_user.get_memberships(filter, sort, limit, page).memberships;

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
        
        String custom_object_json_string = membership.membership_data().custom_data_json.empty() ? String("{}") : membership.membership_data().custom_data_json;
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
        String include_string = "totalCount,custom,channel,channel.custom,type,channel.type,status,channel.status";
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

        MembershipEntity membership_entity = MembershipEntity::from_json(element);
        Membership membership = this->create_membership_object(current_user, channel, membership_entity);
        memberships.push_back(membership);

        //Emit events for updated memberships
        String event_payload = "{\"messageTimetoken\": \"" + now_timetoken +"\"}";
        chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_RECEPIT, channel_id, event_payload);
    }
    int total = response_json.get_int("totalCount").value_or(0);
    int status = response_json.get_int("status").value_or(0);
    Page page_response;
    page_response.next = response_json.get_string("next").value_or(String(""));
    page_response.prev = response_json.get_string("prev").value_or(String(""));

    return std::tuple<Pubnub::Page, int, int, std::vector<Pubnub::Membership>>(page_response, total, status, memberships);
}


std::shared_ptr<Subscription> MembershipService::stream_updates(Pubnub::Membership calling_membership, std::function<void(Membership)> membership_callback) const
{
    auto subscription = this->pubnub->lock()->subscribe(calling_membership.channel.channel_id());

    auto callback_service = this->chat_service.lock()->callback_service;
    subscription->add_membership_update_listener(callback_service->to_c_membership_update_callback(calling_membership, this->chat_service, membership_callback));

    return subscription;
}

std::shared_ptr<SubscriptionSet> MembershipService::stream_updates_on(Pubnub::Membership calling_membership, const std::vector<Pubnub::Membership>& memberships, std::function<void(std::vector<Membership>)> membership_callback) const
{
    if(memberships.empty())
    {
        throw std::invalid_argument("Cannot stream membership updates on an empty list");
    }

    std::vector<String> memberships_ids;

    std::transform(memberships.begin(), memberships.end(), std::back_inserter(memberships_ids), [](const Pubnub::Membership& membership) {
        return membership.channel.channel_id();
    });
    
    auto subscription = this->pubnub->lock()->subscribe_multiple(memberships_ids);

    auto callback_service = this->chat_service.lock()->callback_service;
    subscription->add_membership_update_listener(callback_service->to_c_memberships_updates_callback(memberships, this->chat_service, membership_callback));

    return subscription;
}

Membership MembershipService::create_membership_object(const User& user, const Channel& channel, const MembershipEntity& membership_entity) const {
    if (auto chat = this->chat_service.lock()) {
        return Membership(user, channel, chat, shared_from_this(), std::make_unique<MembershipDAO>(membership_entity));
    }

    throw std::runtime_error("Can't create membership, chat service pointer is invalid");
}

Pubnub::Membership MembershipService::update_membership_with_base(const Pubnub::Membership& membership, const Pubnub::Membership& base_membership) const {
        MembershipEntity base_entity = MembershipDAO(base_membership.membership_data()).to_entity();
        MembershipEntity membership_entity = MembershipDAO(membership.membership_data()).to_entity();

        return create_membership_object(
                base_membership.user,
                base_membership.channel,
                MembershipEntity::from_base_and_updated_membership(base_entity, membership_entity)
        );
}
