#include "c_channel.hpp"
#include "chat.hpp"
#include "message.hpp"
#include "message_draft.hpp"
#include "c_errors.hpp"
#include "nlohmann/json.hpp"
#include "membership.hpp"
#include "restrictions.hpp"
#include <iostream>
#include <sstream>
#include "application/channel_service.hpp"

// TODO: utils module for c ABI
const char* move_message_to_heap2(std::vector<pubnub_v2_message> messages) {
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



void pn_channel_delete(Pubnub::Channel* channel) {
    delete channel;
}

Pubnub::Channel* pn_channel_new_dirty(
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

Pubnub::Channel* pn_channel_update_dirty(
        Pubnub::Channel* channel,
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
        return new Pubnub::Channel(channel->update(converted_data));
    } catch(std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

// TODO: dont copy code
const char* jsonize_messages2(std::vector<Pubnub::String> messages) {
    if (messages.size() == 0) {
        char* empty_result = new char[3];
        memcpy(empty_result, "[]\0", 3);
        return empty_result;
    }
    
    Pubnub::String result = "[";
    for (auto message : messages) {
        result += message;
    }   

    result.erase(result.length() - 1);
    result += "]";
    
    char* c_result = new char[result.length() + 1];

    memcpy(c_result, result.c_str(), result.length() + 1);

    return c_result;
}

// TODO: dont copy code
const char* jsonize_messages3(std::vector<Pubnub::String> messages) {
    if (messages.size() == 0) {
        char* empty_result = new char[3];
        memcpy(empty_result, "[]\0", 3);
        return empty_result;
    }
    
    Pubnub::String result = "[";
    for (auto message : messages) {
        result += "\"";
        result += message;
        result += "\",";
    }   

    result.erase(result.length() - 1);
    result += "]";
    
    char* c_result = new char[result.length() + 1];

    memcpy(c_result, result.c_str(), result.length() + 1);

    return c_result;
}



PnCResult pn_channel_connect(Pubnub::Channel* channel, char* result_messages) {
    try {
        auto messages = channel->connect();
        auto heaped = move_message_to_heap2(messages);
        strcpy(result_messages, heaped);
        delete[] heaped;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_channel_disconnect(Pubnub::Channel* channel, char* result_messages) {
    try {
        auto messages = channel->disconnect();
        auto heaped = move_message_to_heap2(messages);
        strcpy(result_messages, heaped);
        delete[] heaped;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_channel_join(Pubnub::Channel* channel, const char* additional_params, char* result_messages) {
    try {
        auto messages = channel->join(additional_params);
        auto heaped = move_message_to_heap2(messages);
        strcpy(result_messages, heaped);
        delete[] heaped;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_channel_leave(Pubnub::Channel* channel, char* result_messages) {
    try {
        auto messages = channel->leave();
        auto heaped = move_message_to_heap2(messages);
        delete[] heaped;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_channel_delete_channel(Pubnub::Channel* channel) {
    try {
        channel->delete_channel();
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_channel_set_restrictions(
        Pubnub::Channel* channel,
        const char* user_id,
        bool ban,
        bool mute,
        const char* reason
        ) {
    Pubnub::Restriction restrictions;
    restrictions.ban = ban;
    restrictions.mute = mute;
    restrictions.reason = reason;

    try {
        channel->set_restrictions(user_id, restrictions);
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_channel_send_text(
    Pubnub::Channel* channel,
    const char* message,
    Pubnub::pubnub_chat_message_type type,
    const char* metadata
    ) {
    try {
        channel->send_text(message, type, metadata);
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

void pn_channel_get_channel_id(
        Pubnub::Channel* channel,
        char* result
        ) {
    auto channel_id = channel->channel_id();
    strcpy(result, channel_id.c_str());
}

void pn_channel_get_data_channel_name(
        Pubnub::Channel* channel,
        char* result
        ) {
    auto channel_name = channel->channel_data().channel_name;
    strcpy(result, channel_name.c_str());
}

void pn_channel_get_data_description(
        Pubnub::Channel* channel,
        char* result
        ) {
    auto description = channel->channel_data().description;
    strcpy(result, description.c_str());
}

void pn_channel_get_data_custom_data_json(
        Pubnub::Channel* channel,
        char* result
        ) {
    auto custom_data_json = channel->channel_data().custom_data_json;
    strcpy(result, custom_data_json.c_str());
}

void pn_channel_get_data_updated(
        Pubnub::Channel* channel,
        char* result
        ) {
    auto updated = channel->channel_data().updated;
    strcpy(result, updated.c_str());
}

void pn_channel_get_data_status(
        Pubnub::Channel* channel,
        char* result
        ) {
    auto status = channel->channel_data().status;
    strcpy(result, status.c_str());
}

void pn_channel_get_data_type(
        Pubnub::Channel* channel,
        char* result
        ) {
    auto type = channel->channel_data().type;
    strcpy(result, type.c_str());
}

PnCTribool pn_channel_is_present(Pubnub::Channel* channel, const char* user_id) {
    try {
        return channel->is_present(user_id);
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }
}

PnCResult pn_channel_who_is_present(Pubnub::Channel* channel, char* result) {
    try {
        auto present = channel->who_is_present().into_std_vector();
        auto jsonised = jsonize_messages3(present);
        strcpy(result, jsonised);
        delete[] jsonised;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

// TODO: utils
static void restrictions_to_json(nlohmann::json& j, const Pubnub::Restriction& data) {
    j = nlohmann::json{
        {"ban", data.ban},
        {"mute", data.mute},
        {"reason", data.reason.c_str()}
    };
}

PnCResult pn_channel_get_user_restrictions(
        Pubnub::Channel* channel,
        const char* user_id,
        const char* channel_id,
        int limit,
        const char* start,
        const char* end,
        char* result
        ) {
    try {
        auto restrictions = channel->get_user_restrictions(user_id, channel_id, limit, start, end);
        nlohmann::json json;
        restrictions_to_json(json, restrictions);


        strcpy(result, json.dump().c_str());
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_channel_get_members(
        Pubnub::Channel* channel,
        int limit,
        const char* start,
        const char* end,
        char* result) {
    try {
        auto members = channel->get_members(limit, start, end);

        if (members.size() == 0) {
            memcpy(result, "[]\0", 3);
            return PN_C_OK;
        }

        Pubnub::String string = "[";
        for (auto member : members) {
            auto ptr = new Pubnub::Membership(member);
            // TODO: utils void* to string
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

        strcpy(result, string.c_str());
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_channel_get_history(
        Pubnub::Channel* channel,
        const char* start,
        const char* end,
        int count,
        char* result) {
    try {
        auto history = channel->get_history(start, end, count);

        if (history.size() == 0) {
            memcpy(result, "[]\0", 3);
            return PN_C_OK;
        }

        Pubnub::String string = "[";
        for (auto message : history) {
            auto ptr = new Pubnub::Message(message);
            // TODO: utils void* to string
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

        memcpy(result, string.c_str(), string.length() + 1);
   } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

Pubnub::Message* pn_channel_get_message(
        Pubnub::Channel* channel,
        const char* timetoken) {
    try {
        return new Pubnub::Message(channel->get_message(timetoken));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

Pubnub::Membership* pn_channel_invite_user(Pubnub::Channel* channel, Pubnub::User* user) {
    try {
        return new Pubnub::Membership(channel->invite(*user));
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

PnCResult pn_channel_invite_multiple(Pubnub::Channel* channel, Pubnub::User** users, int users_length, char* result_json) {
    try {
        std::vector<Pubnub::User> users_vector;
        users_vector.reserve(users_length);
        for (int i = 0; i < users_length; i++)
        {
            users_vector.push_back(*users[i]);
        }
        auto memberships = channel->invite_multiple(std::move(users_vector));
        
        if (memberships.size() == 0) {
            memcpy(result_json, "[]\0", 3);
            return PN_C_OK;
        }

        Pubnub::String string = "[";
        for (auto membership : memberships) {
            auto ptr = new Pubnub::Membership(membership);
            // TODO: utils void* to string
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

PnCResult pn_channel_start_typing(Pubnub::Channel* channel) {
    try {
        channel->start_typing();
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_channel_stop_typing(Pubnub::Channel* channel) {
    try {
        channel->stop_typing();
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

Pubnub::Channel* pn_channel_pin_message(Pubnub::Channel* channel, Pubnub::Message* message) {
    try {
        return new Pubnub::Channel(channel->pin_message(*message));
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

Pubnub::Channel* pn_channel_unpin_message(Pubnub::Channel* channel) {
    try {
        return new Pubnub::Channel(channel->unpin_message());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

Pubnub::Message* pn_channel_get_pinned_message(Pubnub::Channel* channel) {
    try {
        return new Pubnub::Message(channel->get_pinned_message());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

//Pubnub::MessageDraft* pn_channel_create_message_draft_dirty(Pubnub::Channel* channel, 
//    char* user_suggestion_source, 
//    bool is_typing_indicator_triggered, 
//    int user_limit,
//    int channel_limit)
//{
//    try {
//        Pubnub::MessageDraftConfig config;
//        config.user_suggestion_source = user_suggestion_source;
//        config.is_typing_indicator_triggered = is_typing_indicator_triggered;
//        config.user_limit = user_limit;
//        config.channel_limit = channel_limit;
//        
//        return new Pubnub::MessageDraft(channel->create_message_draft(config));
//    }
//    catch (std::exception& e) {
//        pn_c_set_error_message(e.what());
//
//        return PN_C_ERROR_PTR;
//    }
//}
