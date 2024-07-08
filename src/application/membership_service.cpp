#include "membership_service.hpp"
#include "application/chat_service.hpp"
#include "application/user_service.hpp"
#include "application/channel_service.hpp"
#include "chat_helpers.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"
#include "nlohmann/json.hpp"

using namespace Pubnub;
using json = nlohmann::json;

MembershipService::MembershipService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository, std::weak_ptr<ChatService> chat_service):
    pubnub(pubnub),
    entity_repository(entity_repository),
    chat_service(chat_service)
{}

std::vector<Membership> MembershipService::get_channel_members(String channel_id, int limit, String start_timetoken, String end_timetoken)
{
    String include_string = "totalCount,customFields,channelFields,customChannelFields";

    auto pubnub_handle = this->pubnub->lock();
    String get_channel_members_response = pubnub_handle->get_channel_members(channel_id, include_string, limit, start_timetoken, end_timetoken);

    json response_json = json::parse(get_channel_members_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get members, response is incorrect");
    }

    json users_array_json = response_json["data"];

    auto chat_service_shared = chat_service.lock();

    std::vector<Membership> memberships;
    for (auto& element : users_array_json)
    {
        //Create user entity, as this user maight be not in the repository yet. If it already is there, it will be updated
        UserEntity user_entity = chat_service_shared->user_service->create_domain_from_user_response_data(String(element["uuid"].dump()));
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

    auto pubnub_handle = this->pubnub->lock();
    String get_memberships_response = pubnub_handle->get_memberships(user_id, include_string, limit, start_timetoken, end_timetoken);

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
        ChannelEntity channel_entity = chat_service_shared->channel_service->create_domain_from_channel_response_data(String(element["channel"].dump()));
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
    if(chat_service_shared == nullptr)
    {
        throw std::runtime_error("Can't invite to channel, chat service pointer is invalid");
    }

    Channel channel = chat_service_shared->channel_service->create_presentation_object(channel_id);

    if(channel.channel_data().type == String("public"))
    {
        throw std::runtime_error("Channel invites are not supported in Public chats");
    }

    //TODO:: check here if user already is on that channel. Requires C-Core filtering

    auto pubnub_handle = this->pubnub->lock();

    String include_string = "totalCount,customFields,channelFields,customChannelFields";
    String set_memeberships_obj = create_set_memberships_object(channel_id, "");
    String memberships_response = pubnub_handle->set_memberships(user.user_id(), set_memeberships_obj, include_string);
    
    json memberships_response_json = json::parse(memberships_response);

    String channel_data_string = memberships_response_json["data"][0].dump();

    String event_payload = "{\"channelType\": \"" + channel.channel_data().type + "\", \"channelId\": \"" + channel_id + "\"}";
    chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_INVITE, user.user_id(), event_payload);

    //This channel is updated, so we need to update it in entity repository as well
    ChannelEntity channel_entity = chat_service_shared->channel_service->create_domain_from_channel_response_data(channel_data_string);
    entity_repository->get_channel_entities().update_or_insert(channel_id, channel_entity);
    
    return create_presentation_object(user, channel);
}

std::vector<Membership> MembershipService::invite_multiple_to_channel(String channel_id, std::vector<User> users)
{
    auto chat_service_shared = chat_service.lock();
    if(chat_service_shared == nullptr)
    {
        throw std::runtime_error("Can't invite multiple to channel, chat service pointer is invalid");
    }

    Channel channel = chat_service_shared->channel_service->create_presentation_object(channel_id);

    if(channel.channel_data().type == String("public"))
    {
        throw std::runtime_error("Channel invites are not supported in Public chats");
    }

    //TODO:: check here if users already are on that channel. Requires C-Core filtering

    auto pubnub_handle = this->pubnub->lock();

    std::vector<String> filtered_users_ids;

    for(auto &user : users)
    {
        filtered_users_ids.push_back(user.user_id());
    }

    String include_string = "totalCount,customFields,channelFields,customChannelFields";
    String set_memebers_obj = create_set_members_object(filtered_users_ids, "");
    String set_members_response = pubnub_handle->set_members(channel_id, set_memebers_obj, include_string);
    
    std::vector<Pubnub::Membership> invitees_memberships;

    json memberships_response_json = json::parse(set_members_response);
    json memberships_data_array = memberships_response_json["data"];

    String test = memberships_data_array.dump();
    

    for (json::iterator single_data_json = memberships_data_array.begin(); single_data_json != memberships_data_array.end(); ++single_data_json) 
    {
        User user = chat_service_shared->user_service->create_presentation_object(String(single_data_json.value()["uuid"]["id"]));
        Membership membership = create_presentation_object(user, channel);
        invitees_memberships.push_back(membership);

        String event_payload = "{\"channelType\": \"" + channel.channel_data().type + "\", \"channelId\": \"" + channel_id + "\"}";
        chat_service_shared->emit_chat_event(pubnub_chat_event_type::PCET_INVITE, user.user_id(), event_payload);
    }

    return invitees_memberships;

}

Membership MembershipService::create_presentation_object(User user, Channel channel)
{
    auto chat_service_shared = chat_service.lock();
    if(chat_service_shared == nullptr)
    {
        throw std::runtime_error("Can't create membership object, chat service pointer is invalid");
    }

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
