#include "chat.hpp"
#include "channel.hpp"
#include "pubnub_chat/chat.hpp"
#include <gtest/gtest.h>
#include "string.hpp"
#include "pubnub_chat/message_draft.hpp"
#include "pubnub_chat/message.hpp"
#include "pubnub_chat/chat.hpp"
#include "pubnub_chat/vector.hpp"
#include "e2e_tests_helpers.h"
#include <algorithm>
#include <thread>
#include <future>

class MessageDraftTests : public :: testing::Test {
    protected:
        std::unique_ptr<Pubnub::Chat> chat;
        std::unique_ptr<Pubnub::Channel> channel;

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

TEST_F(MessageDraftTests, TestInsertSuggestedMention) {
    auto message_draft = channel->create_message_draft();

    auto current_user = chat->current_user();
    auto user_data = Pubnub::ChatUserData {};
    user_data.user_name = "Testinski";
    current_user.update(user_data);

    std::this_thread::sleep_for(std::chrono::seconds(4));

    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();
    
    message_draft.add_change_listener(
        [&](const Pubnub::Vector<Pubnub::MessageElement>& elements,
            const Pubnub::Vector<Pubnub::SuggestedMention>& suggestions) {
       
            for (size_t i = 0; i < suggestions.size(); i++) {
                if (suggestions[i].target.get_target() == current_user.user_id()) {
                    promise.set_value(true);
                    break;
                }
            }
        
    });
    message_draft.insert_text(0, "test insert @Test");

    if (future.wait_for(std::chrono::milliseconds(10000)) == std::future_status::timeout) {
        std::cout << "Timeout waiting for suggestion" << std::endl;
        FAIL();
    }

    ASSERT_TRUE(future.get());
}

TEST_F(MessageDraftTests, TestAddMention) {
    auto message_draft = channel->create_message_draft();
    auto current_user = chat->current_user();
    message_draft.insert_text(0, "wololo and stuff");

    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();

    message_draft.add_change_listener(
        [&](const Pubnub::Vector<Pubnub::MessageElement>& elements,
            const Pubnub::Vector<Pubnub::SuggestedMention>& suggestions) {
            for (size_t i = 0; i < elements.size(); i++) {
                if (elements[i].target.value().get_target() == current_user.user_id()) {
                    promise.set_value(true);
                    break;
                }
            }
        }
    );
    
    auto mention_target =
        Pubnub::MentionTarget::user(current_user.user_id());
    message_draft.add_mention(0, 6, mention_target);

    if (future.wait_for(std::chrono::milliseconds(10000)) == std::future_status::timeout) {
        std::cout << "Timeout waiting for mention" << std::endl;
        FAIL();
    }

    ASSERT_TRUE(future.get());
}

TEST_F(MessageDraftTests, TestUpdate) {
    auto message_draft = channel->create_message_draft();

    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();

    message_draft.add_change_listener(
        [&](const Pubnub::Vector<Pubnub::MessageElement>& elements,
            const Pubnub::Vector<Pubnub::SuggestedMention>& suggestions) {
            for (size_t i = 0; i < elements.size(); i++) {
                if (elements[i].text == Pubnub::String("wololo and stuff")) {
                    promise.set_value(true);
                    break;
                }
            }
        }
    );
    message_draft.update("wololo and stuff");

    if (future.wait_for(std::chrono::milliseconds(10000)) == std::future_status::timeout) {
        std::cout << "Timeout waiting for update" << std::endl;
        FAIL();
    }

    ASSERT_TRUE(future.get());
}

TEST_F(MessageDraftTests, TestSend) {
    auto message_draft = channel->create_message_draft();

    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();

    channel->join([&](Pubnub::Message message) { 
        if (message.text() == Pubnub::String("wololo and stuff")) {
            promise.set_value(true);
        }
    });
    message_draft.update("wololo and stuff");
    message_draft.send();

    if (future.wait_for(std::chrono::milliseconds(10000)) == std::future_status::timeout) {
        std::cout << "Timeout waiting for message" << std::endl;
        FAIL();
    }

    ASSERT_TRUE(future.get());
}