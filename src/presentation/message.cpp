#include "presentation/message.hpp"

using namespace Pubnub;

Message::Message(Pubnub::String timetoken, std::shared_ptr<ChatService> chat_service, std::shared_ptr<MessageService> message_service) :
timetoken_internal(timetoken),
chat_service(chat_service),
message_service(message_service)
{}