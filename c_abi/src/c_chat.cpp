#include "c_chat.hpp"
#include "c_channel.hpp"
#include "c_response.hpp"
#include "callback_handle.hpp"
#include "channel.hpp"
#include "c_errors.hpp"
#include "application/enum_converters.hpp"
#include "chat.hpp"
#include "event.hpp"
#include "string.hpp"
#include "restrictions.hpp"
#include "nlohmann/json.hpp"
#include <pubnub_helper.h>
#include <string>
#include <sstream>

// TODO: add config
Pubnub::Chat* pn_chat_new(
        const char* publish,
        const char* subscribe,
        const char* user_id,
        const char* auth_key,
        int typing_timeout,
        int typing_timeout_difference,
        int store_user_activity_interval,
        bool store_user_activity_timestamps,
        float rate_limit_factor,
        int direct_conversation_rate_limit,
        int group_conversation_rate_limit,
        int public_conversation_rate_limit,
        int unknown_conversation_rate_limit
    ) {

    try {
        auto rate_limits_per_channel = Pubnub::ChannelRateLimits();
        rate_limits_per_channel.direct_conversation = direct_conversation_rate_limit;
        rate_limits_per_channel.group_conversation = group_conversation_rate_limit;
        rate_limits_per_channel.public_conversation = public_conversation_rate_limit;
        rate_limits_per_channel.unknown_conversation = unknown_conversation_rate_limit;
        
        Pubnub::ChatConfig config; 
            config.auth_key = auth_key;
            config.typing_timeout = typing_timeout;
            config.typing_timeout_difference = typing_timeout_difference;
            config.store_user_activity_interval = store_user_activity_interval;
            config.store_user_activity_timestamps = store_user_activity_timestamps;
            config.rate_limit_factor = rate_limit_factor;
            config.rate_limit_per_channel = rate_limits_per_channel;

        auto* chat = new Pubnub::Chat(Pubnub::Chat::init(publish, subscribe, user_id, config));
        return chat;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

void pn_chat_delete(Pubnub::Chat* chat) {
    if (chat == nullptr) {
        return;
    }
    delete chat;
}

Pubnub::Channel* pn_chat_create_public_conversation_dirty(
        Pubnub::Chat* chat,
        const char* channel_id,
        char* channel_name,
        char* channel_description,
        char* channel_custom_data_json,
        char* channel_updated,
        char* channel_status,
        char* channel_type
        ) {
    Pubnub::ChatChannelData converted_data; 
    converted_data.channel_name = channel_name;
    converted_data.description = channel_description;
    converted_data.custom_data_json = channel_custom_data_json;
    converted_data.updated = channel_updated;
    converted_data.status = channel_status;
    converted_data.type = channel_type;

    try {
        return new Pubnub::Channel(chat->create_public_conversation(channel_id, converted_data));
    } catch(std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

Pubnub::Channel* pn_chat_update_channel_dirty(
        Pubnub::Chat* chat,
        const char* channel_id,
        char* channel_name,
        char* channel_description,
        char* channel_custom_data_json,
        char* channel_updated,
        char* channel_status,
        char* channel_type) {
    Pubnub::ChatChannelData converted_data; 
    converted_data.channel_name = channel_name;
    converted_data.description = channel_description;
    converted_data.custom_data_json = channel_custom_data_json;
    converted_data.updated = channel_updated;
    converted_data.status = channel_status;
    converted_data.type = channel_type;

    try {
        return new Pubnub::Channel(chat->update_channel(channel_id, converted_data));
    } catch(std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

Pubnub::Channel* pn_chat_get_channel(
        Pubnub::Chat* chat,
        const char* channel_id) {
    try {
        return new Pubnub::Channel(chat->get_channel(channel_id));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

PnCResult pn_chat_delete_channel(
        Pubnub::Chat* chat,
        const char* channel_id) {
    try {
        chat->delete_channel(channel_id);
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_chat_set_restrictions(
        Pubnub::Chat* chat,
        const char* user_id,
        const char* channel_id,
        bool ban, 
        bool mute,
        const char* reason) {
    Pubnub::Restriction restrictions;
    restrictions.ban = ban;
    restrictions.mute= mute;
    restrictions.reason = reason;

    try {
        chat->set_restrictions(user_id, channel_id, restrictions);
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

Pubnub::User* pn_chat_create_user_dirty(
        Pubnub::Chat* chat,
        const char* user_id,
        const char* user_name,
        const char* external_id,
        const char* profile_url,
        const char* email,
        const char* custom_data_json,
        const char* status,
        const char* type) {
    Pubnub::ChatUserData converted_data; 
    converted_data.user_name = user_name;
    converted_data.external_id = external_id;
    converted_data.profile_url = profile_url;
    converted_data.email = email;
    converted_data.custom_data_json = custom_data_json;
    converted_data.status = status;
    converted_data.type = type;
    
    try {
        return new Pubnub::User(chat->create_user(user_id, converted_data));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

Pubnub::User* pn_chat_get_user(
        Pubnub::Chat* chat,
        const char* user_id) {
    try {
        return new Pubnub::User(chat->get_user(user_id));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

//std::vector<Pubnub::User>* pn_chat_get_users(
//        Pubnub::Chat* chat,
//        const char* include,
//        const int limit,
//        const char* start,
//        const char* end) {
//    return ->get_users(include, limit, start, end);
//}

Pubnub::User* pn_chat_update_user_dirty(
        Pubnub::Chat* chat,
        const char* user_id,
        const char* user_name,
        const char* external_id,
        const char* profile_url,
        const char* email,
        const char* custom_data_json,
        const char* status,
        const char* type) {
    Pubnub::ChatUserData converted_data; 
    converted_data.user_name = user_name;
    converted_data.external_id = external_id;
    converted_data.profile_url = profile_url;
    converted_data.email = email;
    converted_data.custom_data_json = custom_data_json;
    converted_data.status = status;
    converted_data.type = type;
    
    try {
        return new Pubnub::User(chat->update_user(user_id, converted_data));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

PnCResult pn_chat_delete_user(
        Pubnub::Chat* chat,
        const char* user_id) {
    try {
        chat->delete_user(user_id);
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

const char* move_message_to_heap(std::vector<pubnub_v2_message> messages) {
    if (messages.size() == 0) {
        char* empty_result = new char[3];
        memcpy(empty_result, "[]\0", 3);
        return empty_result;
    }
    Pubnub::String result = "[";
    for (auto message : messages) {
        auto ptr = new pubnub_v2_message(message);
        // TODO: utils void* to string
#ifdef _WIN32
        result += "0x";
#endif
        std::ostringstream oss;
        oss << static_cast<void*>(ptr);
        result += oss.str();
        result += ",";
    }
    result.erase(result.length() - 1);
    result += "]";
    char* c_result = new char[result.length() + 1];
    memcpy(c_result, result.c_str(), result.length() + 1);
    return c_result;
}

void pn_clear_string(char* str) {
    if (nullptr == str) {
        return;
    }

    delete[] str;
}

PnCTribool pn_chat_is_present(
        Pubnub::Chat* chat,
        const char* user_id,
        const char* channel_id) {
    try {
        return chat->is_present(user_id, channel_id);
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }
}

PnCResult pn_chat_who_is_present(
        Pubnub::Chat* chat,
        const char* channel_id,
        char* result) {
    try {
        auto people = chat->who_is_present(channel_id);

        if (people.size() == 0) {
            strcpy(result, "[]");
            return PN_C_OK;
        }

        Pubnub::String string = "[";
        for (auto person : people) {
            string += person;
            string += ",";
        }

        string.erase(string.length() - 1);
        string += "]";

        strcpy(result, string.c_str());

        return PN_C_OK;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }
}

PnCResult pn_chat_get_users(
        Pubnub::Chat* chat,
        const char* filter,
        const char* sort,
        const int limit,
        const char* next,
        const char* prev,
        char* result) {
    try {
        auto usersWrapper = chat->get_users(filter, sort, limit, Pubnub::Page({ next, prev }));

        std::vector<intptr_t> users_pointers;
        for (auto user : usersWrapper.users) {
            auto ptr = new Pubnub::User(user);
            users_pointers.push_back(reinterpret_cast<intptr_t>(ptr));
        }

        auto j = nlohmann::json{
            {"users", users_pointers},
            {"total", usersWrapper.total},
            {"page", nlohmann::json{
                        {"prev", usersWrapper.page.prev.c_str()},
                        {"next", usersWrapper.page.next.c_str()},
                    }
            } 
        };

        strcpy(result, j.dump().c_str());
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }
        
    return PN_C_OK;
}

PnCResult pn_chat_get_channels(
        Pubnub::Chat* chat,
        const char* filter,
        const char* sort,
        const int limit,
        const char* next,
        const char* prev,
        char* result) {
    try {
        auto channels_wrapper = chat->get_channels(filter, sort, limit, Pubnub::Page({next, prev}));

        std::vector<intptr_t> channel_pointers;
        for (auto channel : channels_wrapper.channels) {
            auto ptr = new Pubnub::Channel(channel);
            channel_pointers.push_back(reinterpret_cast<intptr_t>(ptr));
        }

        auto j = nlohmann::json{
            {"channels", channel_pointers},
            {"total", channels_wrapper.total},
            {"page", nlohmann::json{
                        {"prev", channels_wrapper.page.prev.c_str()},
                        {"next", channels_wrapper.page.next.c_str()},
                    }
            }
        };

        strcpy(result, j.dump().c_str());

    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }
        
    return PN_C_OK;
}

Pubnub::CallbackHandle* pn_chat_listen_for_events(
        Pubnub::Chat* chat,
        const char* channel_id,
        Pubnub::pubnub_chat_event_type event_type) {
    try {
        auto chat_service = chat->shared_chat_service();

        return new Pubnub::CallbackHandle(chat->listen_for_events(
                channel_id,
                event_type,
                [chat_service](const Pubnub::Event& event) {
                Pubnub::String event_str("{\"event\":");

                    auto j = nlohmann::json{
                            {"timetoken", event.timetoken.c_str()},
                            {"type", event.type},
                            {"channelId", event.channel_id.c_str()},
                            {"userId", event.user_id.c_str()},
                            {"payload", event.payload.c_str()}
                    };

                    event_str += j.dump().c_str();
                    event_str += "}";

                    pn_c_append_to_response_buffer(chat_service.get(), event_str.c_str());
                }));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

Pubnub::CreatedChannelWrapper* pn_chat_create_direct_conversation_dirty(
    Pubnub::Chat* chat,
    Pubnub::User* user,
    const char* channel_id,
    char* channel_name,
    char* channel_description,
    char* channel_custom_data_json,
    char* channel_updated,
    char* channel_status,
    char* channel_type
) {
    Pubnub::ChatChannelData converted_data;
    converted_data.channel_name = channel_name;
    converted_data.description = channel_description;
    converted_data.custom_data_json = channel_custom_data_json;
    converted_data.updated = channel_updated;
    converted_data.status = channel_status;
    converted_data.type = channel_type;

    try {
        return new Pubnub::CreatedChannelWrapper(chat->create_direct_conversation(*user, channel_id, converted_data));
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

Pubnub::CreatedChannelWrapper* pn_chat_create_group_conversation_dirty(
    Pubnub::Chat* chat,
    Pubnub::User** users,
    int users_length,
    const char* channel_id,
    char* channel_name,
    char* channel_description,
    char* channel_custom_data_json,
    char* channel_updated,
    char* channel_status,
    char* channel_type
) {
    Pubnub::ChatChannelData converted_data;
    converted_data.channel_name = channel_name;
    converted_data.description = channel_description;
    converted_data.custom_data_json = channel_custom_data_json;
    converted_data.updated = channel_updated;
    converted_data.status = channel_status;
    converted_data.type = channel_type;

    try {

        std::vector<Pubnub::User> users_vector;
        users_vector.reserve(users_length);
        for (int i = 0; i < users_length; i++)
        {
            users_vector.push_back(*users[i]);
        }

        return new Pubnub::CreatedChannelWrapper(chat->create_group_conversation(Pubnub::Vector(std::move(users_vector)), channel_id, converted_data));
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

Pubnub::Channel* pn_chat_get_created_channel_wrapper_channel(
    Pubnub::CreatedChannelWrapper* wrapper) {
    return new Pubnub::Channel(wrapper->created_channel);
}

Pubnub::Membership* pn_chat_get_created_channel_wrapper_host_membership(
    Pubnub::CreatedChannelWrapper* wrapper) {
    return new Pubnub::Membership(wrapper->host_membership);
}

PnCResult pn_chat_get_created_channel_wrapper_invited_memberships(
    Pubnub::CreatedChannelWrapper* wrapper, char* result_json) {
    try {
        auto memberships = wrapper->invitees_memberships;

        if (memberships.size() == 0) {
            memcpy(result_json, "[]\0", 3);
            return PN_C_OK;
        }

        Pubnub::String string = "[";
        for (auto membership : memberships) {
            auto ptr = new Pubnub::Membership(membership);
#ifdef _WIN32
            string += "0x";
#endif
            std::ostringstream oss;
            oss << static_cast<void*>(ptr);
            string += oss.str();
            string += ",";
        }

        string.erase(string.length() - 1);
        string += "]";

        memcpy(result_json, string.c_str(), string.length() + 1);
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

void pn_chat_dispose_created_channel_wrapper(
    Pubnub::CreatedChannelWrapper* wrapper) {
    delete wrapper;
}

PnCResult pin_message_to_channel(Pubnub::Chat* chat, Pubnub::Message* message, Pubnub::Channel* channel) {
    try {
        chat->pin_message_to_channel(*message, *channel);
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}
PnCResult unpin_message_from_channel(Pubnub::Chat* chat, Pubnub::Channel* channel) {
    try {
        chat->unpin_message_from_channel(*channel);
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_chat_forward_message(Pubnub::Chat* chat, Pubnub::Message* message, Pubnub::Channel* channel) {
    try {
        chat->forward_message(*message, *channel);
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_chat_emit_event(Pubnub::Chat* chat, Pubnub::pubnub_chat_event_type chat_event_type, const char* channel_id, const char* payload) {
    try {
        chat->emit_chat_event(chat_event_type, channel_id, payload);
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_chat_get_unread_messages_counts(
        Pubnub::Chat* chat, 
        const char* filter, 
        const char* sort, 
        int limit, 
        const char* next, 
        const char* prev,
        char* result) {
    try {
        auto wrappers = chat->get_unread_messages_counts(filter, sort, limit, Pubnub::Page({next, prev}));
        
        Pubnub::String wrappers_json = "[";
        for (auto wrapper : wrappers) {
            auto channel_ptr = new Pubnub::Channel(wrapper.channel);
            auto membership_ptr = new Pubnub::Membership(wrapper.membership);

            auto j = nlohmann::json{
                {"membership", reinterpret_cast<intptr_t>(membership_ptr)},
                {"channel", reinterpret_cast<intptr_t>(channel_ptr)},
                {"count", wrapper.count}
            };

            wrappers_json += j.dump().c_str();
            wrappers_json += ",";
        }

        if (wrappers_json.length() > 1) {
            wrappers_json.erase(wrappers_json.length() - 1);
        }
        wrappers_json += "]";

        strcpy(result, wrappers_json.c_str());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_chat_get_channel_suggestions(Pubnub::Chat* chat, const char* text, int limit, char* result) {
    try {
        auto suggestions = chat->get_channel_suggestions(text, limit);
        std::vector<intptr_t> channel_pointers;
        for (auto suggestion : suggestions)
        {
            auto ptr = new Pubnub::Channel(suggestion);
            channel_pointers.push_back(reinterpret_cast<intptr_t>(ptr));
        }
        auto j = nlohmann::json{
                {"value", channel_pointers}
        };
        strcpy(result, j.dump().c_str());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_chat_mark_all_messages_as_read(
        Pubnub::Chat* chat, 
        const char* filter, 
        const char* sort, 
        int limit, 
        const char* next, 
        const char* prev, 
        char* result) {
    try {
        auto wrapper = chat->mark_all_messages_as_read(filter, sort, limit, Pubnub::Page({next, prev}));
        std::vector<intptr_t> membership_pointers;
        for (auto membership : wrapper.memberships)
        {
            auto ptr = new Pubnub::Membership(membership);
            membership_pointers.push_back(reinterpret_cast<intptr_t>(ptr));
        }
        auto j = nlohmann::json{
                {"memberships", membership_pointers},
                {"total", wrapper.total},
                {"status", wrapper.status},
                {"page", nlohmann::json{
                        {"prev", wrapper.page.prev.c_str()},
                        {"next", wrapper.page.next.c_str()},
                    }
            }
        };
        strcpy(result, j.dump().c_str());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_chat_get_events_history(Pubnub::Chat* chat, const char* channel_id, const char* start_timetoken, const char* end_timetoken, int count, char* result) {
    try {
        auto wrapper = chat->get_events_history(channel_id, start_timetoken, end_timetoken, count);
        std::vector<nlohmann::json> events;
        for (auto chat_event : wrapper.events)
        {
            //TODO: put into util
            auto event_j = nlohmann::json{
                {"timeToken", chat_event.timetoken.c_str()},
                {"type", chat_event_type_to_string(chat_event.type).c_str()},
                {"userId", chat_event.user_id.c_str()},
                {"channelId", chat_event.channel_id.c_str()},
                {"payload", chat_event.payload.c_str()}
            };
            events.push_back(event_j);
        }
        auto j = nlohmann::json{
                {"events", events},
                {"isMore", wrapper.is_more},
        };
        strcpy(result, j.dump().c_str());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_chat_get_current_user_mentions(Pubnub::Chat *chat, const char *start_timetoken, const char *end_timetoken, int count, char *result) {
    try {
        auto mentions = chat->get_current_user_mentions(start_timetoken, end_timetoken, count);
        std::vector<nlohmann::json> mentions_json;
        for (auto mention : mentions.user_mention_data)
        {
            //TODO: put into util
            auto event_json = nlohmann::json{
                {"timeToken", mention.event.timetoken.c_str()},
                {"type", chat_event_type_to_string(mention.event.type).c_str()},
                {"userId", mention.event.user_id.c_str()},
                {"channelId", mention.event.channel_id.c_str()},
                {"payload", mention.event.payload.c_str()}
            };

            auto msg_ptr = new Pubnub::Message(mention.message);

            auto mention_json = nlohmann::json {
                {"channelId", mention.channel_id.c_str()},
                {"userId", mention.user_id.c_str()},
                {"event", event_json},
                {"message", reinterpret_cast<intptr_t>(msg_ptr)},
           };

           if (mention.thread_channel_id.has_value() && mention.parent_channel_id.has_value())
           {
               mention_json["threadChannelId"] = mention.thread_channel_id.value().c_str();
               mention_json["parentChannelId"] = mention.parent_channel_id.value().c_str();
           }

           mentions_json.push_back(mention_json);
        }

        auto json = nlohmann::json {
            {"userMentionData", mentions_json},
            {"isMore", mentions.is_more}
        };

        strcpy(result, json.dump().c_str());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_chat_get_user_suggestions(Pubnub::Chat* chat, char* text, int limit, char* result) {
    try {
        auto users = chat->get_user_suggestions(text, limit);
        std::vector<intptr_t> user_pointers;
        for (auto user : users)
        {
            auto ptr = new Pubnub::User(user);
            user_pointers.push_back(reinterpret_cast<intptr_t>(ptr));
        }
        auto j = nlohmann::json{
                {"value", user_pointers}
        };
        strcpy(result, j.dump().c_str());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

Pubnub::User* pn_chat_current_user(Pubnub::Chat* chat) {
    try {
        return new Pubnub::User(chat->current_user());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

PnCTribool pn_pam_can_i(Pubnub::Chat *chat, Pubnub::AccessManager::Permission permission, Pubnub::AccessManager::ResourceType resource_type, const char *resource_name) {
    try {
        return chat->access_manager().can_i(permission, resource_type, resource_name) ? PN_C_TRUE : PN_C_FALSE;
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_UNKNOWN;
    }
}

PnCResult pn_pam_parse_token(Pubnub::Chat* chat, const char* token, char* result) {
    try {
        auto parsed = chat->access_manager().parse_token(token);
        strcpy(result, parsed.c_str());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_pam_set_auth_token(Pubnub::Chat* chat, const char* token) {
    try {
        chat->access_manager().set_auth_token(token);
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_pam_set_pubnub_origin(Pubnub::Chat* chat, const char* origin) {
    try {
        chat->access_manager().set_pubnub_origin(origin);
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

