#include "user_service.hpp"

#include <chrono>
#include <memory>
#include <optional>

#include "application/dao/user_dao.hpp"
#include "application/subscription.hpp"
#include "callback_service.hpp"
#include "chat_service.hpp"
#include "domain/timetoken.hpp"
#include "infra/entity_repository.hpp"
#include "infra/pubnub.hpp"
#include "nlohmann/json.hpp"

using namespace Pubnub;
using json = nlohmann::json;

UserService::UserService(
    ThreadSafePtr<PubNub> pubnub,
    std::weak_ptr<ChatService> chat_service,
    int store_user_active_interval
) :
    pubnub(pubnub),
    chat_service(chat_service),
    store_user_active_interval(store_user_active_interval) {}

User UserService::get_current_user() const {
    auto user_id = [this] {
        auto pubnub_handle = pubnub->lock();
        return pubnub_handle->get_user_id();
    }();

    // TODO: it should be saved in the UserService
    return this->get_user(user_id);
}

User UserService::create_user(const String& user_id, const UserDAO& user_data, bool skip_get_user)
    const {
    if (user_id.empty()) {
        throw std::invalid_argument("Failed to create user, user_id is empty");
    }

    if (!skip_get_user) {
        bool user_exists = true;

        try {
            get_user(user_id);

        } catch (...) {
            user_exists = false;
        }

        if (user_exists) {
            throw std::invalid_argument("User with this ID already exists");
        }
    }

    auto new_user_entity = user_data.to_entity();
    {
        auto pubnub_handle = this->pubnub->lock();
        pubnub_handle->set_user_metadata(
            user_id,
            new_user_entity.get_user_metadata_json_string(user_id)
        );
    }

    return this->create_user_object({user_id, user_data});
}

User UserService::get_user(const String& user_id) const {
    if (user_id.empty()) {
        throw std::invalid_argument("Failed to get user, user_id is empty");
    }

    std::cout << "Getting user...1" << std::endl;
    auto user_response = [this, user_id] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle->get_user_metadata(user_id);
    }();
    std::cout << "Getting user...2" << std::endl;

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

std::tuple<std::vector<Pubnub::User>, Pubnub::Page, int> UserService::get_users(
    const Pubnub::String& filter,
    const Pubnub::String& sort,
    int limit,
    const Pubnub::Page& page
) const {
    if (limit < 0 || limit > PN_MAX_LIMIT) {
        throw std::invalid_argument(
            "can't get users, limit has to be within 0 - " + std::to_string(PN_MAX_LIMIT) + " range"
        );
    }

    Pubnub::String include = "custom,totalCount";
    auto users_response = [this, include, limit, filter, sort, page] {
        auto pubnub_handle = this->pubnub->lock();
        return pubnub_handle
            ->get_all_user_metadata(include, limit, filter, sort, page.next, page.prev);
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

    std::transform(
        user_entities.begin(),
        user_entities.end(),
        std::back_inserter(users),
        [this](auto user_entity) { return this->create_user_object(user_entity); }
    );

    int total_count = response_json.get_int("totalCount").value_or(0);
    Page page_response(
        {response_json.get_string("next").value_or(String("")),
         response_json.get_string("prev").value_or(String(""))}
    );
    std::tuple<std::vector<Pubnub::User>, Pubnub::Page, int> return_tuple =
        std::make_tuple(users, page_response, total_count);

    return return_tuple;
}

User UserService::update_user(const String& user_id, const UserDAO& user_data) const {
    if (user_id.empty()) {
        throw std::invalid_argument("Failed to update user, user_id is empty");
    }

    {
        auto pubnub_handle = this->pubnub->lock();
        pubnub_handle->set_user_metadata(
            user_id,
            user_data.to_entity().get_user_metadata_json_string(user_id)
        );
    }

    return this->create_user_object({user_id, user_data});
}

void UserService::delete_user(const String& user_id) const {
    if (user_id.empty()) {
        throw std::invalid_argument("Failed to delete user, user_id is empty");
    }

    auto pubnub_handle = this->pubnub->lock();
    pubnub_handle->remove_user_metadata(user_id);
}

std::shared_ptr<Subscription> UserService::stream_updates(
    Pubnub::User calling_user,
    std::function<void(const Pubnub::User)> user_callback
) const {
    auto subscription = this->pubnub->lock()->subscribe(calling_user.user_id());

    auto callback_service = this->chat_service.lock()->callback_service;
    subscription->add_user_update_listener(callback_service->to_c_user_update_callback(
        calling_user,
        this->shared_from_this(),
        user_callback
    ));

    return subscription;
}

std::shared_ptr<SubscriptionSet> UserService::stream_updates_on(
    Pubnub::User calling_user,
    const std::vector<Pubnub::User>& users,
    std::function<void(std::vector<Pubnub::User>)> user_callback
) const {
    if (users.empty()) {
        throw std::invalid_argument("Cannot stream user updates on an empty list");
    }

    std::vector<Pubnub::String> users_ids;

    std::transform(
        users.begin(),
        users.end(),
        std::back_inserter(users_ids),
        [](const Pubnub::User& channel) { return channel.user_id(); }
    );

    auto subscription = this->pubnub->lock()->subscribe_multiple(users_ids);

    auto callback_service = this->chat_service.lock()->callback_service;
    subscription->add_channel_update_listener(
        callback_service->to_c_users_updates_callback(users, shared_from_this(), user_callback)
    );

    return subscription;
}

std::vector<Pubnub::User> UserService::get_users_suggestions(Pubnub::String text, int limit) const {
    if (limit < 0 || limit > PN_MAX_LIMIT) {
        throw std::invalid_argument(
            "can't get users suggestions, limit has to be within 0 - "
            + std::to_string(PN_MAX_LIMIT) + " range"
        );
    }

    auto chat_shared = this->chat_service.lock();

    if (!chat_shared) {
        throw std::runtime_error("can't get users suggestions, chat service is invalid");
    }

    String cache_key = chat_shared->message_service->get_phrase_to_look_for(text);

    if (cache_key.empty()) {
        return {};
    }

    //TODO:: cashe rezults here like in js

    String filter = "name LIKE \"" + cache_key + "*\"";

    auto get_users_tuple = this->get_users(filter, "", limit);
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
    const auto elapsed_time_since_last_check =
        current_time - Timetoken::to_long(user.last_active_timestamp().value());

    if (elapsed_time_since_last_check >= this->store_user_active_interval) {
        this->run_save_timestamp_interval(user_data);
        return;
    }

    const auto remaining_time = this->store_user_active_interval - elapsed_time_since_last_check;

    this->save_timestamp_timer =
        Timer(remaining_time, [this, user_data] { this->run_save_timestamp_interval(user_data); });
}

void UserService::run_save_timestamp_interval(UserEntity user_entity) const {
    this->save_timestamp_function(user_entity);

    this->lastSavedActivityInterval.emplace(
        [this, user_entity] { this->save_timestamp_function(user_entity); },
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
