#include "chat.hpp"
#include "channel.hpp"
#include "message_draft.hpp"
#include "pubnub_chat/chat.hpp"
#include <gtest/gtest.h>

class MessageDraftTests : public :: testing::Test {
    protected:
        std::unique_ptr<Pubnub::Chat> chat;
        std::unique_ptr<Pubnub::Channel> channel;

        // This method will be called before each test in the test suite  
        void SetUp() override {
            Pubnub::String publish_key = std::getenv("PUBNUB_PUBLISH_KEY");
            Pubnub::String subscribe_key = std::getenv("PUBNUB_SUBSCRIBE_KEY");

            if (publish_key.empty() || subscribe_key.empty()) {
                throw std::runtime_error("PUBNUB_PUBLISH_KEY and PUBNUB_SUBSCRIBE_KEY must be set");
            }

            chat.reset(new Pubnub::Chat(Pubnub::Chat::init(
                publish_key, subscribe_key, "test_id", Pubnub::ChatConfig())));

            channel.reset(new Pubnub::Channel(chat->create_public_conversation("test_channel", Pubnub::ChatChannelData{})));
        }

        // This method will be called after each test in the test suite  
        void TearDown() override {
            // Do your cleanup operations here  
        }

    TEST_F(MessageDraft_GoogleTests, TestCreateMessageDraft) {
        ASSERT_NO_THROW(channel->create_message_draft());
    }
};

