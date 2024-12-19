#include "c_message.hpp"
#include "c_thread_channel.hpp"
#include "c_errors.hpp"
#include "nlohmann/json.hpp"
#include "message.hpp"
#include "thread_message.hpp"

void pn_message_delete(Pubnub::Message* message) {
    delete message;
}

Pubnub::Message* pn_message_edit_text(
        Pubnub::Message* message,
        const char* text) {
    try {
        return new Pubnub::Message(message->edit_text(text));
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

PnCResult pn_message_text(
        Pubnub::Message* message,
        char* result) {
    try {
        auto text = message->text();
        strcpy(result, text.c_str());
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

Pubnub::Message* pn_message_delete_message(Pubnub::Message* message) {
    try {
        return new Pubnub::Message(message->delete_message());
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

PnCResult pn_message_delete_message_hard(Pubnub::Message* message) {
    try {
        message->delete_message_hard();
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCTribool pn_message_deleted(Pubnub::Message* message) {
    try {
        return message->deleted() ? PN_C_TRUE : PN_C_FALSE;
    } catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_UNKNOWN;
    }
}

void pn_message_get_timetoken(Pubnub::Message* message, char* result) {
    auto timetoken = message->timetoken();
    strcpy(result, timetoken.c_str());
}

int pn_message_get_data_type(Pubnub::Message* message) {
    return message->message_data().type;
}

void pn_message_get_data_text(Pubnub::Message* message, char* result) {
    auto data_text = message->message_data().text;
    strcpy(result, data_text.c_str());
}

void pn_message_get_data_channel_id(Pubnub::Message* message, char* result) {
    auto data_channel_id = message->message_data().channel_id;
    strcpy(result, data_channel_id.c_str());
}

void pn_message_get_data_user_id(Pubnub::Message* message, char* result) {
    auto data_user_id = message->message_data().user_id;
    strcpy(result, data_user_id.c_str());
}

void pn_message_get_data_meta(Pubnub::Message* message, char* result) {
    auto data_meta = message->message_data().meta;
    strcpy(result, data_meta.c_str());
}

// TODO: utils
static void message_action_to_json(nlohmann::json& j, const Pubnub::MessageAction data) {
    j = nlohmann::json{
        {"timeToken", data.timetoken.c_str()},
        {"type", message_action_type_to_non_quoted_string(data.type).c_str()},
        {"userId", data.user_id.c_str()},
        {"value", data.value.c_str()}
    };
}

// TODO: dont copy code (again)
const char* jsonize_reactions(std::vector<Pubnub::MessageAction> reactions) {
    if (reactions.size() == 0) {
        char* empty_result = new char[3];
        memcpy(empty_result, "[]\0", 3);
        return empty_result;
    }

    Pubnub::String result = "[";
    for (auto reaction : reactions) {
        nlohmann::json json;
        message_action_to_json(json, reaction);
        result += json.dump().c_str();
        result += ",";
    }

    result.erase(result.length() - 1);
    result += "]";

    char* c_result = new char[result.length() + 1];

    memcpy(c_result, result.c_str(), result.length() + 1);

    return c_result;
}

PnCResult pn_message_get_data_message_actions(Pubnub::Message* message, char* result) {
    try {
        auto message_actions = message->message_data().message_actions.into_std_vector();
        auto jsonised = jsonize_reactions(message_actions);
        strcpy(result, jsonised);
        delete[] jsonised;
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_message_pin(Pubnub::Message* message) {
    try {
        message->pin();
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_message_get_reactions(Pubnub::Message* message, char* reactions_json) {
    try {
        auto reactions = message->reactions().into_std_vector();
        auto jsonised = jsonize_reactions(reactions);
        strcpy(reactions_json, jsonised);
        delete[] jsonised;
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

Pubnub::Message* pn_message_toggle_reaction(Pubnub::Message* message, const char* reaction) {
    try {
        return new Pubnub::Message(message->toggle_reaction(reaction));
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

PnCTribool pn_message_has_user_reaction(Pubnub::Message* message, const char* reaction) {
    try {
        return message->has_user_reaction(reaction);
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }
}

PnCResult pn_message_report(Pubnub::Message* message, const char* reason) {
    try {
        message->report(reason);
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

Pubnub::ThreadChannel* pn_message_create_thread(Pubnub::Message* message) {
    try {
        return new Pubnub::ThreadChannel(message->create_thread());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

PnCTribool pn_message_has_thread(Pubnub::Message* message) {
    try {
        return message->has_thread();
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }
}

Pubnub::ThreadChannel* pn_message_get_thread(Pubnub::Message* message) {
    try {
        return new Pubnub::ThreadChannel(message->get_thread());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

PnCResult pn_message_remove_thread(Pubnub::Message* message) {
    try {
        message->remove_thread();
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

Pubnub::Message* pn_message_update_with_base_message(Pubnub::Message* message, Pubnub::Message* base_message) {
    try {
        return new Pubnub::Message(message->update_with_base(*base_message));
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

PnCResult pn_message_mentioned_users(Pubnub::Message* message, Pubnub::Chat* chat, char* result) {
    try {
        auto mentioned_users = message->mentioned_users();
        std::vector<intptr_t> user_pointers;
        for (auto mentioned_user : mentioned_users)
        {
            try
            {
                auto user = chat->get_user(mentioned_user.id);
                auto ptr = new Pubnub::User(user);
                user_pointers.push_back(reinterpret_cast<intptr_t>(ptr));
            }
            catch (std::exception& e)
            {
                std::cout << "Wasn't able to get the mentioned user!" << std::endl;
            }
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
PnCResult pn_message_referenced_channels(Pubnub::Message* message, Pubnub::Chat* chat, char* result) {
    try {
        auto referenced_channels = message->referenced_channels();
        std::vector<intptr_t> channel_pointers;
        for (auto referenced_channel : referenced_channels)
        {
            try
            {
                auto channel = chat->get_channel(referenced_channel.id);
                auto ptr = new Pubnub::Channel(channel);
                channel_pointers.push_back(reinterpret_cast<intptr_t>(ptr));
            }
            catch (std::exception& e)
            {
                std::cout << "Wasn't able to get the referenced channel!" << std::endl;
            }
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
Pubnub::Message* pn_message_quoted_message(Pubnub::Message* message) {
    try {
        return new Pubnub::Message(message->quoted_message().value());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

PnCResult pn_message_text_links(Pubnub::Message* message, char* result) {
    try {
        auto text_links = message->text_links();
        std::vector<nlohmann::json> link_jsons;
        for (auto link : text_links) {
            link_jsons.push_back(nlohmann::json{
                {"StartIndex", link.start_index},
                {"EndIndex", link.end_index},
                {"Link", link.link.c_str()},
                });
        }
        auto j = nlohmann::json{
                {"value", link_jsons}
        };
        strcpy(result, j.dump().c_str());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

Pubnub::Message* pn_message_restore(Pubnub::Message* message) {
    try {
        return new Pubnub::Message(message->restore());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}


