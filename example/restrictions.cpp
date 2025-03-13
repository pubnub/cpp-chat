#include "restrictions.hpp"
#include <cstdlib>
#include "channel.hpp"
#include "pubnub_chat/chat.hpp"
#include "string.hpp"

void print_restriction(const Pubnub::Restriction& restriction);
void prepare_user(Pubnub::Chat& chat, const Pubnub::String& user_id);
void prepare_channel(Pubnub::Chat& chat, const Pubnub::String& channel_id);

int main() {
    auto subscribe_key = std::getenv("PUBNUB_SUBSCRIBE_KEY");
    auto publish_key = std::getenv("PUBNUB_PUBLISH_KEY");
    auto user_id = std::getenv("USER_ID");
    
    // Create a chat instance
    auto chat = Pubnub::Chat::init(publish_key, subscribe_key, user_id, Pubnub::ChatConfig{});

    // Chat Object restrictions
    prepare_user(chat, "user1");
    prepare_channel(chat, "channel1");

    auto chat_restricitons = Pubnub::Restriction{};   
    chat_restricitons.mute = true;
    chat_restricitons.reason = "mute for some reason";

    chat.set_restrictions("user1", "channel1", chat_restricitons);

    std::cout << "|Chat Object restrictions:|" << std::endl;
    auto user1 = chat.get_user("user1");
    auto channel1 = chat.get_channel("channel1");

    std::cout << "user1 restrictions for channel1: " << user1.user_id() << std::endl;
    print_restriction(user1.get_channel_restrictions(channel1));

    std::cout << "channel1 restrictions for user1: " << channel1.channel_id() << std::endl;
    print_restriction(channel1.get_user_restrictions(user1));

    // Channel Object restrictions
    prepare_user(chat, "user2");
    prepare_channel(chat, "channel2");
    
    auto channel = chat.get_channel("channel2");

    auto channel_restricitons = Pubnub::Restriction{};   
    chat_restricitons.ban = true;
    chat_restricitons.reason = "ban for some reason";

    channel.set_restrictions("user2", channel_restricitons);

    std::cout << "|Channel Object restrictions:|" << std::endl;
    auto user2 = chat.get_user("user2");

    std::cout << "user2 restrictions for channel2: " << std::endl;
    print_restriction(user2.get_channel_restrictions(channel));

    std::cout << "channel2 restrictions for user2: " << std::endl;
    print_restriction(channel.get_user_restrictions(user2));

    // User Object restrictions
    prepare_user(chat, "user3");
    prepare_channel(chat, "channel3");
    
    auto user = chat.get_user("user3");

    auto user_restricitons = Pubnub::Restriction{};
    user_restricitons.mute = true;
    user_restricitons.reason = "mute for some reason";

    user.set_restrictions("channel3", user_restricitons);

    std::cout << "|User Object restrictions:|" << std::endl;
    auto channel3 = chat.get_channel("channel3");

    std::cout << "user3 restrictions for channel3: " << std::endl;
    print_restriction(user.get_channel_restrictions(channel3));

    std::cout << "channel3 restrictions for user3: " << std::endl;
    print_restriction(channel3.get_user_restrictions(user));

    return 0;
}

void print_restriction(const Pubnub::Restriction& restriction) {
    std::cout << "Restrctions: {" << std::endl;
    std::cout << "  ban: " << restriction.ban << std::endl;
    std::cout << "  mute: " << restriction.mute << std::endl;
    std::cout << "  reason: " << restriction.reason << std::endl;
    std::cout << "}" << std::endl;
}

void prepare_user(Pubnub::Chat& chat, const Pubnub::String& user_id) {
    try {
        chat.create_user(user_id, Pubnub::ChatUserData{});
    } catch (const std::exception& e) {
        // User already exists
    }
}

void prepare_channel(Pubnub::Chat& chat, const Pubnub::String& channel_id) {
    try {
        chat.create_public_conversation(channel_id, Pubnub::ChatChannelData{});
    } catch (const std::exception& e) {
        // Channel already exists
    }
}

