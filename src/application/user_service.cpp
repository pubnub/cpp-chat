#include "user_service.hpp"
#include "chat_service.hpp"
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

Pubnub::ChatUserData UserService::get_user_data(Pubnub::String user_id)
{
    auto maybe_user = this->entity_repository->get_user_entities().get(user_id);

    if (!maybe_user.has_value()) 
    {
        throw std::invalid_argument("Failed to get user data, there is no channel with this id");
    }

    return presentation_data_from_domain(maybe_user.value());
}

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

    Json response_json = Json::parse(user_response);

    UserEntity new_user_entity = UserEntity::from_json(user_response);
    User user = create_presentation_object(user_id);

    //Add or update user_entity to repository
    entity_repository->get_user_entities().update_or_insert(user_id, new_user_entity);

    return user;
}

std::vector<User> UserService::get_users(String include, int limit, String start, String end)
{
    auto pubnub_handle = this->pubnub->lock();
    String users_response = pubnub_handle->get_all_user_metadata(include, limit, start, end);

    Json response_json = Json::parse(users_response);

    if(response_json.is_null())
    {
        throw std::runtime_error("can't get users, response is incorrect");
    }

    Json user_data_array_json = response_json["data"];
    std::vector<User> users;
   
   for (auto element : user_data_array_json)
   {
        UserEntity new_user_entity = UserEntity::from_json(element.dump());
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

void UserService::stream_updates_on(std::vector<User> users, std::function<void(User)> user_callback)
{
    if(users.empty())
    {
        throw std::invalid_argument("Cannot stream user updates on an empty list");
    }

    auto pubnub_handle = this->pubnub->lock();

    for(auto user : users)
    {
        //TODO:: CALLBACK register user callback here
        pubnub_handle->subscribe_to_channel(user.user_id());
    }
}

User UserService::create_presentation_object(String user_id)
{
    auto chat_service_shared = chat_service.lock();
    if(chat_service_shared == nullptr)
    {
        throw std::runtime_error("Can't create user object, chat service pointer is invalid");
    }

    return User(user_id, chat_service_shared, shared_from_this(), chat_service_shared->presence_service, chat_service_shared->restrictions_service, chat_service_shared->membership_service);
}

UserEntity UserService::create_domain_from_presentation_data(String user_id, ChatUserData &presentation_data)
{
    UserEntity new_user_entity;
    new_user_entity.user_name = presentation_data.user_name;
    new_user_entity.external_id = presentation_data.external_id;
    new_user_entity.profile_url = presentation_data.profile_url;
    new_user_entity.email = presentation_data.email;
    new_user_entity.custom_data_json = presentation_data.custom_data_json;
    new_user_entity.status = presentation_data.status;
    new_user_entity.type = presentation_data.type;

    return new_user_entity;
}

ChatUserData UserService::presentation_data_from_domain(UserEntity &user_entity)
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

User UserService::create_user_object(std::pair<String, UserEntity> user_data)
{
    if (auto chat = this->chat_service.lock()) {
        this->entity_repository
            ->get_user_entities()
            .update_or_insert(user_data);

        return User(
            user_data.first,
            chat,
            shared_from_this(),
            chat->presence_service,
            chat->restrictions_service,
            chat->membership_service
       );
    }

    throw std::runtime_error("Failed to create user object, chat service is not available");
}
