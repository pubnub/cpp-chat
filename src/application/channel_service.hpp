#ifndef PN_CHAT_CHANNEL_SERVICE_HPP
#define PN_CHAT_CHANNEL_SERVICE_HPP

#include "application/dao/channel_dao.hpp"
#include "application/subscription.hpp"
#include "callback_stop.hpp"
#include "channel.hpp"
#include "membership.hpp"
#include "domain/channel_entity.hpp"
#include "enums.hpp"
#include "event.hpp"
#include "infra/rate_limiter.hpp"
#include "string.hpp"
#include "infra/sync.hpp"
#include "page.hpp"
#include <memory>
#include <vector>
#include <map>
#ifdef PN_CHAT_C_ABI
#include <pubnub_helper.h>
#endif

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

struct SendTextParamsInternal
{
    bool store_in_history = true;
    bool send_by_post = false;
    Pubnub::String meta = "";
    std::map<int, Pubnub::MentionedUser> mentioned_users;
    std::map<int, Pubnub::ReferencedChannel> referenced_channels;
    std::vector<Pubnub::TextLink> text_links;
    Pubnub::QuotedMessage quoted_message;

    SendTextParamsInternal() = default;

    SendTextParamsInternal(Pubnub::SendTextParams params) :
    store_in_history(params.store_in_history),
    send_by_post(params.send_by_post),
    meta(params.meta)
    {
        mentioned_users = params.mentioned_users.into_std_map();
        if(params.quoted_message.has_value())
        {
            Pubnub::Message message = params.quoted_message.value();
            quoted_message.channel_id = message.message_data().channel_id;
            quoted_message.text = message.text();
            quoted_message.user_id = message.message_data().user_id;
            quoted_message.timetoken = message.timetoken();
        }
        
    };
};

class ChannelService : public std::enable_shared_from_this<ChannelService>
{
    public:
        ChannelService(ThreadSafePtr<PubNub> pubnub, std::weak_ptr<ChatService> chat_service, float exponential_factor);

        Pubnub::Channel create_public_conversation(const Pubnub::String& channel_id, const ChannelDAO& channel_data) const;
        std::tuple<Pubnub::Channel, Pubnub::Membership, std::vector<Pubnub::Membership>> create_direct_conversation(const Pubnub::User& user, const Pubnub::String& channel_id, const ChannelDAO& channel_data, const Pubnub::ChatMembershipData& membership_data = Pubnub::ChatMembershipData()) const;
        std::tuple<Pubnub::Channel, Pubnub::Membership, std::vector<Pubnub::Membership>> create_group_conversation(const std::vector<Pubnub::User>& users, const Pubnub::String& channel_id, const ChannelDAO& channel_data, const Pubnub::ChatMembershipData& membership_data = Pubnub::ChatMembershipData()) const;
        Pubnub::Channel create_channel(const Pubnub::String& channel_id, const ChannelEntity&& channel_entity) const;
        Pubnub::Channel get_channel(const Pubnub::String& channel_id) const;
        std::tuple<std::vector<Pubnub::Channel>, Pubnub::Page, int> get_channels(const Pubnub::String& filter = "", const Pubnub::String& sort = "", int limit = 0, const Pubnub::Page& page = Pubnub::Page()) const;
        Pubnub::Channel update_channel(const Pubnub::String& channel_id, ChannelDAO channel_data) const;
        void delete_channel(const Pubnub::String& channel_id) const;
        std::vector<Pubnub::Message> get_channel_history(const Pubnub::String& channel_id, const Pubnub::String& start_timetoken, const Pubnub::String& end_timetoken, int count) const;
        Pubnub::Channel pin_message_to_channel(const Pubnub::Message& message, const Pubnub::String& channel_id, const ChannelDAO& channel) const;
        Pubnub::Channel unpin_message_from_channel(const Pubnub::String& channel_id, const ChannelDAO& channel) const;
        std::shared_ptr<Subscription> connect(const Pubnub::String& channel_id, const ChannelDAO& channel_data, std::function<void(Pubnub::Message)> message_callback) const;
        std::shared_ptr<Subscription> join(const Pubnub::Channel& channel, const ChannelDAO& channel_data, std::function<void(Pubnub::Message)> message_callback, const Pubnub::ChatMembershipData& membership_data = Pubnub::ChatMembershipData()) const;
        void disconnect(const ChannelDAO& channel_data) const;
        void leave(const Pubnub::String& channel_id, const ChannelDAO& channel_data) const;
        void send_text(const Pubnub::String& channel_id, const ChannelDAO& dao, const Pubnub::String& message, const SendTextParamsInternal& text_params = SendTextParamsInternal()) const;
        void start_typing(const Pubnub::String& channel_id, ChannelDAO& channel_data) const;
        void stop_typing(const Pubnub::String& channel_id, ChannelDAO& channel_data) const;
        std::shared_ptr<Subscription> get_typing(const Pubnub::String& channel_id, ChannelDAO& channel_data, std::function<void(const std::vector<Pubnub::String>&)> typing_callback) const;
        Pubnub::Message get_pinned_message(const Pubnub::String& channel_id, const ChannelDAO& channel_data) const;
        void emit_user_mention(const Pubnub::String &channel_id, const Pubnub::String& user_id, const Pubnub::String& timetoken, const Pubnub::String& text, const Pubnub::String &parent_channel_id = "") const;
        std::vector<Pubnub::Channel> get_channel_suggestions(Pubnub::String text, int limit = 10) const;
        std::vector<Pubnub::Membership> get_user_suggestions_for_channel(const Pubnub::String& channel_id, ChannelDAO& channel_data, Pubnub::String text, int limit = 10) const;


        std::shared_ptr<Subscription> stream_updates(Pubnub::Channel calling_channel, std::function<void(Pubnub::Channel)> channel_callback) const;
        std::shared_ptr<SubscriptionSet> stream_updates_on(Pubnub::Channel calling_channel, const std::vector<Pubnub::Channel>& channels, std::function<void(std::vector<Pubnub::Channel>)> channel_callback) const;
        std::shared_ptr<Subscription> stream_read_receipts(const Pubnub::String& channel_id, const ChannelDAO& channel_data, std::function<void(std::map<Pubnub::String, std::vector<Pubnub::String>, Pubnub::StringComparer>)> read_receipts_callback) const;
       
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

        Pubnub::Channel update_channel_with_base(const Pubnub::Channel& channel, const Pubnub::Channel& base_channel) const;

        std::tuple<std::vector<Pubnub::Event>, bool> get_message_reports_history(const Pubnub::String& channel_id, const Pubnub::String& start_timetoken, const Pubnub::String& end_timetoken, int count) const;
        std::shared_ptr<Subscription> stream_message_reports(const Pubnub::String& channel_id, std::function<void(const Pubnub::Event)> event_callback) const;


    private:
        ThreadSafePtr<PubNub> pubnub;
        std::weak_ptr<const ChatService> chat_service;
        mutable ExponentialRateLimiter rate_limiter;
        
        ChannelEntity create_domain_from_presentation_data(Pubnub::String channel_id, Pubnub::ChatChannelData& presentation_data);

        Pubnub::ChatChannelData presentation_data_from_domain(ChannelEntity& channel_entity);

        Pubnub::String send_text_meta_from_params(const SendTextParamsInternal& text_params) const;


        friend class ::MembershipService;
};

#endif // PN_CHAT_CHANNEL_SERVICE_HPP
