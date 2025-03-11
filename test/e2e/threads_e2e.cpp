#include "chat.hpp"
#include "channel.hpp"
#include "pubnub_chat/chat.hpp"
#include <gtest/gtest.h>
#include "string.hpp"
#include "pubnub_chat/message_draft.hpp"
#include "pubnub_chat/message.hpp"
#include "pubnub_chat/thread_message.hpp"
#include "pubnub_chat/chat.hpp"
#include "pubnub_chat/vector.hpp"
#include "pubnub_chat/enums.hpp"
#include <algorithm>
#include <thread>
#include <vector>  
#include <string>  
#include <iostream>  

class ThreadsTests : public ::testing::Test {
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
            publish_key, subscribe_key, "threads_tests_user", Pubnub::ChatConfig())));
    }

    // This method will be called after each test in the test suite  
    void TearDown() override {
        // Do your cleanup operations here  
    }
};

TEST_F(ThreadsTests, TestGetThreadHistory) {
    auto channel = chat->create_public_conversation("thread_history_test_channel", Pubnub::ChatChannelData{});
    
    auto correct_amount = false;
    auto correct_text = false;
    channel.join([&](Pubnub::Message message) {
        auto thread = message.create_thread();
        thread.join([&](Pubnub::Message message){});

        std::this_thread::sleep_for(std::chrono::seconds(2));

        thread.send_text("one");
        thread.send_text("two");
        thread.send_text("three");

        std::this_thread::sleep_for(std::chrono::seconds(8));

        auto history = thread.get_history("99999999999999999", "00000000000000000", 3);

        correct_amount = history.size() == 3;
        for (size_t i = 0; i < history.size(); i++)
        {
            auto text = history[i].text();
            correct_text = text == Pubnub::String("one") ||
                text == Pubnub::String("two") ||
                text == Pubnub::String("three");
        }
        });
    std::this_thread::sleep_for(std::chrono::seconds(2));
    channel.send_text("thread_start_message");
    std::this_thread::sleep_for(std::chrono::seconds(15));
    ASSERT_TRUE(correct_amount);
    ASSERT_TRUE(correct_text);
}

TEST_F(ThreadsTests, TestThreadChannelParentChannelPinning) {
    auto channel = chat->create_public_conversation("thread_parent_pinning_test_channel", Pubnub::ChatChannelData{});

    auto pinned = false;
    auto unpinned = false;
    channel.join([&](Pubnub::Message message) {
        auto thread = message.create_thread();
        thread.join([&](Pubnub::Message message) {});

        std::this_thread::sleep_for(std::chrono::seconds(2));

        thread.send_text("thread_init_message");

        std::this_thread::sleep_for(std::chrono::seconds(6));

        auto history = thread.get_thread_history("99999999999999999", "00000000000000000", 1);
        auto message_to_pin = history[0];

        channel = thread.pin_message_to_parent_channel(message_to_pin);
        try
        {
            auto pinned_message = channel.get_pinned_message();
            if (pinned_message.text() == Pubnub::String("thread_init_message")) {
                pinned = true;
            }
        }
        catch (const std::exception&)
        {
            pinned = false;
        }

        channel = thread.unpin_message_from_parent_channel();

        try
        {
            auto pinned_message = channel.get_pinned_message();
            unpinned = false;
        }
        catch (const std::exception&)
        {
            unpinned = true;
        }
        });
    std::this_thread::sleep_for(std::chrono::seconds(3));
    channel.send_text("thread_start_message");
    std::this_thread::sleep_for(std::chrono::seconds(10));
    ASSERT_TRUE(pinned);
    ASSERT_TRUE(unpinned);
}

TEST_F(ThreadsTests, TestThreadMessageUpdate) {
    auto channel = chat->create_public_conversation("thread_parent_pinning_test_channel", Pubnub::ChatChannelData{});

    auto updated = false;
    channel.join([&](Pubnub::Message message) {
        auto thread = message.create_thread();
        thread.join([&](Pubnub::Message message) {});

        std::this_thread::sleep_for(std::chrono::seconds(2));

        thread.send_text("thread_init_message");

        std::this_thread::sleep_for(std::chrono::seconds(4));

        auto history = thread.get_thread_history("99999999999999999", "00000000000000000", 1);
        auto thread_message = history[0];

        thread_message.stream_updates([&](Pubnub::Message updated_message) {
            if (updated_message.text() == Pubnub::String("new_text")) {
                updated = true;
            }
            });
        thread_message.edit_text("new_text");
        
        });
    std::this_thread::sleep_for(std::chrono::seconds(3));
    channel.send_text("thread_start_message");
    std::this_thread::sleep_for(std::chrono::seconds(10));
    ASSERT_TRUE(updated);
}