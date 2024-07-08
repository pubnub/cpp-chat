#include "presentation/message_draft.hpp"


using namespace Pubnub;

MessageDraft::MessageDraft(Channel channel, MessageDraftConfig draft_config) :
channel(channel),
draft_config(draft_config)
{}