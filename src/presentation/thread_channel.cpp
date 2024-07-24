#include "thread_channel.hpp"
#include "application/dao/channel_dao.hpp"


using namespace Pubnub;

ThreadChannel::ThreadChannel(
                    Pubnub::String channel_id, 
                    std::shared_ptr<const ChatService> chat_service,
                    std::shared_ptr<const ChannelService> channel_service,
                    std::shared_ptr<const PresenceService> presence_service,
                    std::shared_ptr<const RestrictionsService> restrictions_service,
                    std::shared_ptr<const MessageService> message_service,
                    std::shared_ptr<const MembershipService> membership_service,
                    std::unique_ptr<ChannelDAO> data,
                    Pubnub::String parent_channel_id, 
                    Pubnub::Message parent_message) :
Channel(channel_id, chat_service, channel_service, presence_service, restrictions_service, message_service, membership_service, std::move(data)),
parent_channel_id(parent_channel_id),
parent_message(parent_message)
{}
