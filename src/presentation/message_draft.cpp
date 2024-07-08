#include "presentation/message_draft.hpp"


using namespace Pubnub;

MessageDraft::MessageDraft(Channel channel, MessageDraftConfig draft_config, std::shared_ptr<MessageService> message_service) :
channel(channel),
draft_config(draft_config),
message_service(message_service)
{}