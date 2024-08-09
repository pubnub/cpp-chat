#include "c_channel.hpp"
#include "application/dao/channel_dao.hpp"
#include "chat.hpp"
#include "domain/channel_entity.hpp"
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
        Pubnub::User* user,
        char* result
        ) {
    try {
        auto restrictions = channel->get_user_restrictions(*user);
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
        const char* filter,
        const char* sort,
        const int limit,
        const char* next,
        const char* prev,
        char* result) {
    try {
        auto members_wrapper = channel->get_members(filter, sort, limit, Pubnub::Page({next, prev}));

        std::vector<intptr_t> membership_pointers;
        for (auto member : members_wrapper.memberships) {
            auto ptr = new Pubnub::Membership(member);
            membership_pointers.push_back(reinterpret_cast<intptr_t>(ptr));
        }   

        auto j = nlohmann::json{
            {"memberships", membership_pointers},
            {"total", members_wrapper.total},
            {"page", nlohmann::json{
                        {"prev", members_wrapper.page.prev.c_str()},
                        {"next", members_wrapper.page.next.c_str()},
                    }
            },
            {"status", members_wrapper.status.c_str()}
        };

        strcpy(result, j.dump().c_str());
        
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

PnCResult pn_channel_emit_user_mention(Pubnub::Channel* channel, const char* user_id, const char* timetoken, const char* text) {
    try {
        std::cout << "ONE" << std::endl;
        channel->emit_user_mention(user_id, timetoken, text);
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

Pubnub::Channel* pn_channel_update_with_base(Pubnub::Channel* channel, Pubnub::Channel* base_channel) {
    try {
        return new Pubnub::Channel(channel->update_with_base(*base_channel));
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR_PTR;
    }
}

PnCResult pn_channel_get_user_suggestions(
        Pubnub::Channel* channel, 
        const char* text, 
        int limit, 
        char* result) {
    try {
        auto members = channel->get_user_suggestions(text, limit);
        std::vector<intptr_t> membership_pointers;
        for (auto membership : members)
        {
            auto ptr = new Pubnub::Membership(membership);
            membership_pointers.push_back(reinterpret_cast<intptr_t>(ptr));
        }
        auto j = nlohmann::json{
                {"value", membership_pointers}
        };
        strcpy(result, j.dump().c_str());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_channel_send_text_dirty(
        Pubnub::Channel* channel,
        const char* message,
        bool store_in_history,
        bool send_by_post,
        const char* meta,
        int mentioned_users_length,
        int* mentioned_users_indexes,
        Pubnub::User** mentioned_users,
        int referenced_channels_length,
        int* referenced_channels_indexes,
        Pubnub::Channel** referenced_channels,
        const char* text_links_json,
        Pubnub::Message* quoted_message) {
    try {
        
        Pubnub::SendTextParams params;

        //The easy ones
        params.store_in_history = store_in_history;
        params.send_by_post = send_by_post;
        params.meta = meta;

        //Mentioned users
        std::map<int, Pubnub::MentionedUser> mentioned_users_map;
        for (int i = 0; i < mentioned_users_length; i++)
        {
            auto user = *mentioned_users[i];
            auto mentioned_user = Pubnub::MentionedUser();
            mentioned_user.id = user.user_id();
            mentioned_user.name = user.user_data().user_name;
            mentioned_users_map.emplace(std::make_pair(mentioned_users_indexes[i], mentioned_user));
        }
        params.mentioned_users = mentioned_users_map;

        //Referenced channels
        std::map<int, Pubnub::ReferencedChannel> referenced_channels_map;
        for (int i = 0; i < referenced_channels_length; i++)
        {
            auto channel = *referenced_channels[i];
            auto referenced_channel = Pubnub::ReferencedChannel();
            referenced_channel.id = channel.channel_id();
            referenced_channel.name = channel.channel_data().channel_name;
            referenced_channels_map.emplace(std::make_pair(referenced_channels_indexes[i], referenced_channel));
        }
        params.referenced_channels = referenced_channels_map;

        //Text links
        auto parsed_links_json = nlohmann::json::parse(text_links_json);
        Pubnub::Vector<Pubnub::TextLink> links;
        for (auto link_json : parsed_links_json) {
            Pubnub::TextLink link;
            link.start_index = link_json["StartIndex"];
            link.end_index = link_json["EndIndex"];
            link.link = link_json["Link"];
            links.push_back(link);
        }
        params.text_links = links;

        //Quoted message
        if (quoted_message == nullptr)
        {
            params.quoted_message = Pubnub::Option<Pubnub::Message>::none();
        }
        else {
            params.quoted_message = Pubnub::Option<Pubnub::Message>(*quoted_message);
        }

        channel->send_text(message, params);
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_channel_get_users_restrictions(Pubnub::Channel* channel, const char* sort, int limit, const char* next, const char* prev, char* result) {
    try {
        auto wrapper = channel->get_users_restrictions(sort, limit, Pubnub::Page({next, prev}));
        std::vector<nlohmann::json> restrictions;
        for (auto restriction : wrapper.restrictions)
        {
            //TODO: put into util
            auto restriction_json = nlohmann::json{
                {"ban", restriction.ban},
                {"mute", restriction.mute},
                {"reason", restriction.reason.c_str()},
                {"userId", restriction.user_id.c_str()},
            };
            restrictions.push_back(restriction_json);
        }
        auto j = nlohmann::json{
                {"Restrictions", restrictions},
                {"Page", nlohmann::json{
                        {"Previous", wrapper.page.prev.c_str()},
                        {"Next", wrapper.page.next.c_str()},
                    }
                },
                {"Total", wrapper.total},
                {"Status", wrapper.status.c_str()},
        };
        strcpy(result, j.dump().c_str());
    }
    catch (std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
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
