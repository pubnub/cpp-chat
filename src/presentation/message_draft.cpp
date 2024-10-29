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
    this->value.reset(new MessageDraftDAO(
                this->draft_service->insert_text_to_message(*this->value, position, text)));
    channel.start_typing();
    this->fire_message_elements_changed();
}

void MessageDraft::remove_text(std::size_t position, std::size_t length) {
    this->value.reset(new MessageDraftDAO(
                this->draft_service->remove_text_from_message(*this->value, position, length)));
    this->channel.start_typing();
    this->fire_message_elements_changed();
}

void MessageDraft::send(SendTextParams send_params) {
    // TODO: now only sending text
    this->channel.send_text(this->value->get_entity().value, send_params);
}

void MessageDraft::fire_message_elements_changed() {
    // TODO: implement
}
