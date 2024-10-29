#include "chat.hpp"
#include "channel.hpp"
#include "pubnub_chat/chat.hpp"
#include "cgreen/cgreen.h"
#include "cgreen/unit.h"

using namespace cgreen;

Describe(MessageDrafts);
std::unique_ptr<Pubnub::Chat> chat;
std::unique_ptr<Pubnub::Channel> channel;

BeforeEach(MessageDrafts) {
    Pubnub::String publish_key = std::getenv("PUBNUB_PUBLISH_KEY");
    Pubnub::String subscribe_key = std::getenv("PUBNUB_SUBSCRIBE_KEY");

    if (publish_key.empty() || subscribe_key.empty()) {
        throw std::runtime_error("PUBNUB_PUBLISH_KEY and PUBNUB_SUBSCRIBE_KEY must be set");
    }

    chat.reset(new Pubnub::Chat(Pubnub::Chat::init(
                    publish_key, subscribe_key, "test_id", Pubnub::ChatConfig())));

    channel.reset(new Pubnub::Channel(chat->create_public_conversation("test_channel", Pubnub::ChatChannelData{})));
}

AfterEach(MessageDrafts) { /* No cleanup needed */ }

Ensure(MessageDrafts, should_prepare_message_to_send) {
    sut = channel->create_message_draft();

    sut->insert_text(0, "typoHello, world!");
    sut->remove_text(0, 4);

    sut->send();
}

