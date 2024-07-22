#include "user_service.hpp"
#include "application/dao/user_dao.hpp"
#include "chat_service.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"
#include "nlohmann/json.hpp"
#include "callback_service.hpp"

using namespace Pubnub;
using json = nlohmann::json;

UserService::UserService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository, std::weak_ptr<ChatService> chat_service):
    pubnub(pubnub),
    entity_repository(entity_repository),
    chat_service(chat_service)
{}

User UserService::get_current_user() const
{
    auto user_id = [this] {
        auto pubnub_handle = pubnub->lock();
        return pubnub_handle->get_user_id();
    }();

    return this->create_user_object({user_id, UserDAO()});
}

User UserService::create_user(const String& user_id, const UserDAO& user_data) const
{
    if(user_id.empty())
    {
        throw std::invalid_argument("Failed to create user, user_id is empty");
    }

    auto new_user_entity = user_data.to_entity();

    {
        auto pubnub_handle = this->pubnub->lock();
        pubnub_handle->set_user_metadata(user_id, new_user_entity.get_user_metadata_json_string(user_id));
    }

    return this->create_user_object({user_id, user_data});
}

User UserService::get_user(const String& user_id) const
{
    if(user_id.empty()) {
        throw std::invalid_argument("Failed to get user, user_id is empty");
    }

    auto user_response = [this, user_id] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_user_metadata(user_id);
    }();

    Json response_json = Json::parse(user_response);

    if (response_json.is_null()) {
        throw std::runtime_error("can't get user, response is incorrect");
    }

    if (response_json["data"].is_null()) {
        throw std::runtime_error("can't get user, response doesn't have data field");
    }

    //In most responses this data field is an array but in some cases (for example in get_channel) it's just an object.
    UserEntity new_user_entity = UserEntity::from_user_response(response_json);

    return this->create_user_object({user_id, UserDAO(new_user_entity)});
}

std::vector<User> UserService::get_users(const String& include, int limit, const String& start, const String& end) const {
    auto users_response = [this, include, limit, start, end] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_all_user_metadata(include, limit, start, end);
    }();

    Json response_json = Json::parse(users_response);

    if (response_json.is_null()) {
        throw std::runtime_error("can't get users, response is incorrect");
    }

    if (response_json["data"].is_null()) {
        throw std::runtime_error("can't get users, response doesn't have data field");
    }

    std::vector<User> users;
    auto user_entities = UserEntity::from_user_list_response(response_json);

    std::transform(user_entities.begin(), user_entities.end(), std::back_inserter(users), [this](auto user_entity) {
        return this->create_user_object(user_entity);
    });

    return users;
}

User UserService::update_user(const String& user_id, const UserDAO& user_data) const {
    if(user_id.empty())
    {
        throw std::invalid_argument("Failed to update user, user_id is empty");
    }

    {
        auto pubnub_handle = this->pubnub->lock();
        pubnub_handle->set_user_metadata(
                user_id, user_data.to_entity().get_user_metadata_json_string(user_id));
    }

    return this->create_user_object({user_id, user_data});
}

void UserService::delete_user(const String& user_id) const {
    if(user_id.empty())
    {
        throw std::invalid_argument("Failed to delete user, user_id is empty");
    }

    auto pubnub_handle = this->pubnub->lock();
    pubnub_handle->remove_user_metadata(user_id);
}

void UserService::stream_updates_on(const std::vector<User>& users, std::function<void(const User&)> user_callback) const
{
    if(users.empty())
    {
        throw std::invalid_argument("Cannot stream user updates on an empty list");
    }

    auto pubnub_handle = this->pubnub->lock();

#ifndef PN_CHAT_C_ABI
    if (auto chat = this->chat_service.lock()) {
#endif
        for(auto user : users)
        {
            auto messages = pubnub_handle->subscribe_to_channel_and_get_messages(user.user_id());

            // TODO: C ABI way
#ifndef PN_CHAT_C_ABI
            chat->callback_service->broadcast_messages(messages);
            chat->callback_service->register_user_callback(user.user_id(), user_callback);
        }
#endif
    }
}

User UserService::create_user_object(std::pair<String, UserDAO> user_data) const {
    if (auto chat = this->chat_service.lock()) {
        return User(
            user_data.first,
            chat,
            shared_from_this(),
            chat->presence_service,
            chat->restrictions_service,
            chat->membership_service,
            std::make_unique(user_data.second)
       );
    }

    throw std::runtime_error("Failed to create user object, chat service is not available");
}
