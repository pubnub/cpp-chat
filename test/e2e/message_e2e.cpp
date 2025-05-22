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
#include "e2e_tests_helpers.h"
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
            publish_key = PubnubTests::TESTS_DEFAULT_PUB_KEY;
        }
        Pubnub::String subscribe_key = std::getenv("PUBNUB_SUBSCRIBE_KEY");
        if (subscribe_key.empty()) {
            subscribe_key = PubnubTests::TESTS_DEFAULT_SUB_KEY;
        } 

        chat.reset(new Pubnub::Chat(Pubnub::Chat::init(
            publish_key, subscribe_key, "message_tests_user", Pubnub::ChatConfig())));
    }

    // This method will be called after each test in the test suite  
    void TearDown() override {
        // Do your cleanup operations here  
    }
};

TEST_F(MessageTests, TestSendAndReceive) {
    auto channel =
        chat->create_public_conversation("message_send_test_channel", Pubnub::ChatChannelData {});
    
    auto received = false;
    channel.join([&](Pubnub::Message message) { 
        if (message.text() == Pubnub::String("message")) {
            received = true;
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(3));

    channel.send_text("message");
    
    std::this_thread::sleep_for(std::chrono::seconds(6));

    ASSERT_TRUE(received);
}

TEST_F(MessageTests, TestReceivingMessageData) {
    auto channel =
        chat->create_public_conversation("message_data_test_channel", Pubnub::ChatChannelData {});

    auto received = false;
    channel.join([&](Pubnub::Message message) {
        if (message.text() == Pubnub::String("message")
            && message.message_data().meta == Pubnub::String("{\"some_meta\":\"some_value\"}")) {
            received = true;
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(3));

    auto send_params = Pubnub::SendTextParams();
    send_params.meta = "{\"some_meta\":\"some_value\"}";
    channel.send_text("message", send_params);

    std::this_thread::sleep_for(std::chrono::seconds(6));

    ASSERT_TRUE(received);
}

TEST_F(MessageTests, TestGetMessage) {
    auto channel =
        chat->create_public_conversation("message_get_test_channel", Pubnub::ChatChannelData {});

    auto time_token = "";
    channel.join([&](Pubnub::Message message) {
        if (message.text() == Pubnub::String("message")) {
            time_token = message.timetoken();
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(3));

    channel.send_text("message");

     std::this_thread::sleep_for(std::chrono::seconds(8));

    auto fetched_message = channel.get_message(time_token);
    ASSERT_TRUE(fetched_message.timetoken() == Pubnub::String(time_token));
}

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

TEST_F(MessageTests, TestDeleteMessage) {
    auto channel =
        chat->create_public_conversation("message_delete_test_channel", Pubnub::ChatChannelData {});
    auto deleted = false;
    channel.join([&](Pubnub::Message message) { 
        message = message.delete_message();
        deleted = message.deleted();
    });

    std::this_thread::sleep_for(std::chrono::seconds(3));

    channel.send_text("message");
    std::this_thread::sleep_for(std::chrono::seconds(12));

    ASSERT_TRUE(deleted);
}

TEST_F(MessageTests, TestRestoreMessage) {
    auto channel =
        chat->create_public_conversation("message_restore_test_channel", Pubnub::ChatChannelData {});
    auto deleted = false;
    auto restored = false;
    channel.join([&](Pubnub::Message message) {
        message = message.delete_message();
        deleted = message.deleted();
        message = message.restore();
        restored = !message.deleted();
    });

    std::this_thread::sleep_for(std::chrono::seconds(3));

    channel.send_text("message");
    std::this_thread::sleep_for(std::chrono::seconds(12));

    ASSERT_TRUE(deleted);
    ASSERT_TRUE(restored);
}

TEST_F(MessageTests, TestPinAndUnPinMessage) {
    auto channel = chat->create_public_conversation(
        "message_pin_test_channel",
        Pubnub::ChatChannelData {}
    );
    auto pinned = false;
    auto unpinned = false;
    channel.join([&](Pubnub::Message message) { 
        message.pin();
        std::this_thread::sleep_for(std::chrono::seconds(5));
        try {
            auto pinned_message = channel.get_pinned_message();
            pinned = pinned_message.text() == message.text();
        } catch (const std::exception&) {
            pinned = false;
        }
        message.unpin();
        std::this_thread::sleep_for(std::chrono::seconds(5));
        try {
            auto pinned_message = channel.get_pinned_message();
        } catch (const std::exception&) {
            unpinned = true;
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(3));

    channel.send_text("message");
    std::this_thread::sleep_for(std::chrono::seconds(25));

    ASSERT_TRUE(pinned);
    ASSERT_TRUE(unpinned);
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

TEST_F(MessageTests, TestMessageReport) {
    auto channel = chat->create_public_conversation(
        "test_message_report_channel",
        Pubnub::ChatChannelData {}
    );

    auto reported = false;
    channel.stream_message_reports([&](Pubnub::Event report_event) {
        reported = report_event.user_id == Pubnub::String("message_tests_user");
    });

    channel.join([&](Pubnub::Message message) { 
        message.report("report_reason");
    });

    std::this_thread::sleep_for(std::chrono::seconds(5));
    channel.send_text("message");
    std::this_thread::sleep_for(std::chrono::seconds(10));
    ASSERT_TRUE(reported);
}

TEST_F(MessageTests, TestCreateThread) {
    auto channel =
        chat->create_public_conversation("message_create_thread_test_channel", Pubnub::ChatChannelData {});
    auto created_thread = false;
    channel.join([&](Pubnub::Message message) {

        message.stream_updates([&](Pubnub::Message updated_message) {
            created_thread = updated_message.has_thread();
            });

        auto thread = message.create_thread();

        thread.join([&](Pubnub::Message message) {});
        thread.send_text("init");

        std::this_thread::sleep_for(std::chrono::seconds(7));
    });
    std::this_thread::sleep_for(std::chrono::seconds(2));
    channel.send_text("thread_start_message");
    std::this_thread::sleep_for(std::chrono::seconds(20));
    ASSERT_TRUE(created_thread);
}