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

class UserTests : public ::testing::Test {
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

        Pubnub::ChatConfig config;
        config.store_user_activity_timestamps = true;
        chat.reset(new Pubnub::Chat(Pubnub::Chat::init(
            publish_key, subscribe_key, "user_tests_user", config)));
    }

    // This method will be called after each test in the test suite  
    void TearDown() override {
        // Do your cleanup operations here  
    }

    //To generate random strings
    std::string gen_random(const int len) {
        static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        std::string tmp_s;
        tmp_s.reserve(len);

        for (int i = 0; i < len; ++i) {
            tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
        }

        return tmp_s;
    }
};

TEST_F(UserTests, TestUserActive) {
    auto channel =
        chat->create_public_conversation("active_test_channel", Pubnub::ChatChannelData {});
    channel.join([&](Pubnub::Message message) {});

    std::this_thread::sleep_for(std::chrono::seconds(4));

    Pubnub::User current_user = chat->current_user();
    ASSERT_TRUE(current_user.active());
}

TEST_F(UserTests, TestLastUserActive) {
    auto channel =
        chat->create_public_conversation("last_active_test_channel", Pubnub::ChatChannelData {});
    channel.join([&](Pubnub::Message message) {});

    std::this_thread::sleep_for(std::chrono::seconds(4));

    Pubnub::User current_user = chat->current_user();
    ASSERT_TRUE(std::stoll(current_user.last_active_timestamp().value().c_str()) > 0);
}

TEST_F(UserTests, TestUserUpdate) {
    Pubnub::User test_user;
    try
    {
        test_user = chat->get_user("test_user");
    }
    catch (const std::exception&)
    {
        test_user = chat->create_user("test_user", Pubnub::ChatUserData{});
    }
    srand((unsigned)time(NULL) * getpid());
    auto random_name = gen_random(10);
    auto updated = false;
    test_user.stream_updates([&](Pubnub::User user) {
        if (user.user_data().user_name == Pubnub::String(random_name)) {
            updated = true;
        }
        });

    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto new_user_data = Pubnub::ChatUserData();
    new_user_data.user_name = random_name;
    test_user.update(new_user_data);

    std::this_thread::sleep_for(std::chrono::seconds(8));

    ASSERT_TRUE(updated);
}