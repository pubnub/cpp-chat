#include "message_draft_dao.hpp"
#include "domain/message_draft_entity.hpp"

MessageDraftDAO::MessageDraftDAO(const MessageDraftEntity& entity) : entity(entity) {}

MessageDraftEntity MessageDraftDAO::get_entity() const {
    return this->entity;
}

void MessageDraftDAO::update_entity(const MessageDraftEntity& entity) {
    this->entity = entity;
}

#ifndef PN_CHAT_C_ABI
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

void MessageDraftDAO::call_callbacks(
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

#else

void MessageDraftDAO::set_search_for_suggestions(bool search_for_suggestions) {
    this->search_for_suggestions = search_for_suggestions;
}

void MessageDraftDAO::call_callbacks(
        Pubnub::Vector<Pubnub::MessageElement> elements,
        Pubnub::Vector<Pubnub::SuggestedMention> mentions
) {
    this->message_elements.insert(
            this->message_elements.end(),
            elements.begin(),
            elements.end()
    );

    this->suggested_mentions.insert(
            this->suggested_mentions.end(),
            mentions.begin(),
            mentions.end()
    );
}

std::vector<Pubnub::MessageElement> MessageDraftDAO::consume_message_elements() {
    auto result = this->message_elements;
    this->message_elements.clear();

    return result;
}

std::vector<Pubnub::SuggestedMention> MessageDraftDAO::consume_suggested_mentions() {
    auto result = this->suggested_mentions;
    this->suggested_mentions.clear();

    return result;
}
#endif

bool MessageDraftDAO::should_search_for_suggestions() const {
#ifndef PN_CHAT_C_ABI
    return !this->on_message_change_callbacks_with_suggestions.empty();
#else
    return this->search_for_suggestions;
#endif
}


