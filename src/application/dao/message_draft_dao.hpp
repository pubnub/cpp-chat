#ifndef PN_CHAT_MESSAGE_DRAFT_DAO_HPP
#define PN_CHAT_MESSAGE_DRAFT_DAO_HPP

#include "domain/message_draft_entity.hpp"
#include "message_draft.hpp"
#include "vector.hpp"
#include <functional>

class MessageDraftDAO {
    public:
        MessageDraftDAO() = default;
        MessageDraftDAO(const MessageDraftEntity& entity);
        ~MessageDraftDAO() = default;

        MessageDraftEntity get_entity() const;
        void update_entity(const MessageDraftEntity& entity);

        // TODO: Callback stop?
        void add_callback(std::function<void(Pubnub::Vector<Pubnub::MessageElement>)> callback);
        void add_callback(std::function<void(Pubnub::Vector<Pubnub::MessageElement>, Pubnub::Vector<Pubnub::SuggestedMention>)> callback);

        bool should_search_for_suggestions() const;

        void call_callbacks(Pubnub::Vector<Pubnub::MessageElement> elements, Pubnub::Vector<Pubnub::SuggestedMention> mentions);
    private:
        MessageDraftEntity entity;

        std::vector<std::function<void(Pubnub::Vector<Pubnub::MessageElement>)>>
            on_message_change_callbacks;
        std::vector<std::function<void(Pubnub::Vector<Pubnub::MessageElement>, Pubnub::Vector<Pubnub::SuggestedMention>)>>
            on_message_change_callbacks_with_suggestions;
};

#endif // PN_CHAT_MESSAGE_DRAFT_DAO_HPP
