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

class ChannelTests : public ::testing::Test {
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

TEST_F(ChannelTests, TestGetUserSuggestions) {
    auto current_user = chat->current_user();
    auto user_data = Pubnub::ChatUserData{};
    user_data.user_name = "Testinski";
    current_user.update(user_data);

    std::this_thread::sleep_for(std::chrono::seconds(4));

    auto channel = chat->create_public_conversation("user_suggestions_channel", Pubnub::ChatChannelData{});
    channel.join([&channel](Pubnub::Message message) {});

    std::this_thread::sleep_for(std::chrono::seconds(3));

    auto suggestions = chat->get_user_suggestions("@Test");

    auto contains_name = std::any_of(suggestions.begin(), suggestions.end(), [](const Pubnub::User& user) {
        return user.user_data().user_name == Pubnub::String("Testinski");
        });

    ASSERT_TRUE(contains_name);
}

TEST_F(ChannelTests, TestGetMemberships) {
    auto current_user = chat->current_user();
    auto channel =
        chat->create_public_conversation("memberships_channel", Pubnub::ChatChannelData {});
    channel.join([&channel](Pubnub::Message message) {});

    std::this_thread::sleep_for(std::chrono::seconds(3));

    auto suggestions = channel.get_members();

    ASSERT_TRUE(suggestions.memberships.size() >= 1);
}

TEST_F(ChannelTests, TestStartTyping) {
    auto current_user = chat->current_user();
    Pubnub::User talk_user;
    try
    {
        talk_user = chat->get_user("typing_test_user");
    }
    catch (const std::exception&)
    {
        talk_user = chat->create_user("typing_test_user", Pubnub::ChatUserData{});
    }
    auto channel = chat->create_direct_conversation(talk_user, "typing_test_channel", Pubnub::ChatChannelData{}).created_channel;
    channel.join([&channel](Pubnub::Message message) {});

    std::this_thread::sleep_for(std::chrono::seconds(3));

    auto got_typing_callback = false;
    channel.get_typing([&](Pubnub::Vector<Pubnub::String> typing_users) {
        for (size_t i = 0; i < typing_users.size(); i++)
        {
            if (typing_users[i] == Pubnub::String("channel_tests_user")) {
                got_typing_callback = true;
                break;
            }
        }
        });
    channel.start_typing();

    std::this_thread::sleep_for(std::chrono::seconds(4));

    ASSERT_TRUE(got_typing_callback);
}

TEST_F(ChannelTests, TestStopTyping) {
    auto current_user = chat->current_user();
    Pubnub::User talk_user;
    try
    {
        talk_user = chat->get_user("typing_stop_test_user");
    }
    catch (const std::exception&)
    {
        talk_user = chat->create_user("typing_stop_test_user", Pubnub::ChatUserData{});
    }
    auto channel = chat->create_direct_conversation(talk_user, "typing_test_channel", Pubnub::ChatChannelData{}).created_channel;
    channel.join([&channel](Pubnub::Message message) {});

    std::this_thread::sleep_for(std::chrono::seconds(3));

    channel.start_typing();

    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto got_stopped_typing_callback = false;
    channel.get_typing([&](Pubnub::Vector<Pubnub::String> typing_users) {
        if (typing_users.size() == 0) {
            got_stopped_typing_callback = true;
        }
        });
    channel.stop_typing();
    
    std::this_thread::sleep_for(std::chrono::seconds(5));

    ASSERT_TRUE(got_stopped_typing_callback);
}

TEST_F(ChannelTests, TestStopTypingFromTimer) {
    auto current_user = chat->current_user();
    Pubnub::User talk_user;
    try
    {
        talk_user = chat->get_user("typing_stop_test_user");
    }
    catch (const std::exception&)
    {
        talk_user = chat->create_user("typing_stop_test_user", Pubnub::ChatUserData{});
    }
    auto channel = chat->create_direct_conversation(talk_user, "typing_test_channel", Pubnub::ChatChannelData{}).created_channel;
    channel.join([&channel](Pubnub::Message message) {});

    std::this_thread::sleep_for(std::chrono::seconds(3));

    channel.start_typing();

    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto got_stopped_typing_callback = false;
    channel.get_typing([&](Pubnub::Vector<Pubnub::String> typing_users) {
        if (typing_users.size() == 0) {
            got_stopped_typing_callback = true;
        }
        });

    std::this_thread::sleep_for(std::chrono::seconds(10));

    ASSERT_TRUE(got_stopped_typing_callback);
}

TEST_F(ChannelTests, TestPinAndUnPinMessage) {
    auto channel = chat->create_public_conversation("pin_message_test_channel", Pubnub::ChatChannelData{});
    auto pinned = false;
    auto unpinned = false;
    channel.join([&](Pubnub::Message message) {
        channel = channel.pin_message(message);
        try
        {
            auto pinned_message = channel.get_pinned_message();
            if (pinned_message.text() == Pubnub::String("message_to_pin")) {
                pinned = true;
            }
        }
        catch (const std::exception&)
        {
            pinned = false;
        }
        channel = channel.unpin_message();
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
    channel.send_text("message_to_pin");
    std::this_thread::sleep_for(std::chrono::seconds(3));
    ASSERT_TRUE(pinned);
    ASSERT_TRUE(unpinned);
}

TEST_F(ChannelTests, TestEmitUserMention) {
    auto channel = chat->create_public_conversation("user_mention_test_channel", Pubnub::ChatChannelData{});
    channel.join([&](Pubnub::Message message) {});
    std::this_thread::sleep_for(std::chrono::seconds(3));
    auto mentioned = false;
    chat->listen_for_events("channel_tests_user", Pubnub::PCET_MENTION, [&](Pubnub::Event event_object) {
        std::cout << "AAAAAA " << event_object.payload << std::endl;
        if (event_object.payload.find("heyyyy") != std::string::npos) {
            mentioned = true;
        }
        });
    std::this_thread::sleep_for(std::chrono::seconds(3));
    channel.emit_user_mention("channel_tests_user", "99999999999999999", "heyyyy");
    std::this_thread::sleep_for(std::chrono::seconds(3));
    ASSERT_TRUE(mentioned);
}