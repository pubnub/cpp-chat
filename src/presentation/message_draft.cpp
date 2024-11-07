#include "application/channel_service.hpp"
#include "application/dao/message_draft_dao.hpp"
#include "application/draft_service.hpp"
#include "option.hpp"
#include "message_draft.hpp"

using namespace Pubnub;

MentionTarget::MentionTarget(const Pubnub::String& target, const Pubnub::MentionTarget::Type target_type) :
    target(target),
    type(target_type) {}

MentionTarget MentionTarget::user(const Pubnub::String& user_id) {
    return MentionTarget(user_id, MentionTarget::Type::USER);
}

MentionTarget MentionTarget::channel(const Pubnub::String& channel_id) {
    return MentionTarget(channel_id, MentionTarget::Type::CHANNEL);
}

MentionTarget MentionTarget::url(const Pubnub::String& url) {
    return MentionTarget(url, MentionTarget::Type::URL);
}

Pubnub::MentionTarget::Type MentionTarget::get_type() const {
    return this->type;
}

Pubnub::String MentionTarget::get_target() const {
    return this->target;
}

MessageElement::MessageElement(const Pubnub::String& text, const Pubnub::Option<MentionTarget>& target) :
    text(text),
    target(target) {}

MessageElement MessageElement::plain_text(const Pubnub::String& text) {
    return MessageElement(text, Pubnub::Option<MentionTarget>::none());
}

MessageElement MessageElement::link(const Pubnub::String& text, const MentionTarget& target) {
    return MessageElement(text, target);
}

MessageDraft::MessageDraft(
        const Channel& channel,
        const MessageDraftConfig& draft_config,
        std::shared_ptr<const ChannelService> channel_service,
        std::shared_ptr<const UserService> user_service) :
channel(channel),
draft_config(draft_config),
value(std::make_unique<MessageDraftDAO>()),
draft_service(std::make_unique<DraftService>(user_service, channel_service)) {}

MessageDraft::~MessageDraft() = default;

void MessageDraft::insert_text(std::size_t position, const Pubnub::String& text) {
    this->trigger_typing_indicator();
    this->draft_service->insert_text_to_message(*this->value, position, text);
}

void MessageDraft::remove_text(std::size_t position, std::size_t length) {
    this->trigger_typing_indicator();
    this->draft_service->remove_text_from_message(*this->value, position, length);
}

void MessageDraft::insert_suggested_mention(const SuggestedMention& suggested_mention, const Pubnub::String& text) {
    this->trigger_typing_indicator();
    this->draft_service->insert_suggested_mention_to_message(*this->value, suggested_mention, text);
}

void MessageDraft::add_mention(std::size_t position, std::size_t length, const MentionTarget& target) {
    this->draft_service->add_mention_to_message(*this->value, position, length, target);
}

void MessageDraft::remove_mention(std::size_t position) {
    this->draft_service->remove_mention_from_message(*this->value, position);
}

void MessageDraft::update(const Pubnub::String& text) {
    this->trigger_typing_indicator();
    this->draft_service->update_message(*this->value, text);
}

void MessageDraft::send(SendTextParams send_params) {
    auto sending_data = this->draft_service->prepare_sending_data(*this->value, send_params);
    this->channel.send_text(sending_data.second, sending_data.first);
}

void MessageDraft::trigger_typing_indicator() {
    if (this->draft_config.is_typing_indicator_triggered && this->channel.channel_data().type != Pubnub::String("public")) {
        this->channel.start_typing();
    }
}

void MessageDraft::add_message_elements_listener(std::function<void(Pubnub::Vector<Pubnub::MessageElement>)> listener) {
    this->value->add_callback(listener);
}

void MessageDraft::add_message_elements_listener(
        std::function<void(
            Pubnub::Vector<Pubnub::MessageElement>,
            Pubnub::Vector<Pubnub::SuggestedMention>
        )> listener
) {
    this->value->add_callback(listener);
}



