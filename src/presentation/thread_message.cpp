#include "thread_message.hpp"


using namespace Pubnub;


ThreadMessage::ThreadMessage(String timetoken, std::shared_ptr<ChatService> chat_service, std::shared_ptr<MessageService> message_service, std::shared_ptr<ChannelService> channel_service,
                                    std::shared_ptr<RestrictionsService> restrictions_service, String parent_channel_id) :
Message(timetoken, chat_service, message_service, channel_service, restrictions_service),
parent_channel_id(parent_channel_id)
{}