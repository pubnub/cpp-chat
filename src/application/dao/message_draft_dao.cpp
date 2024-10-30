#include "message_draft_dao.hpp"
#include "domain/message_draft_entity.hpp"

MessageDraftDAO::MessageDraftDAO(const MessageDraftEntity& entity) : entity(entity) {}

MessageDraftEntity MessageDraftDAO::get_entity() const {
    return this->entity;
}

void MessageDraftDAO::update_entity(const MessageDraftEntity& entity) {
    this->entity = entity;
}

void MessageDraftDAO::add_callback(std::function<void(Pubnub::Vector<Pubnub::MessageElement>)> callback) {
    this->on_message_change_callbacks.push_back(callback);
}

void MessageDraftDAO::add_callback(
        std::function<void(
            Pubnub::Vector<Pubnub::MessageElement>,
            Pubnub::Vector<Pubnub::SuggestedMention>
        )> callback
) {
    this->on_message_change_callbacks_with_suggestions.push_back(callback);
}

bool MessageDraftDAO::should_search_for_suggestions() const {
    return !this->on_message_change_callbacks_with_suggestions.empty();
}

void MessageDraftDAO::fire_message_elements_changed(
        Pubnub::Vector<Pubnub::MessageElement> elements,
        Pubnub::Vector<Pubnub::SuggestedMention> mentions
) {
    for (auto& callback : this->on_message_change_callbacks) {
        callback(elements);
    }

    if (mentions.size() > 0) {
        for (auto& callback : this->on_message_change_callbacks_with_suggestions) {
            callback(elements, mentions);
        }
    }
}

