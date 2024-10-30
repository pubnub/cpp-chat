#include "application/dao/message_draft_dao.hpp"
#include "application/draft_service.hpp"
#include "message_draft.hpp"

using namespace Pubnub;

MessageDraft::MessageDraft(const Channel& channel, const MessageDraftConfig& draft_config) :
channel(channel),
draft_config(draft_config),
value(std::make_unique<MessageDraftDAO>()),
draft_service(std::make_unique<DraftService>()) {}

void MessageDraft::insert_text(std::size_t position, const Pubnub::String& text) {
    this->channel.start_typing();
    this->draft_service->insert_text_to_message(*this->value, position, text);
}

void MessageDraft::remove_text(std::size_t position, std::size_t length) {
    this->channel.start_typing();
    this->draft_service->remove_text_from_message(*this->value, position, length);
}

void MessageDraft::insert_suggested_mention(const SuggestedMention& suggested_mention, const Pubnub::String& text) {
    this->channel.start_typing();
    this->draft_service->insert_suggested_mention_to_message(*this->value, suggested_mention, text);
}

void MessageDraft::add_mention(std::size_t position, std::size_t length, const MentionTarget& target) {
    this->draft_service->add_mention_to_message(*this->value, position, length, target);
}

void MessageDraft::remove_mention(std::size_t position) {
    this->draft_service->remove_mention_from_message(*this->value, position);
}

void MessageDraft::update(const Pubnub::String& text) {
    this->draft_service->update_message(*this->value, text);
}

void MessageDraft::send(SendTextParams send_params) {
    // TODO: now only sending text
    this->channel.send_text(this->value->get_entity().value, send_params);
}

