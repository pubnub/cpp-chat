#include <gtest/gtest.h>

#include <algorithm>
#include <iostream>
#include <future>
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
#include "e2e_tests_helpers.h"
#include "string.hpp"

class MembershipTests: public ::testing::Test {
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

TEST_F(MembershipTests, TestGetMemberships) {
    auto current_user = chat->current_user();
    auto channel =
        chat->create_public_conversation("some_public_channel", Pubnub::ChatChannelData {});
    channel.join([&channel](Pubnub::Message message) {});

    std::this_thread::sleep_for(std::chrono::seconds(4));

    auto memberships = current_user.get_memberships();

    auto contains =
        std::any_of(memberships.memberships.begin(), memberships.memberships.end(), [](const Pubnub::Membership& membership) { 
        return membership.channel.channel_id() == Pubnub::String("some_public_channel");
        });

    ASSERT_TRUE(contains);
}

TEST_F(MembershipTests, TestUpdateMembership) {
    auto current_user = chat->current_user();
    auto channel =
        chat->create_public_conversation("a_public_channel", Pubnub::ChatChannelData {});
    channel.join([&channel](Pubnub::Message message) {});

    std::this_thread::sleep_for(std::chrono::seconds(4));

    auto test_membership = current_user.get_memberships().memberships[0];

    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();

    test_membership.stream_updates([&](Pubnub::Membership membership) {
        if (membership.membership_data().custom_data_json == Pubnub::String("{\"key\":\"value\"}")) {
            promise.set_value(true);
        }
    });
    std::this_thread::sleep_for(std::chrono::seconds(1));
    test_membership.update("{\"key\": \"value\"}");

    if (future.wait_for(std::chrono::milliseconds(8000)) == std::future_status::timeout) {
        std::cout << "Timeout waiting for membership update" << std::endl;
        FAIL();
    }

    auto received_correct_update = future.get();

    ASSERT_TRUE(received_correct_update);
}

TEST_F(MembershipTests, TestInvite) {
    auto current_user = chat->current_user();
    Pubnub::User user;
    try {
        user = chat->get_user("some_guy");
    } catch (const std::exception&) {
        user = chat->create_user("some_guy", Pubnub::ChatUserData {});
    }
    auto users = Pubnub::Vector<Pubnub::User>();
    users.push_back(current_user);
    auto channel = chat->create_group_conversation(users, "some_group", Pubnub::ChatChannelData {}).created_channel;
    auto invite_membership = channel.invite(user);
    ASSERT_TRUE(
        invite_membership.channel.channel_id() == Pubnub::String("some_group")
        && invite_membership.user.user_id() == Pubnub::String("some_guy")
    );
}

TEST_F(MembershipTests, TestInviteMultiple) {
    auto current_user = chat->current_user();
    Pubnub::User user_0;
    try {
        user_0 = chat->get_user("some_guy");
    } catch (const std::exception&) {
        user_0 = chat->create_user("some_guy", Pubnub::ChatUserData {});
    }
    Pubnub::User user_1;
    try {
        user_1 = chat->get_user("some_other_guy");
    } catch (const std::exception&) {
        user_1 = chat->create_user("some_other_guy", Pubnub::ChatUserData {});
    }
    
    auto start_users = Pubnub::Vector<Pubnub::User>();
    start_users.push_back(current_user);
    
    auto channel =
        chat->create_group_conversation(start_users, "some_group", Pubnub::ChatChannelData {})
                       .created_channel;
    
    auto invited_users = Pubnub::Vector<Pubnub::User>();
    invited_users.push_back(user_0);
    invited_users.push_back(user_1);

    auto invited_memberships = channel.invite_multiple(invited_users);
    
    ASSERT_TRUE(invited_memberships.size() == 2);

    auto contains_first = std::any_of(
        invited_memberships.begin(),
        invited_memberships.end(),
        [](const Pubnub::Membership& membership) {
            return (
                membership.channel.channel_id() == Pubnub::String("some_group")
                && membership.user.user_id() == Pubnub::String("some_guy")
            );
        }
    );
    auto contains_second = std::any_of(
        invited_memberships.begin(),
        invited_memberships.end(),
        [](const Pubnub::Membership& membership) {
            return (
                membership.channel.channel_id() == Pubnub::String("some_group")
                && membership.user.user_id() == Pubnub::String("some_other_guy")
            );
        }
    );

    ASSERT_TRUE(contains_first && contains_second);
}

TEST_F(MembershipTests, TestLastRead) {
    auto current_user = chat->current_user();
    auto test_channel =
        chat->create_public_conversation("last_read_test_channel", Pubnub::ChatChannelData {});

    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();

    test_channel.join([&](Pubnub::Message message) {
        auto memberships = current_user.get_memberships();
        auto membership_it = std::find_if(
            memberships.memberships.begin(),
            memberships.memberships.end(),
            [&test_channel](const Pubnub::Membership& m) {
                return m.channel.channel_id() == test_channel.channel_id();
            }
        );
        if (membership_it == memberships.memberships.end()) {
            FAIL();
            return;
        }

        auto membership = *membership_it;

        membership = membership.set_last_read_message(message);
        auto last_time_token = membership.last_read_message_timetoken();
        ASSERT_TRUE(last_time_token == message.timetoken());
        membership = membership.set_last_read_message_timetoken("99999999999999999");
        ASSERT_TRUE(
            membership.last_read_message_timetoken() == Pubnub::String("99999999999999999")
        );
        promise.set_value(true);
    });

    std::this_thread::sleep_for(std::chrono::seconds(4));

    test_channel.send_text("some_message");

    if (future.wait_for(std::chrono::seconds(9)) == std::future_status::timeout) {
        std::cout << "Timeout waiting for message processing" << std::endl;
        FAIL();
    }

    auto received_correct_update = future.get();
    ASSERT_TRUE(received_correct_update);
}

TEST_F(MembershipTests, TestUnreadMessagesCount) {
    auto current_user = chat->current_user();
    auto unread_channel = chat->create_public_conversation(
        "test_channel_unreads", Pubnub::ChatChannelData {}
    );

    unread_channel.join([](Pubnub::Message message) {});

    std::this_thread::sleep_for(std::chrono::milliseconds(3500));

    unread_channel.send_text("one");
    unread_channel.send_text("two");
    unread_channel.send_text("three");

    std::this_thread::sleep_for(std::chrono::milliseconds(8000));

    auto memberships = unread_channel.get_members();
    auto membership_it = std::find_if(
        memberships.memberships.begin(),
        memberships.memberships.end(),
        [&current_user](const Pubnub::Membership& m) { return m.user.user_id() == current_user.user_id(); }
    );
    int unread_count = -1;
    if (membership_it != memberships.memberships.end()) {
        unread_count = membership_it->get_unread_messages_count();
    }

    ASSERT_TRUE(unread_count >= 3);
}  

