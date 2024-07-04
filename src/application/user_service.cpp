#include "user_service.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"
#include "nlohmann/json.hpp"

using namespace Pubnub;
using json = nlohmann::json;

UserService::UserService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository, std::weak_ptr<ChatService> chat_service):
    pubnub(pubnub),
    entity_repository(entity_repository),
    chat_service(chat_service)
{}

User UserService::create_user(String user_id, ChatUserData user_data)
{
    if(user_id.empty())
    {
        throw std::invalid_argument("Failed to create user, user_id is empty");
    }

    auto maybe_user= this->entity_repository->get_user_entities().get(user_id);

    if (maybe_user.has_value()) {
        return create_presentation_object(user_id);
    }

    User user = create_presentation_object(user_id);

    UserEntity new_user_entity = create_domain_from_presentation_data(user_id, user_data);

    auto pubnub_handle = this->pubnub->lock();
    pubnub_handle->set_user_metadata(user_id, new_user_entity.get_user_metadata_json_string());

    //Add user_entity to repository
    entity_repository->get_user_entities().update_or_insert(user_id, new_user_entity);

    return user;
}

User UserService::get_user(String user_id)
{
    if(user_id.empty())
    {
        throw std::invalid_argument("Failed to get user, user_id is empty");
    }

    auto pubnub_handle = this->pubnub->lock();
    String user_response = pubnub_handle->get_user_metadata(user_id);

    json response_json = json::parse(user_response);

    UserEntity new_user_entity = create_domain_from_user_response(user_response);
    User user = create_presentation_object(user_id);

    //Add or update user_entity to repository
    entity_repository->get_user_entities().update_or_insert(user_id, new_user_entity);

    return user;
}

std::vector<User> UserService::get_users(Pubnub::String include, int limit, Pubnub::String start, Pubnub::String end)
{
    auto pubnub_handle = this->pubnub->lock();
    String users_response = pubnub_handle->get_all_user_metadata(include, limit, start, end);

    json response_json = json::parse(users_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get users, response is incorrect");
    }

    json user_data_array_json = response_json["data"];
    std::vector<User> users;
   
   for (auto& element : user_data_array_json)
   {
        UserEntity new_user_entity = create_domain_from_user_response_data(String(element.dump()));
        User user = create_presentation_object(String(element["id"]));

        entity_repository->get_user_entities().update_or_insert(String(element["id"]), new_user_entity);

        users.push_back(user);
   }

    return users;
}

User UserService::update_user(String user_id, ChatUserData user_data)
{
    if(user_id.empty())
    {
        throw std::invalid_argument("Failed to update user, user_id is empty");
    }

    User user = create_presentation_object(user_id);

    UserEntity new_user_entity = create_domain_from_presentation_data(user_id, user_data);

    auto pubnub_handle = this->pubnub->lock();
    pubnub_handle->set_user_metadata(user_id, new_user_entity.get_user_metadata_json_string());

    //Add userentity to repository
    entity_repository->get_user_entities().update_or_insert(user_id, new_user_entity);
    
    return user;
}

void UserService::delete_user(String user_id)
{
    if(user_id.empty())
    {
        throw std::invalid_argument("Failed to delete user, user_id is empty");
    }

    auto pubnub_handle = this->pubnub->lock();
    pubnub_handle->remove_user_metadata(user_id);

    //Also remove this user from entities repository
    entity_repository->get_user_entities().remove(user_id);
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

Pubnub::User UserService::create_presentation_object(Pubnub::String user_id)
{
    auto chat_service_shared = chat_service.lock();
    if(chat_service_shared == nullptr)
    {
        throw std::runtime_error("Can't create user object, chat service pointer is invalid");
    }

    return User(user_id, chat_service_shared, shared_from_this());
}

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
UserEntity UserService::create_domain_from_user_response(Pubnub::String json_response)
{
    json user_response_json = json::parse(json_response);

    if(user_response_json.is_null())
    {
        throw std::runtime_error("can't create user from response, response is incorrect");
    }

    json user_data_json = user_response_json["data"][0];

    if(user_data_json.is_null())
    {
        throw std::runtime_error("can't create user from response, response doesn't have data field");
    }

    return create_domain_from_user_response_data(String(user_data_json.dump()));
}

UserEntity UserService::create_domain_from_user_response_data(Pubnub::String json_response_data)
{
    json user_data_json = json::parse(json_response_data);

    if(user_data_json.is_null())
    {
        throw std::runtime_error("can't create user from response data, json_response_data is not a correct json");
    }

    UserEntity new_user_entity;

    if(user_data_json.contains("id") && !user_data_json["id"].is_null())
    {
        new_user_entity.user_id = user_data_json["id"].dump();
    }
    else
    {
        throw std::runtime_error("can't create user from response data, response doesn't have id field");
    }

    if(user_data_json.contains("name") && !user_data_json["name"].is_null())
    {
        new_user_entity.user_name = user_data_json["name"].dump();
    }
    if(user_data_json.contains("externalId") && !user_data_json["externalId"].is_null())
    {
        new_user_entity.external_id = user_data_json["externalId"].dump();
    }
    if(user_data_json.contains("profileUrl") && !user_data_json["profileUrl"].is_null())
    {
        new_user_entity.profile_url = user_data_json["profileUrl"].dump();
    }
    if(user_data_json.contains("email") && !user_data_json["email"].is_null())
    {
        new_user_entity.email = user_data_json["email"].dump();
    }
    if(user_data_json.contains("custom") && !user_data_json["custom"].is_null())
    {
        new_user_entity.custom_data_json = user_data_json["custom"];
    }
    if(user_data_json.contains("status") && !user_data_json["status"].is_null())
    {
        new_user_entity.status = user_data_json["status"].dump();
    }
    if(user_data_json.contains("type") && !user_data_json["type"].is_null())
    {
        new_user_entity.type = user_data_json["type"].dump();
    }

    return new_user_entity;
}

Pubnub::ChatUserData UserService::presentation_data_from_domain(UserEntity &user_entity)
{
    ChatUserData user_data;
    user_data.user_name = user_entity.user_name;
    user_data.external_id = user_entity.external_id;
    user_data.profile_url = user_entity.profile_url;
    user_data.email = user_entity.email;
    user_data.custom_data_json = user_entity.custom_data_json;
    user_data.status = user_entity.status;
    user_data.type = user_entity.type;

    return user_data;
}