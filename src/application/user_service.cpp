#include "user_service.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"
#include "nlohmann/json.hpp"

using namespace Pubnub;
using json = nlohmann::json;

UserService::UserService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository):
    pubnub(pubnub),
    entity_repository(entity_repository)
{}


UserEntity UserService::create_domain_from_presentation_data(Pubnub::String user_id, Pubnub::ChatUserData &presentation_data)
{
    UserEntity new_user_entity;
    new_user_entity.user_id = user_id;
    new_user_entity.user_name = presentation_data.user_name;
    new_user_entity.external_id = presentation_data.external_id;
    new_user_entity.profile_url = presentation_data.profile_url;
    new_user_entity.email = presentation_data.email;
    new_user_entity.custom_data_json = presentation_data.custom_data_json;
    new_user_entity.status = presentation_data.status;
    new_user_entity.type = presentation_data.type;

    return new_user_entity;
}

std::vector<String> UserService::where_present(String user_id)
{
    auto pubnub_handle = this->pubnub->lock();
    String where_now_response = pubnub_handle->where_now(user_id);

    json response_json = json::parse(where_now_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get where present, response is incorrect");
    }

    json response_payload_json = response_json["payload"];
    json channels_array_json = response_payload_json["channels"];

    std::vector<String> channel_ids;
   
    for (json::iterator it = channels_array_json.begin(); it != channels_array_json.end(); ++it) 
    {
        channel_ids.push_back(static_cast<String>(*it));
    }
    
    return channel_ids;
}