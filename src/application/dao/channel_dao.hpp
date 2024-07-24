#ifndef PN_CHAT_CHANNEL_DAO_HPP
#define PN_CHAT_CHANNEL_DAO_HPP

#include "channel.hpp"
#include "domain/channel_entity.hpp"
#include "domain/typing.hpp"
#include "infra/sync.hpp"
#include "infra/timer.hpp"
#include <map>

class ChannelDAO {
    public:
        ChannelDAO(const ChannelEntity channel_entity);
        ChannelDAO(const Pubnub::ChatChannelData& channel_data);
        ~ChannelDAO() = default;

        Pubnub::ChatChannelData to_channel_data() const;

        ChannelEntity to_entity() const;
        const ChannelEntity& get_entity() const;

        // TODO: chrono...
        void start_typing(int miliseconds) const;
        void stop_typing() const;
        void start_typing_indicator(const Pubnub::String& user_id, int miliseconds, std::function<void(const std::vector<Pubnub::String>&)> on_timeout) const;
        void stop_typing_indicator(const Pubnub::String& user_id) const;
        std::vector<Pubnub::String> get_typing_indicators() const;
        void remove_typing_indicator(const Pubnub::String& user_id) const;
        bool is_typing_sent() const;
        bool contains_typing_indicator(const Pubnub::String& user_id) const;
    private:
        static ChannelEntity entity_from_channel_data(const Pubnub::ChatChannelData& channel_data);

        const ChannelEntity channel_entity;
        mutable Mutex<Typing> typing;
        mutable Mutex<Timer> typing_timer;
        mutable Mutex<std::map<Pubnub::String, Timer, Pubnub::StringComparer>> indicators_timers;
};

#endif // PN_CHAT_CHANNEL_DAO_HPP

