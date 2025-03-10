#include "chat.hpp"
#include "channel.hpp"
#include "pubnub_chat/chat.hpp"
#include <gtest/gtest.h>
#include "string.hpp"
#include "pubnub_chat/message_draft.hpp"
#include "pubnub_chat/message.hpp"
#include "pubnub_chat/chat.hpp"
#include "pubnub_chat/vector.hpp"
#include <algorithm>
#include <thread>

class MessageDraftTests : public :: testing::Test {
    protected:
        std::unique_ptr<Pubnub::Chat> chat;
        std::unique_ptr<Pubnub::Channel> channel;

        // This method will be called before each test in the test suite  
        void SetUp() override {
            Pubnub::String publish_key = std::getenv("PUBNUB_PUBLISH_KEY");
            if (publish_key.empty()) {
                publish_key = "pub-c-79c582a2-d7a4-4ee7-9f28-7a6f1b7fa11c";
            }
            Pubnub::String subscribe_key = std::getenv("PUBNUB_SUBSCRIBE_KEY");
            if (subscribe_key.empty()) {
                subscribe_key = "sub-c-ca0af928-f4f9-474c-b56e-d6be81bf8ed0";
            }

            chat.reset(new Pubnub::Chat(Pubnub::Chat::init(
                publish_key, subscribe_key, "test_id", Pubnub::ChatConfig())));

            channel.reset(new Pubnub::Channel(chat->create_public_conversation("test_channel", Pubnub::ChatChannelData{})));
        }

        // This method will be called after each test in the test suite  
        void TearDown() override {
            // Do your cleanup operations here  
        }
};

TEST_F(MessageDraftTests, TestCreateMessageDraft) {
    ASSERT_NO_THROW(channel->create_message_draft());
}

TEST_F(MessageDraftTests, TestInsertAndRemoveText) {
    auto message_draft = channel->create_message_draft();

    auto insert_success = false;
    auto remove_success = false;
    message_draft.add_change_listener([&](const Pubnub::Vector<Pubnub::MessageElement>& elements) {
        std::for_each(elements.begin(), elements.end(), [&](const Pubnub::MessageElement& element) {
            if (element.text == Pubnub::String("test insert")) {
                insert_success = true;
            }
            else if (element.text == Pubnub::String("insert")) {
                remove_success = true;
            }
            });
        }
    );
    message_draft.insert_text(0, "test insert");
    
    std::this_thread::sleep_for(std::chrono::seconds(2));

    ASSERT_TRUE(insert_success);

    message_draft.remove_text(0, 5);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    ASSERT_TRUE(remove_success);
}