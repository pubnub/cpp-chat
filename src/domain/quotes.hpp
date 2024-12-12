#ifndef PN_CHAT_QUOTES_HPP
#define PN_CHAT_QUOTES_HPP

#include "string.hpp"

namespace Quotes {
    Pubnub::String add(const Pubnub::String& message, const Pubnub::String& quotes_mark = "\"");
    Pubnub::String add_unchecked(const Pubnub::String& message, const Pubnub::String& quotes_mark = "\"");

    Pubnub::String remove(const Pubnub::String& message, const Pubnub::String& quotes_mark = "\"");
    Pubnub::String remove_unchecked(const Pubnub::String& message, const Pubnub::String& quotes_mark = "\"");

    bool is_quoted(const Pubnub::String& message, const Pubnub::String& quotes_mark = "\"");
}

#endif // PN_CHAT_QUOTES_HPP
