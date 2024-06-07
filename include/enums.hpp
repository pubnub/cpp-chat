#ifndef PN_ENUMS_HPP
#define PN_ENUMS_HPP

#include "string.hpp"

namespace Pubnub
{
enum pubnub_message_action_type : uint8_t
{
    PMAT_Reaction,
    PMAT_Receipt,
    PMAT_Custom,
	PMAT_Edited,
	PMAT_Deleted
};

enum pubnub_chat_event_type : uint8_t
{
    PCET_TYPING,
    PCET_REPORT,
    PCET_RECEPIT,
    PCET_MENTION,
    PCET_INVITE,
    PCET_CUSTOM,
    PCET_MODERATION
};

enum pubnub_chat_message_type : uint8_t
{
    PCMT_TEXT
};

static inline Pubnub::String chat_event_type_to_string(pubnub_chat_event_type chat_event_type)
{
    switch(chat_event_type)
	{
	case pubnub_chat_event_type::PCET_TYPING:
		return "typing";
	case pubnub_chat_event_type::PCET_REPORT:
		return "report";
	case pubnub_chat_event_type::PCET_RECEPIT:
		return "receipt";
	case pubnub_chat_event_type::PCET_MENTION:
		return "mention";
	case pubnub_chat_event_type::PCET_INVITE:
		return "invite";
	case pubnub_chat_event_type::PCET_CUSTOM:
		return "custom";
	case pubnub_chat_event_type::PCET_MODERATION:
		return "moderation";
	}
	return "incorrect_chat_event_type";
};

static inline Pubnub::String chat_message_type_to_string(pubnub_chat_message_type chat_message_type)
{
    switch(chat_message_type)
	{
    case pubnub_chat_message_type::PCMT_TEXT:
        return "text";
    }
    return "incorrect_chat_message_type";
}
}

#endif // PN_ENUMS_HPP
