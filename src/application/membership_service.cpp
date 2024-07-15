#include "membership_service.hpp"
#include "application/chat_service.hpp"
#include "application/user_service.hpp"
#include "application/channel_service.hpp"
#include "chat_helpers.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"
#include "nlohmann/json.hpp"
#include "callback_service.hpp"

using namespace Pubnub;
using json = nlohmann::json;

MembershipService::MembershipService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository, std::weak_ptr<ChatService> chat_service):
    pubnub(pubnub),
    entity_repository(entity_repository),
    chat_service(chat_service)
{}

String MembershipService::get_membership_custom_data(String user_id, String channel_id)
{
    auto maybe_membership = this->entity_repository->get_membership_entities().get(std::pair<String, String>(user_id, channel_id));

    if (!maybe_membership.has_value()) 
    {
        throw std::invalid_argument("Failed to get membership data, there is no memebrship with these user and channel ids");
    }

    return maybe_membership.value().custom_field;
}

std::vector<Membership> MembershipService::get_channel_members(String channel_id, int limit, String start_timetoken, String end_timetoken)
{
    String include_string = "totalCount,customFields,channelFields,customChannelFields";

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

    std::vector<Membership> memberships;
    for (auto element : users_array_json)
    {
        //Create user entity, as this user maight be not in the repository yet. If it already is there, it will be updated
        UserEntity user_entity = UserEntity::from_json(element["uuid"].dump());
        String user_id = String(element["uuid"]["id"]);
        entity_repository->get_user_entities().update_or_insert(user_id, user_entity);

        User user = chat_service_shared->user_service->create_presentation_object(user_id);
        //We don't need to add channel to entity repository, as this whole function is called from a channel object - it has to already exist
        Channel channel = chat_service_shared->channel_service->create_presentation_object(channel_id);
        
        Membership membership = create_presentation_object(user, channel);
        memberships.push_back(membership);
    }

    return memberships;
}

std::vector<Membership> MembershipService::get_user_memberships(String user_id, int limit, String start_timetoken, String end_timetoken)
{
    String include_string = "totalCount,customFields,channelFields,customChannelFields,channelTypeField,statusField,channelStatusField";

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
        ChannelEntity channel_entity = ChannelEntity::from_json(String(element["channel"].dump()));
        String channel_id = String(element["channel"]["id"]);
        entity_repository->get_channel_entities().update_or_insert(channel_id, channel_entity);

        Channel channel = chat_service_shared->channel_service->create_presentation_object(channel_id);

        //We don't need to add user to entity repository, as this whole function is called from a user object - it has to already exist
        User user = chat_service_shared->user_service->create_presentation_object(user_id);

        Membership membership = create_presentation_object(user, channel);
        memberships.push_back(membership);
    }

    return memberships;
}

Membership MembershipService::invite_to_channel(String channel_id, User user)
{
    auto chat_service_shared = chat_service.lock();

    Channel channel = chat_service_shared->channel_service->create_presentation_object(channel_id);

    if(channel.channel_data().type == String("public"))
    {
        throw std::runtime_error("Channel invites are not supported in Public chats");
    }

    //TODO:: check here if user already is on that channel. Requires C-Core filtering


    String include_string = "totalCount,customFields,channelFields,customChannelFields";
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
    entity_repository->get_channel_entities().update_or_insert(channel_id, channel_entity);
    
    Membership membership_object = create_presentation_object(user, channel);
    membership_object.set_last_read_message_timetoken(get_now_timetoken());
    return membership_object;
}

std::vector<Membership> MembershipService::invite_multiple_to_channel(String channel_id, std::vector<User> users)
{
    auto chat_service_shared = chat_service.lock();

    Channel channel = chat_service_shared->channel_service->create_presentation_object(channel_id);

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

    String include_string = "totalCount,customFields,channelFields,customChannelFields";
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
        User user = chat_service_shared->user_service->create_presentation_object(String(single_data_json.value()["uuid"]["id"]));
        Membership membership = create_presentation_object(user, channel);
        membership.set_last_read_message_timetoken(get_now_timetoken());
        invitees_memberships.push_back(membership);

        String event_payload = "{\"channelType\": \"" + channel.channel_data().type + "\", \"channelId\": \"" + channel_id + "\"}";
        chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_INVITE, user.user_id(), event_payload);
    }

    return invitees_memberships;
}

Membership MembershipService::update(User user, Channel channel, String custom_object_json)
{
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

String MembershipService::last_read_message_timetoken(Membership membership)
{
    String custom_data = membership.custom_data();
    if(custom_data.empty())
    {
        return String();
    }
    
    json custom_data_json = json::parse(custom_data);

    if(custom_data_json.contains("lastReadMessageTimetoken") && !custom_data_json["lastReadMessageTimetoken"].is_null())
    {
        return String(custom_data_json["lastReadMessageTimetoken"]);
    }
    
    return String();
}

void MembershipService::set_last_read_message_timetoken(Membership membership, String timetoken)
{
    String custom_data = membership.custom_data().empty() ? "{}" : membership.custom_data();

    json custom_data_json = json::parse(custom_data);
    custom_data_json["lastReadMessageTimetoken"] = timetoken;
    membership.update(custom_data_json.dump());

    //Update entity repository with updated membership
    MembershipEntity new_membership_entity = create_domain_membership(custom_data_json.dump());
    entity_repository->get_membership_entities().update_or_insert(std::pair<String, String>(membership.user.user_id(), membership.channel.channel_id()), new_membership_entity);

    //TODO:: in js chat here is check in access manager if event can be sent

    auto chat_service_shared = this->chat_service.lock();

    String event_payload = String("{\"messageTimetoken\": \"") + timetoken + String("\"}");
    chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_RECEPIT, membership.channel.channel_id(), event_payload);
}

int MembershipService::get_unread_messages_count_one_channel(Membership membership)
{
    auto pubnub_handle = pubnub->lock();
    auto message_counts_map =  pubnub_handle->message_counts({membership.channel.channel_id()}, {this->last_read_message_timetoken(membership)});

    return message_counts_map[membership.channel.channel_id()];
}

std::vector<std::tuple<Pubnub::Channel, Pubnub::Membership, int>> MembershipService::get_all_unread_messages_counts(Pubnub::String start_timetoken, Pubnub::String end_timetoken, Pubnub::String filter, int limit)
{
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
            last_timetoken = "0";
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

void MembershipService::stream_updates_on(std::vector<Membership> memberships, std::function<void(Membership)> membership_callback)
{
    if(memberships.empty())
    {
        throw std::invalid_argument("Cannot stream memberships updates on an empty list");
    }

    auto pubnub_handle = pubnub->lock();

#ifndef PN_CHAT_C_ABI
    if (auto chat = this->chat_service.lock()) {
#endif
        for(auto membership : memberships)
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

Membership MembershipService::create_presentation_object(User user, Channel channel)
{
    auto chat_service_shared = chat_service.lock();

    return Membership(user, channel, chat_service_shared, shared_from_this());
}

Membership MembershipService::create_membership_object(User user, Channel channel, MembershipEntity membership_entity)
{
    if (auto chat = this->chat_service.lock()) {
        this->entity_repository
            ->get_membership_entities()
            .update_or_insert(std::make_pair(user.user_id(), channel.channel_id()), membership_entity);

        return Membership(user, channel, chat, shared_from_this());
    }

    throw std::runtime_error("Can't create membership, chat service pointer is invalid");
}

MembershipEntity MembershipService::create_domain_membership(String custom_object_json)
{
    MembershipEntity new_membership_entity;
    new_membership_entity.custom_field = custom_object_json;
    return new_membership_entity;
    
}
