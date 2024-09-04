#include "user_service.hpp"
#include "application/dao/user_dao.hpp"
#include "chat_service.hpp"
#include "domain/timetoken.hpp"
#include "infra/pubnub.hpp"
#include "infra/entity_repository.hpp"
#include "nlohmann/json.hpp"
#include "callback_service.hpp"
#include <chrono>
#include <memory>
#include <optional>

using namespace Pubnub;
using json = nlohmann::json;

UserService::UserService(ThreadSafePtr<PubNub> pubnub, std::weak_ptr<ChatService> chat_service, int store_user_active_interval):
    pubnub(pubnub),
    chat_service(chat_service),
    store_user_active_interval(store_user_active_interval)
{}

User UserService::get_current_user() const
{
    auto user_id = [this] {
        auto pubnub_handle = pubnub->lock();
        return pubnub_handle->get_user_id();
    }();

    // TODO: it should be saved in the UserService
    return this->get_user(user_id);
}

User UserService::create_user(const String& user_id, const UserDAO& user_data) const
{
    ;
    if(user_id.empty())
    {
        throw std::invalid_argument("Failed to create user, user_id is empty");
    }

    bool user_exists = true;

    try
    {
        get_user(user_id);

    }
    catch(...)
    {
        user_exists = false;
    }

    if(user_exists)
    {
        throw std::invalid_argument("User with this ID already exists");
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
    UserEntity new_user_entity = UserEntity::from_user_response(response_json["data"]);

    return this->create_user_object({user_id, UserDAO(new_user_entity)});
}

std::tuple<std::vector<Pubnub::User>, Pubnub::Page, int> UserService::get_users(const Pubnub::String &filter, const Pubnub::String &sort, int limit, const Pubnub::Page &page) const {
    
    Pubnub::String include = "custom,totalCount";
    auto users_response = [this, include, limit, filter, sort, page] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_all_user_metadata(include, limit, filter, sort, page.next, page.prev);
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

    int total_count = response_json.get_int("totalCount").value_or(0);
    Page page_response({response_json.get_string("next").value_or(String("")), response_json.get_string("prev").value_or(String(""))});
    std::tuple<std::vector<Pubnub::User>, Pubnub::Page, int> return_tuple = std::make_tuple(users, page_response, total_count);

    return return_tuple;
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

std::function<void()> UserService::stream_updates(Pubnub::User calling_user, std::function<void(const Pubnub::User)> user_callback) const
{

    auto pubnub_handle = this->pubnub->lock();

    auto chat = this->chat_service.lock();

    std::vector<String> users_ids;
    std::function<void(User)> final_user_callback = [=](User user){
        auto updated_user = this->update_user_with_base(user, calling_user);
       
        user_callback(updated_user);
    };
    
    auto messages = pubnub_handle->subscribe_to_multiple_channels_and_get_messages({calling_user.user_id()});
    chat->callback_service->broadcast_messages(messages);
    chat->callback_service->register_user_callback(calling_user.user_id(), final_user_callback);

    //stop streaming callback
    std::function<void()> stop_streaming = [=](){
        chat->callback_service->remove_user_callback(calling_user.user_id());
    };

    return stop_streaming;

}

std::function<void()> UserService::stream_updates_on(Pubnub::User calling_user, const std::vector<Pubnub::User>& users, std::function<void(std::vector<Pubnub::User>)> user_callback) const
{
    if(users.empty())
    {
        throw std::invalid_argument("Cannot stream user updates on an empty list");
    }

    auto pubnub_handle = this->pubnub->lock();

    auto chat = this->chat_service.lock();
    std::vector<String> users_ids;

    std::function<void(User)> single_user_callback = [=](User user){
        
        std::vector<Pubnub::User> updated_users; 

        for(int i = 0; i < users.size(); i++)
        {
            //Find user that was updated and replace it in Entity stream users
            auto stream_user = users[i];

            if(stream_user.user_id() == user.user_id())
            {
                UserEntity stream_user_entity = UserDAO(stream_user.user_data()).to_entity();
                UserEntity user_entity = UserDAO(user.user_data()).to_entity();
                std::pair<String, UserEntity> pair = std::make_pair(user.user_id(), UserEntity::from_base_and_updated_user(stream_user_entity, user_entity));
                auto updated_user = create_user_object(pair);
                updated_users.push_back(updated_user);
            }
            else
            {
                updated_users.push_back(users[i]);
            }
        }
        user_callback(updated_users);

    };
    
    for(auto user : users)
    {
        users_ids.push_back(user.user_id());
        chat->callback_service->register_user_callback(user.user_id(), single_user_callback);
    }
    

    auto messages = pubnub_handle->subscribe_to_multiple_channels_and_get_messages(users_ids);
    chat->callback_service->broadcast_messages(messages);

    //stop streaming callback
    std::function<void()> stop_streaming = [=, &users_ids](){
        for(auto id : users_ids)
        {
            chat->callback_service->remove_user_callback(id);
        }
    };

    return stop_streaming;
}

std::vector<Pubnub::User> UserService::get_users_suggestions(Pubnub::String text, int limit) const
{
    auto chat_shared = this->chat_service.lock();

    if(!chat_shared)
    {
        throw std::runtime_error("can't get users suggestions, chat service is invalid");
    }

    String cache_key = chat_shared->message_service->get_phrase_to_look_for(text);

    if(cache_key.empty())
    {
        return {};
    }

    //TODO:: cashe rezults here like in js

    String filter = "name LIKE \"" + cache_key + "*\"";

    auto get_users_tuple =  this->get_users(filter, "", limit);
    return std::get<0>(get_users_tuple);
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
            std::make_unique<UserDAO>(user_data.second)
       );
    }

    throw std::runtime_error("Failed to create user object, chat service is not available");
}

User UserService::update_user_with_base(const User& user, const User& base_user) const {
    auto user_data = UserDAO(user.user_data()).to_entity();
    auto base_user_data = UserDAO(base_user.user_data()).to_entity();
    auto updated_user_data = UserEntity::from_base_and_updated_user(base_user_data, user_data);

    return this->create_user_object({user.user_id(), UserDAO(updated_user_data)});
}

bool UserService::active(const UserDAO& user_data) const {
    return user_data.to_entity().is_active(this->store_user_active_interval);
}

Pubnub::Option<Pubnub::String> UserService::last_active_timestamp(const UserDAO& user_data) const {
    return user_data.to_entity().get_last_active_timestamp();
}

void UserService::store_user_activity_timestamp() const {
    if (this->lastSavedActivityInterval.has_value()) {
        this->lastSavedActivityInterval.reset();
    }

    const auto user = this->get_current_user();
    auto user_data = user.data->to_entity();

    if (!user.last_active_timestamp().has_value()) {
        this->run_save_timestamp_interval(user_data);
        return;
    }

    const auto current_time = Timetoken::now_numeric();
    const auto elapsed_time_since_last_check = current_time - Timetoken::to_long(user.last_active_timestamp().value());

    if (elapsed_time_since_last_check >= this->store_user_active_interval) {
        this->run_save_timestamp_interval(user_data);
        return;
    }

    const auto remaining_time = this->store_user_active_interval - elapsed_time_since_last_check;

    // TODO: this is very buggy solution as it will always override the custom data of the user...
    Timer().start(remaining_time, [this, user_data] {
        this->run_save_timestamp_interval(user_data);
    });
}

void UserService::run_save_timestamp_interval(UserEntity user_entity) const {
    this->save_timestamp_function(user_entity);

    this->lastSavedActivityInterval.emplace(
        [this, user_entity] {
            this->save_timestamp_function(user_entity);
        },
        this->store_user_active_interval
    );
}

void UserService::save_timestamp_function(UserEntity user_entity) const {
        auto pubnub_handle = this->pubnub->lock();

        user_entity.set_last_active_timestamp(Timetoken::now());

        auto user_id = pubnub_handle->get_user_id();
        // TODO: set this metadata
        pubnub_handle->set_user_metadata(user_id, user_entity.get_user_metadata_json_string(user_id));
}
