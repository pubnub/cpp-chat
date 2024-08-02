#ifndef PN_CHAT_MESSAGE_ENTITY_HPP
#define PN_CHAT_MESSAGE_ENTITY_HPP

#include "domain/json.hpp"
#include "enums.hpp"
#include "string.hpp"
#include "message_action.hpp"
#include "message_elements.hpp"
#include "option.hpp"
#include <optional>
#include <variant>
#include <vector>

struct MessageEntity {
    using MessageTimetoken = Pubnub::String;
    using ActionTimetoken = Pubnub::String;

    Pubnub::pubnub_chat_message_type type;
    Pubnub::String text;
    Pubnub::String channel_id;
    Pubnub::String user_id;
    Pubnub::String meta;
    // TODO: leak of presentation...
    std::vector<Pubnub::MessageAction> message_actions;

    static MessageEntity from_json(Pubnub::String message_json, Pubnub::String channel_id);
    static std::vector<std::pair<MessageTimetoken, MessageEntity>> from_history_json(Json history_json, Pubnub::String channel_id);

    MessageEntity edit_text(const Pubnub::String& new_text, const Pubnub::String& timetoken) const;
    MessageEntity delete_message(const Pubnub::String& new_text, const Pubnub::String& timetoken) const;

    Pubnub::String current_text() const;
    bool is_deleted() const;
    std::vector<Pubnub::MessageAction> get_reactions() const;

    std::optional<ActionTimetoken> get_user_reaction_timetoken(const Pubnub::String& user_id, const Pubnub::String& reaction) const;
    MessageEntity remove_user_reaction(const ActionTimetoken& timetoken) const;
    MessageEntity add_user_reaction(const Pubnub::String& user_id, const Pubnub::String& reaction, const ActionTimetoken& timetoken) const;
    std::map<int, Pubnub::MentionedUser> get_mentioned_users() const;
    std::map<int, Pubnub::ReferencedChannel> get_referenced_channels() const;
    Pubnub::QuotedMessage get_quoted_message() const;
    std::vector<Pubnub::TextLink> get_text_links() const;
};

#endif // PN_CHAT_MESSAGE_ENTITY_HPP
