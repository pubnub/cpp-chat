#include <gtest/gtest.h>

#include <algorithm>
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "channel.hpp"
#include "chat.hpp"
#include "pubnub_chat/chat.hpp"
#include "pubnub_chat/enums.hpp"
#include "pubnub_chat/message.hpp"
#include "pubnub_chat/message_draft.hpp"
#include "pubnub_chat/vector.hpp"
#include "string.hpp"

class RestrictionTests: public ::testing::Test {
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
                publish_key,
                subscribe_key,
                "membership_tests_user",
                Pubnub::ChatConfig()
            )));
        }

        // This method will be called after each test in the test suite
        void TearDown() override {
            // Do your cleanup operations here
        }
};

TEST_F(RestrictionTests, TestSetRestrictions) {
    Pubnub::User test_user;
    try {
        test_user = chat->get_user("test_user");
    } catch (const std::exception&) {
        test_user = chat->create_user("test_user", Pubnub::ChatUserData {});
    }
    auto channel = chat->create_public_conversation("new_channel", Pubnub::ChatChannelData {});

    std::this_thread::sleep_for(std::chrono::seconds(2));

    Pubnub::Restriction restriction;
    restriction.ban = true;
    restriction.mute = true;
    restriction.reason = "some_reason";

    channel.set_restrictions("test_user", restriction);

    std::this_thread::sleep_for(std::chrono::seconds(3));

    auto fetched_restrictions = test_user.get_channel_restrictions(channel);

    ASSERT_TRUE(fetched_restrictions.ban == true && fetched_restrictions.mute == true
                    && fetched_restrictions.reason == Pubnub::String("some_reason"));
}

TEST_F(RestrictionTests, TestGetRestrictionsSets) {
    Pubnub::User test_user;
    try {
        test_user = chat->get_user("test_user_2");
    } catch (const std::exception&) {
        test_user = chat->create_user("test_user_2", Pubnub::ChatUserData {});
    }
    auto channel = chat->create_public_conversation("new_channel_2", Pubnub::ChatChannelData {});

    std::this_thread::sleep_for(std::chrono::seconds(2));

    Pubnub::Restriction restriction;
    restriction.ban = true;
    restriction.mute = true;
    restriction.reason = "some_reason";

    channel.set_restrictions("test_user_2", restriction);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto channel_restrictions = channel.get_users_restrictions();
    auto user_restrictions = test_user.get_channels_restrictions();

    auto contains_first = std::any_of(
        channel_restrictions.restrictions.begin(),
        channel_restrictions.restrictions.end(),
        [&](const Pubnub::UserRestriction user_restriction) {
            return user_restriction.user_id == test_user.user_id();
        }
    );
    auto contains_second = std::any_of(
        user_restrictions.restrictions.begin(),
        user_restrictions.restrictions.end(),
        [&](const Pubnub::ChannelRestriction channel_restriction) {
            return channel_restriction.channel_id == channel.channel_id();
        }
    );

    ASSERT_TRUE(contains_first && contains_second);
}