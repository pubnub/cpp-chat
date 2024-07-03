#include "channel_service.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"
#include "nlohmann/json.hpp"

using namespace Pubnub;
using json = nlohmann::json;

ChannelService::ChannelService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository):
    pubnub(pubnub),
    entity_repository(entity_repository)
{}

Channel ChannelService::create_public_conversation(String channel_id, ChatChannelData data)
{
    data.type = "public";
    return create_channel(channel_id, data);
}

Channel ChannelService::create_channel(String channel_id, ChatChannelData data) {

    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to create channel, channel_id is empty");
    }

    auto maybe_channel = this->entity_repository->get_channel_entities().get(channel_id);

    if (maybe_channel.has_value()) {
        return Channel(shared_from_this(), channel_id);
    }

    Channel channel = Channel(shared_from_this(), channel_id);

    ChannelEntity new_channel_entity = create_domain_from_presentation_data(channel_id, data);

    auto pubnub_handle = this->pubnub->lock();
    pubnub_handle->set_channel_metadata(channel_id, new_channel_entity.get_channel_metadata_json_string());

    //Add channel_entity to repository
    entity_repository->get_channel_entities().update_or_insert(channel_id, new_channel_entity);
    
    return channel;
}

Channel ChannelService::get_channel(String channel_id)
{
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to get channel, channel_id is empty");
    }

    //We don't try to get this channel from entity repository here, as channel data could be updated on the server

    auto pubnub_handle = this->pubnub->lock();
    String channel_response = pubnub_handle->get_channel_metadata(channel_id);

    ChannelEntity new_channel_entity = create_domain_from_channel_response(channel_response);
    Channel channel = Channel(shared_from_this(), channel_id);

    //Add or update channel_entity to repository
    entity_repository->get_channel_entities().update_or_insert(channel_id, new_channel_entity);

    return channel;
}

std::vector<Channel> ChannelService::get_channels(String include, int limit, String start, String end)
{
    auto pubnub_handle = this->pubnub->lock();
    String channels_response = pubnub_handle->get_all_channels_metadata(include, limit, start, end);

    json response_json = json::parse(channels_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get channels, response is incorrect");
    }

    json channel_data_array_json = response_json["data"];
    std::vector<Channel> Channels;
   
   for (auto& element : channel_data_array_json)
   {
        ChannelEntity new_channel_entity = create_domain_from_channel_response_data(String(element.dump()));
        Channel channel = Channel(shared_from_this(), String(element["id"]));

        entity_repository->get_channel_entities().update_or_insert(String(element["id"]), new_channel_entity);

        Channels.push_back(channel);
   }

    return Channels;
}

Channel ChannelService::update_channel(String channel_id, ChatChannelData channel_data)
{
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to update channel, channel_id is empty");
    }

    Channel channel = Channel(shared_from_this(), channel_id);

    ChannelEntity new_channel_entity = create_domain_from_presentation_data(channel_id, channel_data);

    auto pubnub_handle = this->pubnub->lock();
    pubnub_handle->set_channel_metadata(channel_id, new_channel_entity.get_channel_metadata_json_string());

    //Add channel_entity to repository
    entity_repository->get_channel_entities().update_or_insert(channel_id, new_channel_entity);
    
    return channel;
}

void ChannelService::delete_channel(String channel_id)
{
    if(channel_id.empty())
    {
        throw std::invalid_argument("Failed to delete channel, channel_id is empty");
    }
    
    auto pubnub_handle = this->pubnub->lock();
    pubnub_handle->remove_channel_metadata(channel_id);

    //Also remove this channel from entities repository
    entity_repository->get_channel_entities().remove(channel_id);
}

ChannelEntity ChannelService::create_domain_from_presentation_data(String channel_id, ChatChannelData &presentation_data)
{
    ChannelEntity new_channel_entity;
    new_channel_entity.channel_id = channel_id;
    new_channel_entity.channel_name = presentation_data.channel_name;
    new_channel_entity.description = presentation_data.description;
    new_channel_entity.custom_data_json = presentation_data.custom_data_json;
    new_channel_entity.updated = presentation_data.updated;
    new_channel_entity.status = presentation_data.status;
    new_channel_entity.type = presentation_data.type;

    return new_channel_entity;
}

ChannelEntity ChannelService::create_domain_from_channel_response(Pubnub::String json_response)
{
    json channel_response_json = json::parse(json_response);

    if(channel_response_json.is_null())
    {
        throw std::runtime_error("can't create channel from response, response is incorrect");
    }

    json channel_data_json = channel_response_json["data"][0];

    if(channel_data_json.is_null())
    {
        throw std::runtime_error("can't create channel from response, response doesn't have data field");
    }

    return create_domain_from_channel_response_data(String(channel_data_json.dump()));
}

ChannelEntity ChannelService::create_domain_from_channel_response_data(Pubnub::String json_response_data)
{
    json channel_data_json = json::parse(json_response_data);

    if(channel_data_json.is_null())
    {
        throw std::runtime_error("can't create channel from response data, json_response_data is not a correct json");
    }

    ChannelEntity new_channel_entity;

    if(channel_data_json.contains("id") && !channel_data_json["id"].is_null())
    {
        new_channel_entity.channel_id = channel_data_json["id"].dump();
    }
    else
    {
        throw std::runtime_error("can't create channel from response data, response doesn't have id field");
    }

    if(channel_data_json.contains("name") && !channel_data_json["name"].is_null())
    {
        new_channel_entity.channel_name = channel_data_json["name"].dump();
    }
    if(channel_data_json.contains("description") && !channel_data_json["description"].is_null())
    {
        new_channel_entity.description = channel_data_json["description"].dump();
    }
    if(channel_data_json.contains("custom") && !channel_data_json["custom"].is_null())
    {
        new_channel_entity.custom_data_json = channel_data_json["custom"].dump();
    }
    if(channel_data_json.contains("updated") && !channel_data_json["updated"].is_null())
    {
        new_channel_entity.updated = channel_data_json["updated"].dump();
    }
    if(channel_data_json.contains("status") && !channel_data_json["status"].is_null())
    {
        new_channel_entity.status = channel_data_json["status"].dump();
    }
    if(channel_data_json.contains("type") && !channel_data_json["type"].is_null())
    {
        new_channel_entity.type = channel_data_json["type"].dump();
    }

    return new_channel_entity;
}
