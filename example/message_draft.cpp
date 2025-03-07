#include "pubnub_chat/message_draft.hpp"
#include "pubnub_chat/message.hpp"
#include "pubnub_chat/chat.hpp"
#include "pubnub_chat/vector.hpp"
#include <algorithm>
#include <thread>

void prepare_users_to_mention(Pubnub::Chat& chat);
void prepare_general_channel(Pubnub::Chat& chat);

int main() {
    auto subscribe_key = std::getenv("PUBNUB_SUBSCRIBE_KEY");
    auto publish_key = std::getenv("PUBNUB_PUBLISH_KEY");
    auto user_id = std::getenv("USER_ID");
    
    // Create a chat instance
    auto chat = Pubnub::Chat::init(publish_key, subscribe_key, user_id, Pubnub::ChatConfig{});

    // Prepare users and #general channel to make mentions working as expected.
    // You probably can skip this step if you have users already prepared.
    prepare_users_to_mention(chat);
    prepare_general_channel(chat);

    // Get user you want with which you want to interact
    auto user = chat.get_user("user1");

    // Create a direct conversation
    auto channel = chat.create_direct_conversation(user, "some_direct", Pubnub::ChatChannelData{}).created_channel;

    chat.create_public_conversation("general", Pubnub::ChatChannelData{});
    // Listen for messages in the channel
    channel.connect([](const Pubnub::Message& message) {
        std::cout << "Received message: " << message.text() << std::endl;
        std::cout << "Mentioned users:" << std::endl;
        for (const auto& mention : message.mentioned_users()) {
            std::cout << "- " << mention.name << " " << mention.id << std::endl;
        }
    });

    // Create a message draft for the channel
    auto draft = channel.create_message_draft();

    // Add listener for every change in the draft 
    draft.add_change_listener([](const Pubnub::Vector<Pubnub::MessageElement>& elements) {
            std::cout << "--- Message elements changed! ---" << std::endl;
            std::for_each(elements.begin(), elements.end(), [](const Pubnub::MessageElement& element) {
                std::cout << "- Element -" << std::endl;
                std::cout << "Text:" << element.text << std::endl;
                if (element.target.has_value()) {
                    std::cout << "Linking to:" << element.target.value().get_target() << std::endl;
                }
            });
    });

    // Add listener for every change in the draft with suggestions
    // WARNING: Using this listener might be expensive because it calls Pubnub API to get suggestions
    draft.add_change_listener(
            [&draft](const Pubnub::Vector<Pubnub::MessageElement>& elements, const Pubnub::Vector<Pubnub::SuggestedMention>& suggestions) {
            std::cout << "--- Message elements changed, here suggestions:! ---" << std::endl;
            // Same handling as above for elements

            // Here you have to implement user interaction to choose the right suggestion
            // You have to keep in mind that every insertion of suggestion is a change in the draft 
            // and it will trigger the listener again
            // For simplicity, we will just accept the first suggestion always
            if (suggestions.size() > 0) {
                draft.insert_suggested_mention(suggestions[0], suggestions[0].replace_to);
            }
    });

    // Simulation of user typing in your GUI
    draft.insert_text(0, "Hello");

    // User removes Hello and rewrites it again
    draft.update("Hello World!");

    // User continues typing
    draft.insert_text(12, " I want to ping @user1 for some reason!");

    // Adding two mentions to the message at the same time
    // This one is a little bit risky because it will return suggestions for both mentions 
    // and you have to handle it properly. Check the add_message_elements_listener with suggestions
    // above for more information.
    draft.insert_text(58, " Also I want to show @user2 the new #general !");

    // User removes "Also"
    draft.remove_text(58, 6); 

    // User sends the message (e.g. types Enter)
    draft.send();

    std::cout << "Message sent!" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "End of main" << std::endl;

    return 0;
}

void prepare_users_to_mention(Pubnub::Chat& chat) {
    try {
        Pubnub::ChatUserData user1_data;
        user1_data.user_name = "User1Nickname";
        chat.create_user("user1", user1_data);

        std::this_thread::sleep_for(std::chrono::seconds(1));

        Pubnub::ChatUserData user2_data;
        user2_data.user_name = "User2Nickname";
        chat.create_user("user2", user2_data);

        std::this_thread::sleep_for(std::chrono::seconds(1));
    } catch (...) {
        // Users are already prepared...
    }
}

void prepare_general_channel(Pubnub::Chat& chat) {
    try {
        Pubnub::ChatChannelData general_data;
        general_data.channel_name = "GeneralName";
        chat.create_public_conversation("general", general_data);
    } catch (...) {
        // General channel is already prepared...
    }
}
