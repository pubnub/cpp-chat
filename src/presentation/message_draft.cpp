#include "message_draft.hpp"


using namespace Pubnub;

MessageDraft::MessageDraft(Channel channel, MessageDraftConfig draft_config, std::shared_ptr<MessageService> message_service) :
channel(channel),
draft_config(draft_config),
message_service(message_service)
{}

Message MessageDraft::quoted_message()
{
    if(quoted_message_internal.has_value())
    {
        return quoted_message_internal.value();
    }

    //TODO:: this shouldn't be any error, just draft doesn't have a quoted message. We need to decide what to do here
    throw std::invalid_argument("No quoted message");
}

void MessageDraft::add_quote(Message message)
{
    quoted_message_internal = Option<Message>::some(message);
}
void MessageDraft::remove_quote()
{
    quoted_message_internal = Option<Message>::none();
}
