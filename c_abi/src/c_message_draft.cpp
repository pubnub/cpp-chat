#include "c_message_draft.hpp"
#include "c_errors.hpp"
#include "domain/json.hpp"
#include "message_draft.hpp"
#include <exception>
#include <vector>

void pn_message_draft_delete(Pubnub::MessageDraft* message_draft) {
    delete message_draft;
}

PnCResult pn_message_draft_insert_text(Pubnub::MessageDraft* message_draft, size_t position, const char* text) {
    try {
        message_draft->insert_text(position, text);
    } catch (const std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_message_draft_remove_text(Pubnub::MessageDraft* message_draft, size_t position, size_t length) {
    try {
        message_draft->remove_text(position, length);
    } catch (const std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

static Pubnub::MentionTarget deserialize_mention_target(const char* target_json) {
    auto parsed_target = Json::parse(target_json);
    auto target_type = parsed_target.get_int("type").value();
    auto target_value = parsed_target.get_string("target").value();

    return target_type == PN_MESSAGE_DRAFT_MENTION_TARGET_TYPE_USER 
        ? Pubnub::MentionTarget::user(target_value) 
        : target_type == PN_MESSAGE_DRAFT_MENTION_TARGET_TYPE_CHANNEL 
            ? Pubnub::MentionTarget::channel(target_value) 
            : Pubnub::MentionTarget::url(target_value);
}

PnCResult pn_message_draft_insert_suggested_mention(Pubnub::MessageDraft *message_draft, std::size_t offset, const char *replace_from, const char *replace_to, const char *target_json, const char* text) {
    try {
        auto mention = Pubnub::SuggestedMention {
            offset,
            replace_from,
            replace_to,
            deserialize_mention_target(target_json)
        };
        message_draft->insert_suggested_mention(mention, text);
    } catch (const std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_message_draft_add_mention(Pubnub::MessageDraft* message_draft, size_t start, size_t length, const char* target) {
    try {
        message_draft->add_mention(start, length, deserialize_mention_target(target));
    } catch (const std::exception& e) {
        pn_c_set_error_message(e.what());
        
        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_message_draft_remove_mention(Pubnub::MessageDraft* message_draft, size_t start) {
    try {
        message_draft->remove_mention(start);
    } catch (const std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_message_draft_update(Pubnub::MessageDraft* message_draft, const char* text) {
    try {
        message_draft->update(text);
    } catch (const std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

//TODO: move the creation of SendTextParams to a util, repeated in c_channel
PnCResult pn_message_draft_send(Pubnub::MessageDraft* message_draft, 
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

        message_draft->send(params);
    } catch (const std::exception& e) {
        pn_c_set_error_message(e.what());

        return PN_C_ERROR;
    }

    return PN_C_OK;
}

PnCResult pn_message_draft_consume_callback_data(Pubnub::MessageDraft *message_draft, char *data) {
    try {
        auto message_elements = message_draft->consume_message_elements();
        std::vector<nlohmann::json> elements_json;
        for (const auto& element : message_elements) {
            auto element_json = nlohmann::json {
                {"text", element.text.c_str() != nullptr ? element.text.c_str() : ""},
                {"mentionTarget", element.target.has_value()
                    ? nlohmann::json {
                        {"type", element.target.value().get_type() == Pubnub::MentionTarget::Type::USER
                            ? PN_MESSAGE_DRAFT_MENTION_TARGET_TYPE_USER
                            : element.target.value().get_type() == Pubnub::MentionTarget::Type::CHANNEL
                                ? PN_MESSAGE_DRAFT_MENTION_TARGET_TYPE_CHANNEL
                                : PN_MESSAGE_DRAFT_MENTION_TARGET_TYPE_URL
                        },
                        {"target", element.target.value().get_target().c_str() ? element.target.value().get_target().c_str() : ""}
                    }
                    : nullptr
                }
            };
            elements_json.push_back(element_json);
        }
        auto suggested_mentions = message_draft->consume_suggested_mentions();
        std::vector<nlohmann::json> mentions_json;
        for (const auto& mention : suggested_mentions) {
            auto mention_json = nlohmann::json {
                {"offset", mention.offset},
                {"replaceFrom", mention.replace_from.c_str() ? mention.replace_from.c_str() : ""},
                {"replaceTo", mention.replace_to.c_str() ? mention.replace_to.c_str() : ""},
                {"mentionTarget", nlohmann::json {
                    {"type", mention.target.get_type() == Pubnub::MentionTarget::Type::USER
                        ? PN_MESSAGE_DRAFT_MENTION_TARGET_TYPE_USER
                        : mention.target.get_type() == Pubnub::MentionTarget::Type::CHANNEL
                            ? PN_MESSAGE_DRAFT_MENTION_TARGET_TYPE_CHANNEL
                            : PN_MESSAGE_DRAFT_MENTION_TARGET_TYPE_URL
                    },
                    {"target", mention.target.get_target().c_str() ? mention.target.get_target().c_str() : ""}
                }}
            };
            mentions_json.push_back(mention_json);
        }
        auto data_json = nlohmann::json {
            {"messageElements", elements_json},
            {"suggestedMentions", mentions_json}
        };
        strcpy(data, data_json.dump().c_str());
    } catch (const std::exception& e) {
        pn_c_set_error_message(e.what());
        return PN_C_ERROR;
    }
    return PN_C_OK;
}
void pn_message_draft_set_search_for_suggestions(Pubnub::MessageDraft *message_draft, bool search_for_suggestions) {
    message_draft->set_search_for_suggestions(search_for_suggestions);
}

