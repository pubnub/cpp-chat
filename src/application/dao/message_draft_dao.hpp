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
#ifndef PN_CHAT_C_ABI
        void add_callback(std::function<void(Pubnub::Vector<Pubnub::MessageElement>)> callback);
        void add_callback(std::function<void(Pubnub::Vector<Pubnub::MessageElement>, Pubnub::Vector<Pubnub::SuggestedMention>)> callback);
#else
        void set_search_for_suggestions(bool search_for_suggestions);
        std::vector<Pubnub::MessageElement> consume_message_elements();
        std::vector<Pubnub::SuggestedMention> consume_suggested_mentions();
#endif
        void call_callbacks(Pubnub::Vector<Pubnub::MessageElement> elements, Pubnub::Vector<Pubnub::SuggestedMention> mentions = {});

        bool should_search_for_suggestions() const;

    private:
        MessageDraftEntity entity;

#ifndef PN_CHAT_C_ABI
        std::vector<std::function<void(Pubnub::Vector<Pubnub::MessageElement>)>>
            on_message_change_callbacks;
        std::vector<std::function<void(Pubnub::Vector<Pubnub::MessageElement>, Pubnub::Vector<Pubnub::SuggestedMention>)>>
            on_message_change_callbacks_with_suggestions;
#else
        std::vector<Pubnub::MessageElement> message_elements = {};
        std::vector<Pubnub::SuggestedMention> suggested_mentions = {};
        bool search_for_suggestions = false;
#endif
};

#endif // PN_CHAT_MESSAGE_DRAFT_DAO_HPP
