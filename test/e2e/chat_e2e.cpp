#include <gtest/gtest.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <sstream>  
#include <future>

#include "channel.hpp"
#include "chat.hpp"
#include "pubnub_chat/chat.hpp"
#include "pubnub_chat/enums.hpp"
#include "pubnub_chat/message.hpp"
#include "pubnub_chat/message_draft.hpp"
#include "pubnub_chat/vector.hpp"
#include "e2e_tests_helpers.h"
#include "string.hpp"

class ChatTests: public ::testing::Test {
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
                publish_key,
                subscribe_key,
                "chat_tests_user",
                Pubnub::ChatConfig()
            )));

            channel.reset(new Pubnub::Channel(
                chat->create_public_conversation("chat_tests_channel", Pubnub::ChatChannelData {})
            ));
        }

        // This method will be called after each test in the test suite
        void TearDown() override {
            // Do your cleanup operations here
        }
};

TEST_F(ChatTests, TestGetCurrentUserMentions) {
    auto send_text_params = Pubnub::SendTextParams();
    auto mentioned_user = Pubnub::MentionedUser();
    mentioned_user.id = chat->current_user().user_id();
    std::map<int, Pubnub::MentionedUser> mentions;
    mentions[0] = mentioned_user;
    send_text_params.mentioned_users = Pubnub::Map<int, Pubnub::MentionedUser>(mentions);

    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();
    channel->join([&](Pubnub::Message message) {
        if (message.text() == Pubnub::String("mention_message")) {
            promise.set_value(true);
        }
        });
    
    std::this_thread::sleep_for(std::chrono::seconds(2));

    channel->send_text("mention_message");

    if (future.wait_for(std::chrono::milliseconds(6000)) == std::future_status::timeout) {
        std::cout << "Timeout waiting for message" << std::endl;
        FAIL();
    }

    auto fetched_mentions =
        chat->get_current_user_mentions("99999999999999999", "00000000000000000", 10).user_mention_data;

    auto found_mention = false;
    for (size_t i = 0; i < fetched_mentions.size(); i++) {
        if (fetched_mentions[i].channel_id == channel->channel_id()
            && fetched_mentions[i].message.text() == Pubnub::String("mention_message")) {
            found_mention = true;
            break;
        }
    }
    ASSERT_TRUE(found_mention);
}

TEST_F(ChatTests, TestGetCurrentUser) {
    auto current_user = chat->current_user();
    ASSERT_TRUE(current_user.user_id() == Pubnub::String("chat_tests_user"));
}

TEST_F(ChatTests, TestGetEventHistory) {
    chat->emit_chat_event(
        Pubnub::PCET_CUSTOM,
        channel->channel_id(),
        "{\"test\":\"some_nonsense\"}"
    );

    std::this_thread::sleep_for(std::chrono::seconds(5));

    auto history = chat->get_events_history(
        channel->channel_id(),
        "99999999999999999",
        "00000000000000000",
        50
    ).events;

    auto found_event = false;
    for (size_t i = 0; i < history.size(); i++) {
        if (history[i].channel_id == channel->channel_id()
            && history[i].payload == Pubnub::String("{\"test\":\"some_nonsense\"}")) {
            found_event = true;
            break;
        }
    }
    ASSERT_TRUE(found_event);
}

TEST_F(ChatTests, TestGetUsers) {
    auto users = chat->get_users("id LIKE \"chat_*\"").users;
    auto found_user = false;
    for (size_t i = 0; i < users.size(); i++) {
        if (users[i].user_id() == Pubnub::String("chat_tests_user")) {
            found_user = true;
            break;
        }
    }
    ASSERT_TRUE(found_user);
}

TEST_F(ChatTests, TestGetChannels) {
    auto channels = chat->get_channels("id LIKE \"chat_*\"").channels;
    auto found_channel = false;
    for (size_t i = 0; i < channels.size(); i++) {
        if (channels[i].channel_id() == Pubnub::String("chat_tests_channel")) {
            found_channel = true;
            break;
        }
    }
    ASSERT_TRUE(found_channel);
}

TEST_F(ChatTests, TestCreateDirectConversation) {
    auto current_user = chat->current_user();
    Pubnub::User talk_user;
    try {
        talk_user = chat->get_user("direct_convo_test_user");
    } catch (const std::exception&) {
        talk_user = chat->create_user("direct_convo_test_user", Pubnub::ChatUserData {});
    }
    auto direct_channel = 
        chat->create_direct_conversation(talk_user, "direct_test_channel", Pubnub::ChatChannelData{});
    ASSERT_TRUE(direct_channel.created_channel.channel_id() == Pubnub::String("direct_test_channel"));
    ASSERT_TRUE(direct_channel.host_membership.user.user_id() == current_user.user_id());
    ASSERT_TRUE(direct_channel.invitees_memberships[0].user.user_id() == talk_user.user_id());
}

TEST_F(ChatTests, TestCreateGroupConversation) {
    auto current_user = chat->current_user();

    auto prepare_user = [&](Pubnub::User& user, int index) {
        std::stringstream ss;
        ss << "group_test_user_" << index;
        auto id = ss.str();  
        try {
            user = chat->get_user(id);
        } catch (const std::exception&) {
            user = chat->create_user(id, Pubnub::ChatUserData {});
        }
    };

    Pubnub::User talk_user_0;
    prepare_user(talk_user_0, 0);
    Pubnub::User talk_user_1;
    prepare_user(talk_user_1, 1);
    Pubnub::User talk_user_2;
    prepare_user(talk_user_2, 2);

    auto users = Pubnub::Vector<Pubnub::User>();
    users.push_back(talk_user_0);
    users.push_back(talk_user_1);
    users.push_back(talk_user_2);

    auto group_convo = chat->create_group_conversation(
        users,
        "group_conversation_test",
        Pubnub::ChatChannelData()
    );
   
    ASSERT_TRUE(group_convo.created_channel.channel_id() == Pubnub::String("group_conversation_test"));
    ASSERT_TRUE(group_convo.host_membership.user.user_id() == current_user.user_id());
    ASSERT_TRUE(group_convo.invitees_memberships[0].user.user_id() == talk_user_0.user_id());
}

TEST_F(ChatTests, TestForwardMessage) {
    auto forwarding_channel =
        chat->create_public_conversation("chat_tests_channel", Pubnub::ChatChannelData {});
    std::promise<Pubnub::String> promise;
    std::future<Pubnub::String> future = promise.get_future();
    forwarding_channel.join([&](Pubnub::Message message) {
        promise.set_value(message.text());
    });
    channel->join([&](Pubnub::Message message) {
        if (message.text() == Pubnub::String("forwarded_message")) {
            chat->forward_message(message, forwarding_channel);
        }
    });
    if (future.wait_for(std::chrono::milliseconds(10000)) == std::future_status::timeout) {
        std::cout << "Timeout waiting for message" << std::endl;
        FAIL();
    }

    ASSERT_TRUE(future.get() == Pubnub::String("forwarded_message"));
}

TEST_F(ChatTests, TestEmitEvent) {
    
    std::promise<Pubnub::Event> promise;
    std::future<Pubnub::Event> future = promise.get_future();
    
    chat->listen_for_events(channel->channel_id(), Pubnub::PCET_CUSTOM, [&](Pubnub::Event received_event) { 
        promise.set_value(received_event);
    });
    
    std::this_thread::sleep_for(std::chrono::seconds(3));

    chat->emit_chat_event(
        Pubnub::PCET_CUSTOM,
        channel->channel_id(),
        "{\"test\":\"some_nonsense\"}"
    );

    if (future.wait_for(std::chrono::milliseconds(10000)) == std::future_status::timeout) {
        std::cout << "Timeout waiting for event" << std::endl;
        FAIL();
    }

    auto received_event = future.get();
    ASSERT_TRUE(received_event.payload == Pubnub::String("{\"test\":\"some_nonsense\"}"));
}

TEST_F(ChatTests, TestGetUnreadMessagesCounts) {
    std::promise<void> promise;
    std::future<void> future = promise.get_future();
    
    channel->join([&](Pubnub::Message message) { 
        promise.set_value();
        });

    std::this_thread::sleep_for(std::chrono::seconds(3));

    channel->send_text("wololo");

    if (future.wait_for(std::chrono::milliseconds(10000)) == std::future_status::timeout) {
        std::cout << "Timeout waiting for message" << std::endl;
        FAIL();
    }

    auto unreads = chat->get_unread_messages_counts();
    auto has_unread = false;
    for (size_t i = 0; i < unreads.size(); i++) {
        if (unreads[i].channel.channel_id() == channel->channel_id() && unreads[i].count >= 1) {
            has_unread = true;
            break;
        }
    }

    ASSERT_TRUE(has_unread);
}

TEST_F(ChatTests, TestMarkAllMessagesAsRead) {
    std::promise<void> promise;
    std::future<void> future = promise.get_future();

    channel->join([&](Pubnub::Message message) { promise.set_value(); });

    std::this_thread::sleep_for(std::chrono::seconds(3));

    channel->send_text("wololo");

    if (future.wait_for(std::chrono::milliseconds(10000)) == std::future_status::timeout) {
        std::cout << "Timeout waiting for message" << std::endl;
        FAIL();
    }

    chat->mark_all_messages_as_read();

    std::this_thread::sleep_for(std::chrono::seconds(3));

    auto unreads = chat->get_unread_messages_counts();
    auto zero_unreads = true;
    for (size_t i = 0; i < unreads.size(); i++) {
        if (unreads[i].channel.channel_id() == channel->channel_id() && unreads[i].count > 0) {
            zero_unreads = false;
            break;
        }
    }

    ASSERT_TRUE(zero_unreads);
}