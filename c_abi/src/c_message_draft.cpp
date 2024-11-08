#include "c_message_draft.hpp"
#include "c_errors.hpp"
#include "domain/json.hpp"
#include "message_draft.hpp"
#include <exception>

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

void pn_message_draft_add_mention(Pubnub::MessageDraft* message_draft, size_t start, size_t length, const char* target) {
    try {
        auto parsed_target = Json::parse(target);
        auto target_type = parsed_target.get_int("type").value();
        auto target_value = parsed_target.get_string("target").value();

        auto mention_target = target_type == PN_MESSAGE_DRAFT_MENTION_TARGET_TYPE_USER 
            ? Pubnub::MentionTarget::user(target_value) 
            : target_type == PN_MESSAGE_DRAFT_MENTION_TARGET_TYPE_CHANNEL 
                ? Pubnub::MentionTarget::channel(target_value) 
                : Pubnub::MentionTarget::url(target_value);

        message_draft->add_mention(start, length, mention_target);
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


