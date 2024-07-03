#include "presentation/message.hpp"

using namespace Pubnub;

Message::Message(std::shared_ptr<MessageService> message_service, Pubnub::String timetoken) :
message_service(message_service),
timetoken_internal(timetoken)
{}