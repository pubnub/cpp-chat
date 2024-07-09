#ifndef PN_CHAT_CHANNEL_SERVICE_HPP
#define PN_CHAT_CHANNEL_SERVICE_HPP

#include "presentation/channel.hpp"
#include "enums.hpp"
#include "string.hpp"
#include "infra/sync.hpp"
#include <memory>
#include <vector>

class EntityRepository;
class PubNub;
struct ChannelEntity;

namespace Pubnub
{
    class Message;
    class Membership;
}

class ChannelService : public std::enable_shared_from_this<ChannelService>
{
    public:
        ChannelService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository, std::weak_ptr<ChatService> chat_service);
        Pubnub::ChatChannelData get_channel_data(Pubnub::String channel_id);

        Pubnub::Channel create_public_conversation(Pubnub::String channel_id, Pubnub::ChatChannelData data);
        std::tuple<Pubnub::Channel, Pubnub::Membership, std::vector<Pubnub::Membership>> create_direct_conversation(Pubnub::User user, Pubnub::String channel_id, Pubnub::ChatChannelData channel_data, Pubnub::String membership_data = "");
        std::tuple<Pubnub::Channel, Pubnub::Membership, std::vector<Pubnub::Membership>> create_group_conversation(std::vector<Pubnub::User> users, Pubnub::String channel_id, Pubnub::ChatChannelData channel_data, Pubnub::String membership_data = "");
        Pubnub::Channel create_channel(Pubnub::String channel_id, Pubnub::ChatChannelData data);
        Pubnub::Channel get_channel(Pubnub::String channel_id);
        std::vector<Pubnub::Channel> get_channels(Pubnub::String include, int limit, Pubnub::String start, Pubnub::String end);
        Pubnub::Channel update_channel(Pubnub::String channel_id, Pubnub::ChatChannelData channel_data);
        void delete_channel(Pubnub::String channel_id);
        void pin_message_to_channel(Pubnub::Message message, Pubnub::Channel channel);
        void unpin_message_from_channel(Pubnub::Channel channel);
        void connect(Pubnub::String channel_id, std::function<void(Pubnub::Message)> message_callback);
        void disconnect(Pubnub::String channel_id);
        void join(Pubnub::String channel_id, std::function<void(Pubnub::Message)> message_callback, Pubnub::String additional_params = "");
        void leave(Pubnub::String channel_id);
        void send_text(Pubnub::String channel_id, Pubnub::String message, Pubnub::pubnub_chat_message_type message_type, Pubnub::String meta_data);
        void start_typing(Pubnub::String channel_id);
        void stop_typing(Pubnub::String channel_id);
        void get_typing(Pubnub::String channel_id, std::function<void(std::vector<Pubnub::String>)> typing_callback);
        Pubnub::Message get_pinned_message(Pubnub::String channel_id);
        void stream_updates_on(std::vector<Pubnub::Channel> channels, std::function<void(Pubnub::Channel)> channel_callback);
        

        Pubnub::Channel create_channel_object(std::pair<Pubnub::String, ChannelEntity> channel_data);
        Pubnub::Channel create_presentation_object(Pubnub::String channel_id);

        //TODO: Move this to config
        int TYPING_TIMEOUT = 5000;

    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;
        std::weak_ptr<ChatService> chat_service;

        
        ChannelEntity create_domain_from_presentation_data(Pubnub::String channel_id, Pubnub::ChatChannelData& presentation_data);
        //Creates ChannelEntity from channel response - put the whole response, not only "data" field
        ChannelEntity create_domain_from_channel_response(Pubnub::String json_response);
        //Creates ChannelEntity from channel response "data" field
        ChannelEntity create_domain_from_channel_response_data(Pubnub::String json_response_data);

        Pubnub::ChatChannelData presentation_data_from_domain(ChannelEntity& channel_entity);

        friend class ::MembershipService;
};

#endif // PN_CHAT_CHANNEL_SERVICE_HPP
