#ifndef PN_CHAT_TYPING_HPP
#define PN_CHAT_TYPING_HPP

// TODO: timer shouldn't be in the domain
#include "domain/json.hpp"
#include "infra/timer.hpp"
#include "string.hpp"
#include "event.hpp"
#include <map>
#include <optional>

// TODO: something is wrong with this class
class Typing {
    public:
        using UserId = Pubnub::String;

        Typing() = default;
        ~Typing() = default;

        void start();
        void stop();
        bool is_sent() const;

        void add_typing_indicator(const Pubnub::String& user_id);
        void remove_typing_indicator(const Pubnub::String& user_id);
        const std::vector<Pubnub::String>& get_typing_indicators() const;

        bool contains_typing_indicator(const Pubnub::String& user_id) const;

        static Pubnub::String payload(bool is_typing);
        static std::optional<bool> typing_value_from_event(const Pubnub::Event& event);
    private:
        bool sent = false;
        std::vector<Pubnub::String> typing_indicators;
};

#endif // PN_CHAT_TYPING_HPP
