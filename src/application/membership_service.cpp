#include "membership_service.hpp"
#include "application/chat_service.hpp"
#include "application/user_service.hpp"
#include "application/channel_service.hpp"
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


    std::vector<Membership> memberships;

    for (auto& element : users_array_json)
    {
        auto chat_service_shared = chat_service.lock();
        if(chat_service_shared == nullptr)
        {
            throw std::runtime_error("Can't get channel members, chat service pointer is invalid");
        }
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

Membership MembershipService::create_presentation_object(User user, Channel channel)
{
    auto chat_service_shared = chat_service.lock();
    if(chat_service_shared == nullptr)
    {
        throw std::runtime_error("Can't create membership object, chat service pointer is invalid");
    }

    return Membership(user, channel, channel.channel_data().custom_data_json, chat_service_shared, shared_from_this());
}
