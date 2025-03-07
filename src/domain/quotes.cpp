#include "quotes.hpp"

Pubnub::String Quotes::add(const Pubnub::String& message, const Pubnub::String& quotes_mark) {
    if (message.empty()) {
        return quotes_mark + quotes_mark;
    }

    if (is_quoted(message, quotes_mark)) {
        return message;
    }

    return add_unchecked(message, quotes_mark);
}

Pubnub::String Quotes::add_unchecked(const Pubnub::String& message, const Pubnub::String& quotes_mark) {
    return quotes_mark + message + quotes_mark;
}

Pubnub::String Quotes::remove(const Pubnub::String& message, const Pubnub::String& quotes_mark) {
    if (message.empty()) {
        return message;
    }

    if (!is_quoted(message, quotes_mark)) {
        return message;
    }

    return remove_unchecked(message, quotes_mark);
}

Pubnub::String Quotes::remove_unchecked(const Pubnub::String& message, const Pubnub::String& quotes_mark) {
    return message.substring(1, message.length() - 2);
}

bool Quotes::is_quoted(const Pubnub::String& message, const Pubnub::String& quotes_mark) {
    if (message.length() < 2) {
        return false;
    }

    return message[0] == quotes_mark[0] && message[message.length() - 1] == quotes_mark[0];
}
