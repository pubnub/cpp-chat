#include "chat/message_draft.hpp"
#include <iostream>
#include "chat.hpp"

using namespace Pubnub;

MessageDraft::MessageDraft(Pubnub::Chat& in_chat, Pubnub::Channel in_channel, Pubnub::MessageDraftConfig in_draft_config) :
    chat_obj(in_chat),
    channel(in_channel),
    draft_config(in_draft_config)
{

}