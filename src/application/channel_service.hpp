#ifndef PN_CHAT_CHANNEL_SERVICE_HPP
#define PN_CHAT_CHANNEL_SERVICE_HPP

#include "channel.hpp"
#include "domain/channel_entity.hpp"
#include "enums.hpp"
#include "string.hpp"
#include "infra/sync.hpp"
#include <memory>
#include <vector>
#include <map>

class EntityRepository;
class PubNub;
struct ChannelEntity;

namespace Pubnub
{
    class Message;
    class Membership;
    class ThreadChannel;
    class ThreadMessage;
}

class ChannelService : public std::enable_shared_from_this<ChannelService>
{
    public:
        ChannelService(ThreadSafePtr<PubNub> pubnub, std::weak_ptr<ChatService> chat_service);

        Pubnub::Channel create_public_conversation(const Pubnub::String& channel_id, const ChannelDAO& channel_data) const;
        std::tuple<Pubnub::Channel, Pubnub::Membership, std::vector<Pubnub::Membership>> create_direct_conversation(const Pubnub::User& user, const Pubnub::String& channel_id, const ChannelDAO& channel_data, const Pubnub::String& membership_data = "") const;
        std::tuple<Pubnub::Channel, Pubnub::Membership, std::vector<Pubnub::Membership>> create_group_conversation(const std::vector<Pubnub::User>& users, const Pubnub::String& channel_id, const ChannelDAO& channel_data, const Pubnub::String& membership_data = "") const;
        Pubnub::Channel create_channel(const Pubnub::String& channel_id, const ChannelEntity&& channel_entity) const;
        Pubnub::Channel get_channel(const Pubnub::String& channel_id) const;
        std::vector<Pubnub::Channel> get_channels(const Pubnub::String& include, int limit, const Pubnub::String& start, const Pubnub::String& end) const;
        Pubnub::Channel update_channel(const Pubnub::String& channel_id, ChannelDAO channel_data) const;
        void delete_channel(const Pubnub::String& channel_id) const;
        std::vector<Pubnub::Message> get_channel_history(const Pubnub::String& channel_id, const Pubnub::String& start_timetoken, const Pubnub::String& end_timetoken, int count) const;
        Pubnub::Channel pin_message_to_channel(const Pubnub::Message& message, const Pubnub::String& channel_id, const ChannelDAO& channel) const;
        Pubnub::Channel unpin_message_from_channel(const Pubnub::String& channel_id, const ChannelDAO& channel) const;
#ifndef PN_CHAT_C_ABI
        void connect(const Pubnub::String& channel_id, std::function<void(Pubnub::Message)> message_callback) const;
        void join(const Pubnub::String& channel_id, std::function<void(Pubnub::Message)> message_callback, const Pubnub::String& additional_params = "") const;
#endif
        void disconnect(const Pubnub::String& channel_id) const;
        void leave(const Pubnub::String& channel_id) const;
        void send_text(const Pubnub::String& channel_id, const Pubnub::String& message, Pubnub::pubnub_chat_message_type message_type, const Pubnub::String& meta_data) const;
        void start_typing(const Pubnub::String& channel_id, ChannelDAO& channel_data) const;
        void stop_typing(const Pubnub::String& channel_id, ChannelDAO& channel_data) const;
        void get_typing(const Pubnub::String& channel_id, ChannelDAO& channel_data, std::function<void(const std::vector<Pubnub::String>&)> typing_callback) const;
        Pubnub::Message get_pinned_message(const Pubnub::String& channel_id, const ChannelDAO& channel_data) const;
        
        void stream_updates_on(const std::vector<Pubnub::String>& channel_ids, std::function<void(Pubnub::Channel)> channel_callback) const;
        void stream_read_receipts(const Pubnub::String& channel_id, const ChannelDAO& channel_data, std::function<void(std::map<Pubnub::String, std::vector<Pubnub::String>, Pubnub::StringComparer>)> read_receipts_callback) const;

        /* THREADS */
        Pubnub::String get_thread_id(const Pubnub::Message& message) const;
        Pubnub::ThreadChannel create_thread_channel(const Pubnub::Message& message) const;
        Pubnub::ThreadChannel get_thread_channel(const Pubnub::Message& message) const;
        void confirm_creating_thread(const Pubnub::ThreadChannel& thread_channel) const;
        bool has_thread_channel(const Pubnub::Message& message) const;
        void remove_thread_channel(const Pubnub::Message& message) const;
        Pubnub::ThreadChannel pin_message_to_thread_channel(const Pubnub::ThreadMessage& message, const Pubnub::ThreadChannel& thread_channel) const;
        Pubnub::ThreadChannel unpin_message_from_thread_channel(const Pubnub::ThreadChannel& thread_channel) const;
        std::vector<Pubnub::ThreadMessage> get_thread_channel_history(const Pubnub::String& channel_id, const Pubnub::String& start_timetoken, const Pubnub::String& end_timetoken, int count, const Pubnub::String& parent_channel_id) const;

        Pubnub::Channel create_channel_object(std::pair<Pubnub::String, ChannelEntity> channel_data) const;
        Pubnub::ThreadChannel create_thread_channel_object(std::pair<Pubnub::String, ChannelEntity> channel_data, Pubnub::Message parent_message) const;
        Pubnub::Channel create_presentation_object(Pubnub::String channel_id, ChannelDAO channel_data);

        //TODO: Move this to config
        int TYPING_TIMEOUT = 5000;
        int TYPING_TIMEOUT_DIFFERENCE = 1000;

    private:
        ThreadSafePtr<PubNub> pubnub;
        std::weak_ptr<const ChatService> chat_service;
        
        ChannelEntity create_domain_from_presentation_data(Pubnub::String channel_id, Pubnub::ChatChannelData& presentation_data);

        Pubnub::ChatChannelData presentation_data_from_domain(ChannelEntity& channel_entity);


        friend class ::MembershipService;

#ifdef PN_CHAT_C_ABI
    public:
        std::vector<Pubnub::String> connect(const Pubnub::String& channel_id) const;
        std::vector<Pubnub::String> join(const Pubnub::String& channel_id, const Pubnub::String& additional_params = "") const;
#endif
};

#endif // PN_CHAT_CHANNEL_SERVICE_HPP
