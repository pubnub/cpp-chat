#include "c_message_draft.hpp"
#include "c_errors.hpp"
#include "domain/json.hpp"
#include "message_draft.hpp"
#include <exception>
#include <vector>

void pn_message_draft_delete(Pubnub::MessageDraft* message_draft) {
    delete message_draft;
}

void pn_message_draft_insert_text(Pubnub::MessageDraft* message_draft, size_t position, const char* text) {
    try {
        message_draft->insert_text(position, text);
    } catch (const std::exception& e) {
        pn_c_set_error_message(e.what());
    }
}

void pn_message_draft_remove_text(Pubnub::MessageDraft* message_draft, size_t position, size_t length) {
    try {
        message_draft->remove_text(position, length);
    } catch (const std::exception& e) {
        pn_c_set_error_message(e.what());
    }
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

void pn_message_draft_insert_suggested_mention(Pubnub::MessageDraft *message_draft, std::size_t offset, const char *replace_from, const char *replace_to, const char *target_json, const char* text) {
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
    }

}

void pn_message_draft_add_mention(Pubnub::MessageDraft* message_draft, size_t start, size_t length, const char* target) {
    try {
        message_draft->add_mention(start, length, deserialize_mention_target(target));
    } catch (const std::exception& e) {
        pn_c_set_error_message(e.what());
    }
}

void pn_message_draft_remove_mention(Pubnub::MessageDraft* message_draft, size_t start) {
    try {
        message_draft->remove_mention(start);
    } catch (const std::exception& e) {
        pn_c_set_error_message(e.what());
    }
}

void pn_message_draft_update(Pubnub::MessageDraft* message_draft, const char* text) {
    try {
        message_draft->update(text);
    } catch (const std::exception& e) {
        pn_c_set_error_message(e.what());
    }
}

void pn_message_draft_send(Pubnub::MessageDraft* message_draft, const Pubnub::SendTextParams* send_params) {
    try {
        message_draft->send(*send_params);
    } catch (const std::exception& e) {
        pn_c_set_error_message(e.what());
    }
}

void pn_message_draft_consume_callback_data(Pubnub::MessageDraft *message_draft, char *data) {
    try {
        auto message_elements = message_draft->consume_message_elements();

        std::vector<nlohmann::json> elements_json;
        for (const auto& element : message_elements) {
            auto element_json = nlohmann::json {
                {"text", element.text},
                {"target", element.target.has_value() 
                    ? nlohmann::json {
                        {"type", element.target.value().get_type() == Pubnub::MentionTarget::Type::USER 
                            ? PN_MESSAGE_DRAFT_MENTION_TARGET_TYPE_USER 
                            : element.target.value().get_type() == Pubnub::MentionTarget::Type::CHANNEL 
                                ? PN_MESSAGE_DRAFT_MENTION_TARGET_TYPE_CHANNEL 
                                : PN_MESSAGE_DRAFT_MENTION_TARGET_TYPE_URL
                        },
                        {"target", element.target.value().get_target()}
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
                {"replace_from", mention.replace_from},
                {"replace_to", mention.replace_to},
                {"target", nlohmann::json {
                    {"type", mention.target.get_type() == Pubnub::MentionTarget::Type::USER 
                        ? PN_MESSAGE_DRAFT_MENTION_TARGET_TYPE_USER 
                        : mention.target.get_type() == Pubnub::MentionTarget::Type::CHANNEL 
                            ? PN_MESSAGE_DRAFT_MENTION_TARGET_TYPE_CHANNEL 
                            : PN_MESSAGE_DRAFT_MENTION_TARGET_TYPE_URL
                    },
                    {"target", mention.target.get_target()}
                }}
            };
            mentions_json.push_back(mention_json);
        }

        auto data_json = nlohmann::json {
            {"message_elements", elements_json},
            {"suggested_mentions", mentions_json}
        };

        strcpy(data, data_json.dump().c_str());
    } catch (const std::exception& e) {
        pn_c_set_error_message(e.what());
    }

}


