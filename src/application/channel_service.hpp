#ifndef PN_CHAT_CHANNEL_SERVICE_HPP
#define PN_CHAT_CHANNEL_SERVICE_HPP

#include "presentation/channel.hpp"
#include "export.hpp"
#include "infra/sync.hpp"
#include <memory>
#include <vector>

class EntityRepository;
class PubNub;
class ChannelEntity;

class ChannelService : public std::enable_shared_from_this<ChannelService>
{
    public:
        PN_CHAT_EXPORT ChannelService(ThreadSafePtr<PubNub> pubnub, std::shared_ptr<EntityRepository> entity_repository);

        Pubnub::Channel create_public_conversation(Pubnub::String channel_id, Pubnub::ChatChannelData data);
        Pubnub::Channel create_channel(Pubnub::String channel_id, Pubnub::ChatChannelData data);
        Pubnub::Channel get_channel(Pubnub::String channel_id);
        std::vector<Pubnub::Channel> get_channels(Pubnub::String include, int limit, Pubnub::String start, Pubnub::String end);
        Pubnub::Channel update_channel(Pubnub::String channel_id, Pubnub::ChatChannelData channel_data);
        void delete_channel(Pubnub::String channel_id);

    private:
        ThreadSafePtr<PubNub> pubnub;
        std::shared_ptr<EntityRepository> entity_repository;


        ChannelEntity create_domain_from_presentation_data(Pubnub::String channel_id, Pubnub::ChatChannelData& presentation_data);
        //Creates ChannelEntity from channel response - put the whole response, not only "data" field
        ChannelEntity create_domain_from_channel_response(Pubnub::String json_response);
        //Creates ChannelEntity from channel response "data" field
        ChannelEntity create_domain_from_channel_response_data(Pubnub::String json_response_data);
};

#endif // PN_CHAT_CHANNEL_SERVICE_HPP
