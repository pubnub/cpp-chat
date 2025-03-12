#include "chat.hpp"
#include "channel.hpp"
#include "pubnub_chat/chat.hpp"
#include <gtest/gtest.h>
#include "string.hpp"
#include "pubnub_chat/message_draft.hpp"
#include "pubnub_chat/message.hpp"
#include "pubnub_chat/chat.hpp"
#include "pubnub_chat/vector.hpp"
#include "pubnub_chat/enums.hpp"
#include <algorithm>
#include <thread>
#include <vector>  
#include <string>  
#include <iostream>  

class MessageTests : public ::testing::Test {
protected:
    std::unique_ptr<Pubnub::Chat> chat;

    // This method will be called before each test in the test suite  
    void SetUp() override {
        Pubnub::String publish_key = std::getenv("PUBNUB_PUBLISH_KEY");
        if (publish_key.empty()) {
            publish_key = "demo-36";
        }
        Pubnub::String subscribe_key = std::getenv("PUBNUB_SUBSCRIBE_KEY");
        if (subscribe_key.empty()) {
            subscribe_key = "demo-36";
        }

        chat.reset(new Pubnub::Chat(Pubnub::Chat::init(
            publish_key, subscribe_key, "channel_tests_user", Pubnub::ChatConfig())));
    }

    // This method will be called after each test in the test suite  
    void TearDown() override {
        // Do your cleanup operations here  
    }
};

TEST_F(MessageTests, TestEditMessage) {

    auto channel = chat->create_public_conversation("message_edit_test_channel", Pubnub::ChatChannelData{});
    auto updated = false;
    channel.join([&](Pubnub::Message message) {
        message.stream_updates([&](Pubnub::Message updated_message) {
            if (updated_message.text() == Pubnub::String("edited text")) {
                updated = true;
                }
            });
        message.edit_text("edited text");
        });

    std::this_thread::sleep_for(std::chrono::seconds(3));

    channel.send_text("message");
    std::this_thread::sleep_for(std::chrono::seconds(12));

    ASSERT_TRUE(updated);
}

TEST_F(MessageTests, TestMessageReactions) {

    auto channel = chat->create_public_conversation("test_message_reactions_channel", Pubnub::ChatChannelData{});
    auto reacted = false;
    channel.join([&](Pubnub::Message message) {
        message = message.toggle_reaction("happy");
        reacted = message.has_user_reaction("happy");
        });

    std::this_thread::sleep_for(std::chrono::seconds(3));
    channel.send_text("message");
    std::this_thread::sleep_for(std::chrono::seconds(4));
    ASSERT_TRUE(reacted);
}