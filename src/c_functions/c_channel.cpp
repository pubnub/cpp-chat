#include "c_functions/c_channel.hpp"
#include "callbacks.hpp"
#include "chat.hpp"
#include "chat/message.hpp"
#include "c_functions/c_errors.hpp"
#include "nlohmann/json.hpp"
#include "chat/membership.hpp"
#include <iostream>
#include <sstream>

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
        result += ",";
    }   

    result.erase(result.length() - 1);
    result += "]";
    
    char* c_result = new char[result.length() + 1];

    memcpy(c_result, result.c_str(), result.length() + 1);

    return c_result;
}


PnCResult pn_channel_connect(Pubnub::Channel* channel, char* messages_json) {
    try {
        auto messages = channel->connect_and_get_messages();
        auto jsonised = jsonize_messages2(messages);
        strcpy(messages_json, jsonised);
        delete[] jsonised;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_channel_disconnect(Pubnub::Channel* channel) {
    try {
        channel->disconnect();
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_channel_join(Pubnub::Channel* channel, CallbackStringFunction callback) {
    try {
        channel->join(callback);
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_channel_leave(Pubnub::Channel* channel) {
    try {
        channel->leave();
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
    Pubnub::PubnubRestrictionsData restrictions;
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
    auto channel_id = channel->get_channel_id();
    strcpy(result, channel_id.c_str());
}

void pn_channel_get_data_channel_name(
        Pubnub::Channel* channel,
        char* result
        ) {
    auto channel_name = channel->get_channel_data().channel_name;
    strcpy(result, channel_name.c_str());
}

void pn_channel_get_data_description(
        Pubnub::Channel* channel,
        char* result
        ) {
    auto description = channel->get_channel_data().description;
    strcpy(result, description.c_str());
}

void pn_channel_get_data_custom_data_json(
        Pubnub::Channel* channel,
        char* result
        ) {
    auto custom_data_json = channel->get_channel_data().custom_data_json;
    strcpy(result, custom_data_json.c_str());
}

void pn_channel_get_data_updated(
        Pubnub::Channel* channel,
        char* result
        ) {
    auto updated = channel->get_channel_data().updated;
    strcpy(result, updated.c_str());
}

void pn_channel_get_data_status(
        Pubnub::Channel* channel,
        char* result
        ) {
    auto status = channel->get_channel_data().status;
    strcpy(result, status.c_str());
}

void pn_channel_get_data_type(
        Pubnub::Channel* channel,
        char* result
        ) {
    auto type = channel->get_channel_data().type;
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
        auto present = channel->who_is_present();
        auto jsonised = jsonize_messages2(present);
        strcpy(result, jsonised);
        delete[] jsonised;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

// TODO: utils
static void restrictions_to_json(nlohmann::json& j, const Pubnub::PubnubRestrictionsData& data) {
    j = nlohmann::json{
        {"ban", data.ban},
        {"mute", data.mute},
        {"reason", data.reason}
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

   } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}
