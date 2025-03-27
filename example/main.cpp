#include "pubnub_chat/message.hpp"
#include "pubnub_chat/chat.hpp"
#include "pubnub_chat/chat_helpers.hpp"
#include "pubnub_chat/message_draft.hpp"
#include <chrono>
#include <thread>
#include <vector>
#include <map>

#define TOKEN_TTL 10
#define TOKEN_AUTH_USER_ID "pam_ccp_chat_user"

Pubnub::String get_init_chat_token(Pubnub::AccessManager& token_access_manager, Pubnub::String user_id);
Pubnub::String get_empty_token(Pubnub::AccessManager& token_access_manager);
Pubnub::String get_create_pub_conversation_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id);
Pubnub::String get_create_user_token(Pubnub::AccessManager& token_access_manager, Pubnub::String user_id);
Pubnub::String get_get_user_token(Pubnub::AccessManager& token_access_manager, Pubnub::String user_id);
Pubnub::String get_get_users_token(Pubnub::AccessManager& token_access_manager, std::vector<Pubnub::String> user_ids);
Pubnub::String get_create_dir_conversation_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String current_user_id, Pubnub::String invitee_id);
Pubnub::String get_create_group_conversation_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String current_user_id, std::vector<Pubnub::String> invitee_ids);
Pubnub::String get_get_channel_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id);
Pubnub::String get_read_channel_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id);
Pubnub::String get_get_channels_token(Pubnub::AccessManager& token_access_manager, std::vector<Pubnub::String> channel_ids);
Pubnub::String get_update_channel_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id);
Pubnub::String get_delete_channel_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id);
Pubnub::String get_update_user_token(Pubnub::AccessManager& token_access_manager, Pubnub::String user_id);
Pubnub::String get_delete_user_token(Pubnub::AccessManager& token_access_manager, Pubnub::String user_id);
Pubnub::String get_join_channel_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String user_id);
Pubnub::String get_leave_channel_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String user_id);
Pubnub::String get_channel_read_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id);
Pubnub::String get_channel_write_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id);
Pubnub::String get_channel_connect_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id);
Pubnub::String get_send_text_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String mentioned_user_id);
Pubnub::String get_set_restrictions_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String user_id);
Pubnub::String get_invite_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String invitee_id);
Pubnub::String get_invite_multiple_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, std::vector<Pubnub::String> invitee_ids);
Pubnub::String get_unread_messages_counts_token(Pubnub::AccessManager& token_access_manager, Pubnub::String current_user_id, std::vector<Pubnub::String> channels);
Pubnub::String get_mark_all_messages_as_read_token(Pubnub::AccessManager& token_access_manager, Pubnub::String current_user_id, std::vector<Pubnub::String> channels);
Pubnub::String get_thread_send_text_token(Pubnub::AccessManager& token_access_manager, Pubnub::String thread_channel_id, Pubnub::String parent_channel_id, Pubnub::String mentioned_user_id);
Pubnub::String get_remove_thread_token(Pubnub::AccessManager& token_access_manager, Pubnub::String thread_channel_id, Pubnub::String parent_channel_id);
Pubnub::String get_channel_stream_receipts_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id);
Pubnub::String get_membership_update_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String user_id);
Pubnub::String get_membership_set_last_read_message_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String user_id);
Pubnub::String get_message_pin_token(Pubnub::AccessManager& token_access_manager, Pubnub::String message_channel_id);
Pubnub::String get_message_unpin_token(Pubnub::AccessManager& token_access_manager, Pubnub::String message_channel_id);
Pubnub::String get_message_toggle_reaction_token(Pubnub::AccessManager& token_access_manager, Pubnub::String message_channel_id);


int main() {

    auto publish_key = std::getenv("PUBNUB_PUBLISH_KEY");
    auto subscribe_key = std::getenv("PUBNUB_SUBSCRIBE_KEY");
    auto secret_key = std::getenv("PUBNUB_SECRET_KEY");


    //This token is needed to init chat
    auto initial_auth_token = "p0F2AkF0GmfOvB5DdHRsGajAQ3Jlc6VEY2hhbqBDZ3JwoENzcGOgQ3VzcqBEdXVpZKFxcGFtX2NjcF9jaGF0X3VzZXIYaENwYXSlRGNoYW6gQ2dycKBDc3BjoEN1c3KgRHV1aWSgRG1ldGGgQ3NpZ1ggUA6E8e1Jps1cPGkW6lpzOKYzc-bjUtlLDyeUdiyv_uw=";
    
    auto config = Pubnub::ChatConfig();
    config.auth_key = initial_auth_token;

    auto token_chat = Pubnub::Chat::init(publish_key, subscribe_key, TOKEN_AUTH_USER_ID, config);
    auto token_access_manager = token_chat.access_manager();
    token_access_manager.set_secret_key(secret_key);


    //INIT CHAT
    std::cout << "Init Test" << std::endl;

    auto user_id = "cpp_chat_user";

    Pubnub::String init_chat_token = get_init_chat_token(token_access_manager, user_id);
    auto init_chat_config = Pubnub::ChatConfig();
    init_chat_config.auth_key = init_chat_token;
    auto chat = Pubnub::Chat::init(publish_key, subscribe_key, user_id, init_chat_config);



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;
    auto chat_access_manager = chat.access_manager();



    //CURRENT USER
    std::cout << "Current User Test" << std::endl;

    chat_access_manager.set_auth_token(get_get_user_token(token_access_manager, user_id));
    auto current_user = chat.current_user();



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //CREATE USER
    std::cout << "Create User Test" << std::endl;

    Pubnub::String create_user_id = Pubnub::String("cpp_chat_test_created_user");

    chat_access_manager.set_auth_token(get_create_user_token(token_access_manager, create_user_id));
    auto created_user = chat.create_user(create_user_id, Pubnub::ChatUserData());

    Pubnub::String another_user_id = Pubnub::String("cpp_chat_test_another_user");

    chat_access_manager.set_auth_token(get_create_user_token(token_access_manager, another_user_id));
    auto another_user = chat.create_user(another_user_id, Pubnub::ChatUserData());


        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //GET USER 
    std::cout << "Get User Test" << std::endl;

    Pubnub::String invitee_user_id = "cpp_chat_test_invited_user";

    chat_access_manager.set_auth_token(get_get_user_token(token_access_manager, invitee_user_id));
    auto invitee_user = chat.get_user(invitee_user_id);



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //GET USERS
    std::cout << "Get Users Test" << std::endl;

    //This shouldn't require any permissions
    auto users_from_get = chat.get_users("id LIKE \"cpp_chat_test\"", "", 5);



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //CREATE PUBLIC CONVERSATION
    std::cout << "Create Public Conversation Test" << std::endl;

    Pubnub::String public_channel_id = "cpp_chat_test_public_channel";

    chat_access_manager.set_auth_token(get_create_pub_conversation_token(token_access_manager, public_channel_id));
    auto public_channel = chat.create_public_conversation(public_channel_id, Pubnub::ChatChannelData());



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //CREATE DIRECT CONVERSATION
    std::cout << "Create Direct Conversation Test" << std::endl;

    Pubnub::String direct_channel_id = "cpp_chat_test_direct_channel";

    chat_access_manager.set_auth_token(get_create_dir_conversation_token(token_access_manager, direct_channel_id, current_user.user_id(), invitee_user_id));
    auto direct_channel = chat.create_direct_conversation(invitee_user, direct_channel_id, Pubnub::ChatChannelData());



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //CREATE GROUP CONVERSATION
    std::cout << "Create Group Conversation Test" << std::endl;

    Pubnub::String group_channel_id = "cpp_chat_test_group_channel";

    chat_access_manager.set_auth_token(get_create_group_conversation_token(token_access_manager, group_channel_id, current_user.user_id(), std::vector<Pubnub::String>{invitee_user_id, create_user_id}));
    auto group_channel = chat.create_group_conversation(std::vector<Pubnub::User>{invitee_user, created_user}, group_channel_id, Pubnub::ChatChannelData());
    


        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //GET CHANNEL
    std::cout << "Get Channel Test" << std::endl;

    chat_access_manager.set_auth_token(get_get_channel_token(token_access_manager, public_channel_id));
    auto channel_from_get = chat.get_channel(public_channel_id);
        


        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //GET CHANNELS
    std::cout << "Get Channels" << std::endl;

    //This shouldn't require any permissions
    auto channels_from_get = chat.get_channels("id LIKE \"cpp_chat_test\"", "", 5);



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //UPDATE CHANNEL
    std::cout << "Update Channel" << std::endl;

    chat_access_manager.set_auth_token(get_update_channel_token(token_access_manager, public_channel_id));
    auto new_channel_data = Pubnub::ChatChannelData();
    new_channel_data.description = Pubnub::get_now_timetoken();
    chat.update_channel(public_channel_id, new_channel_data);



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //UPDATE USER
    std::cout << "Update User" << std::endl;

    chat_access_manager.set_auth_token(get_update_user_token(token_access_manager, invitee_user_id));
    auto new_user_data = Pubnub::ChatUserData();
    new_user_data.external_id = Pubnub::get_now_timetoken();
    chat.update_user(invitee_user_id, new_user_data);



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;


/*


    //CHANNEL JOIN
    std::cout << "Channel.Join" << std::endl;

    chat_access_manager.set_auth_token(get_join_channel_token(token_access_manager, public_channel_id, user_id));
    auto join_lambda = [](Pubnub::Message message){};
    auto current_user_membership = public_channel.join(join_lambda);



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;


    
    //THESE FUNCTIONS DON'T REQUIRE PERMISSIONS
    chat_access_manager.set_auth_token(get_empty_token(token_access_manager));
    std::cout << "Get Channel Suggestions" << std::endl;
    chat.get_channel_suggestions("cpp");
    std::cout << "Get User Suggestions" << std::endl;
    chat.get_user_suggestions("cpp");
    std::cout << "Where Present" << std::endl;
    chat.where_present(user_id);
    std::cout << "Is present" << std::endl;
    chat.is_present(user_id, public_channel_id);



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;


    //CHANNEL LEAVE
    std::cout << "Channel.Leave" << std::endl;

    chat_access_manager.set_auth_token(get_leave_channel_token(token_access_manager, public_channel_id, user_id));
    public_channel.leave();



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //EMIT CHAT EVENT
    std::cout << "Emit chat event" << std::endl;

    chat_access_manager.set_auth_token(get_channel_write_token(token_access_manager, public_channel_id));
    chat.emit_chat_event(Pubnub::pubnub_chat_event_type::PCET_CUSTOM, public_channel_id, "{\"abc\": \"bcd\"}");




        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //WHO IS PRESENT
    std::cout << "Who is present" << std::endl;

    chat_access_manager.set_auth_token(get_channel_read_token(token_access_manager, public_channel_id));
    chat.who_is_present(public_channel_id);
    public_channel.who_is_present();

    //GET EVENTS HISTORY
    std::cout << "Get events history" << std::endl;
    //the same permissions as who is present
    chat.get_events_history(public_channel_id, Pubnub::get_now_timetoken(), "16417695239108460", 10);



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //LISTEN FOR EVENTS
    std::cout << "Listen for events" << std::endl;
    chat_access_manager.set_auth_token(get_channel_connect_token(token_access_manager, public_channel_id));
    auto listen_for_events_lambda = [](Pubnub::Event event){};
    auto stop_listening = chat.listen_for_events(public_channel_id, Pubnub::pubnub_chat_event_type::PCET_CUSTOM, listen_for_events_lambda);



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;
    stop_listening();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //CHANNEL CONNECT

    std::cout << "Channel Connect" << std::endl;
    chat_access_manager.set_auth_token(get_channel_connect_token(token_access_manager, group_channel_id));
    auto connect_lambda = [](Pubnub::Message message){};
    group_channel.created_channel.connect(connect_lambda);



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //CHANNEL DISCONNECT

    std::cout << "Channel Disconnect" << std::endl;
    //chat_access_manager.set_auth_token(get_empty_token(token_access_manager));
    group_channel.created_channel.disconnect();


        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //CHANNEL SEND TEXT

    std::cout << "Send text" << std::endl;

    Pubnub::String mentioned_user_id = "cpp_chat_test_user_to_mention";
    Pubnub::String time_before_send_text = Pubnub::get_now_timetoken();

    chat_access_manager.set_auth_token(get_send_text_token(token_access_manager, public_channel_id, mentioned_user_id));

    Pubnub::SendTextParams send_params;
    std::map<int, Pubnub::MentionedUser> mentioned_users;
    Pubnub::MentionedUser mentioned_user;
    mentioned_user.id = mentioned_user_id;
    mentioned_user.name = "cpp_chat_test_user_to_mention_name";
    mentioned_users[2] = mentioned_user;
    Pubnub::Map<int, Pubnub::MentionedUser> pn_mentioned_user(mentioned_users);
    send_params.mentioned_users = pn_mentioned_user;
    public_channel.send_text("message from test", send_params);



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //CHANNEL GET HISTORY
    std::cout << "Get history" << std::endl;
    chat_access_manager.set_auth_token(get_channel_read_token(token_access_manager, public_channel_id));

    auto history_messages = public_channel.get_history(Pubnub::get_now_timetoken(), time_before_send_text, 1);



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //CHANNEL PIN/UNPIN MESSAGE
    std::cout << "Pin message" << std::endl;

    chat_access_manager.set_auth_token(get_update_channel_token(token_access_manager, public_channel_id));
    chat.pin_message_to_channel(history_messages[0], public_channel);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));;

    std::cout << "Channel get pinned message" << std::endl;

    chat_access_manager.set_auth_token(get_get_channel_token(token_access_manager, public_channel_id));
    public_channel = chat.get_channel(public_channel_id);

    chat_access_manager.set_auth_token(get_read_channel_token(token_access_manager, public_channel_id));
    public_channel.get_pinned_message();

        std::this_thread::sleep_for(std::chrono::milliseconds(500));;

    std::cout << "Unpin message" << std::endl;

    chat_access_manager.set_auth_token(get_update_channel_token(token_access_manager, public_channel_id));
    chat.unpin_message_from_channel(public_channel);


    
        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //FORWARD MESSAGE
    std::cout << "Forward message" << std::endl;

    chat_access_manager.set_auth_token(get_channel_write_token(token_access_manager, group_channel_id));
    chat.forward_message(history_messages[0], group_channel.created_channel);



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //GET MEMBERS
    std::cout << "Channel Get Members" << std::endl;

    chat_access_manager.set_auth_token(get_get_channel_token(token_access_manager, public_channel_id));
    public_channel.get_members();



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //RESTRICTIONS
    std::cout << "Set restrictions" << std::endl;

    chat_access_manager.set_auth_token(get_set_restrictions_token(token_access_manager, public_channel_id, user_id));

    Pubnub::Restriction ban_restriction;
    ban_restriction.ban = true;
    chat.set_restrictions(user_id, public_channel_id, ban_restriction);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));;

    std::cout << "Get user restrictions" << std::endl;

    chat_access_manager.set_auth_token(get_get_user_token(token_access_manager, user_id));
    public_channel.get_user_restrictions(current_user);
    
    std::cout << "Get users restrictions" << std::endl;

    chat_access_manager.set_auth_token(get_get_channel_token(token_access_manager, Pubnub::String("PUBNUB_INTERNAL_MODERATION_") + public_channel_id));
    public_channel.get_users_restrictions();

    std::cout << "Lift restrictions" << std::endl;

    chat_access_manager.set_auth_token(get_set_restrictions_token(token_access_manager, public_channel_id, user_id));
    Pubnub::Restriction lift_restriction;
    lift_restriction.reason = "lift";
    chat.set_restrictions(user_id, public_channel_id, lift_restriction);



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;


    
    //INVITE
    std::cout << "Invite not member" << std::endl;

    chat_access_manager.set_auth_token(get_invite_token(token_access_manager, group_channel_id, another_user_id));
    group_channel.created_channel.invite(another_user);

    std::cout << "Invite member" << std::endl;
    chat_access_manager.set_auth_token(get_invite_token(token_access_manager, group_channel_id, invitee_user_id));
    auto invitee_membership = group_channel.created_channel.invite(invitee_user);

    std::cout << "Invite multiple" << std::endl;

    chat_access_manager.set_auth_token(get_invite_multiple_token(token_access_manager, group_channel_id, std::vector<Pubnub::String>{user_id, another_user_id}));
    group_channel.created_channel.invite_multiple(std::vector<Pubnub::User>{current_user, another_user});



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;


    
    //GET UNREAD MESSAGES COUNT
    std::cout << "Get Unread Messages Count" << std::endl;


    chat_access_manager.set_auth_token(get_unread_messages_counts_token(token_access_manager, user_id, std::vector<Pubnub::String>{group_channel_id, direct_channel_id, public_channel_id}));
    chat.get_unread_messages_counts();



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;


    
    //MARK ALL MESSAGES AS READ
    std::cout << "Mark All Messages as Read" << std::endl;

    chat_access_manager.set_auth_token(get_mark_all_messages_as_read_token(token_access_manager, user_id, std::vector<Pubnub::String>{group_channel_id, direct_channel_id, public_channel_id}));
    chat.mark_all_messages_as_read();



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;


    
    //CREATE THREAD CHANNEL
    std::cout << "Create thread channel" << std::endl;
    auto thread_channel_id = "PUBNUB_INTERNAL_THREAD_" + history_messages[0].message_data().channel_id + "_" + history_messages[0].timetoken();
    
    chat_access_manager.set_auth_token(get_get_channel_token(token_access_manager, thread_channel_id));
    auto created_thread = chat.create_thread_channel(history_messages[0]);


        std::this_thread::sleep_for(std::chrono::milliseconds(500));;


    
    //THREAD CHANNEL: SEND MESSAGE
    std::cout << "Thread channel send message" << std::endl;

    chat_access_manager.set_auth_token(get_thread_send_text_token(token_access_manager, thread_channel_id, created_thread.parent_channel_id(), mentioned_user_id));
    created_thread.send_text("message from thread", send_params);



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;


    //GET THREAD CHANNEL
    std::cout << "Get thread channel" << std::endl;

    chat_access_manager.set_auth_token(get_get_channel_token(token_access_manager, thread_channel_id));
    chat.get_thread_channel(history_messages[0]);



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //GET MESSAGE
    std::cout << "Channel Get Message" << std::endl;

    chat_access_manager.set_auth_token(get_channel_read_token(token_access_manager, public_channel_id));
    auto message = public_channel.get_message(history_messages[0].timetoken());



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //REMOVE THREAD CHANNEL
    std::cout << "Remove thread channel" << std::endl;

    chat_access_manager.set_auth_token(get_remove_thread_token(token_access_manager, thread_channel_id, created_thread.parent_channel_id()));
    chat.remove_thread_channel(message);



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //CHANNEL TYPING
    std::cout << "Channel: Start Typing" << std::endl;

    chat_access_manager.set_auth_token(get_channel_write_token(token_access_manager, group_channel_id));
    group_channel.created_channel.start_typing();

    std::cout << "Channel: Get Typing" << std::endl;

    chat_access_manager.set_auth_token(get_channel_connect_token(token_access_manager, group_channel_id));
    auto typing_lambda = [](Pubnub::Vector<Pubnub::String> typing){};
    auto get_typing_stop = group_channel.created_channel.get_typing(typing_lambda);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));;
    get_typing_stop();

    std::cout << "Channel: Stop Typing" << std::endl;
    chat_access_manager.set_auth_token(get_channel_write_token(token_access_manager, group_channel_id));
    group_channel.created_channel.stop_typing();


        std::this_thread::sleep_for(std::chrono::milliseconds(500));;





    //CHANNEL EMIT USER MENTION
    std::cout << "Channel: Emit user mention" << std::endl;

    chat_access_manager.set_auth_token(get_channel_write_token(token_access_manager, another_user_id));
    public_channel.emit_user_mention(another_user_id, Pubnub::get_now_timetoken(), "cpp_test user mention");



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;


    //CHANNEL STREAM UPDATES
    std::cout << "Channel: Stream updates" << std::endl;

    chat_access_manager.set_auth_token(get_channel_connect_token(token_access_manager, public_channel_id));
    auto stream_updates_lambda = [](Pubnub::Channel channel){};
    public_channel.stream_updates(stream_updates_lambda);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));;

    std::cout << "Channel: Stream updates on" << std::endl;
    auto stream_updates_on_lambda = [](Pubnub::Vector<Pubnub::Channel> channels){};
    public_channel.stream_updates_on(Pubnub::Vector<Pubnub::Channel>{public_channel}, stream_updates_on_lambda);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));;


    std::cout << "Channel: Stream presence" << std::endl;
    chat_access_manager.set_auth_token(get_channel_connect_token(token_access_manager, public_channel_id));
    auto stream_presence_lambda = [](Pubnub::Vector<Pubnub::String> presence){};
    public_channel.stream_presence(stream_presence_lambda);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));;

    std::cout << "Channel: Stream read receipts" << std::endl;
    chat_access_manager.set_auth_token(get_channel_stream_receipts_token(token_access_manager, public_channel_id));
    auto stream_receipts_lambda = [](Pubnub::Map<Pubnub::String, Pubnub::Vector<Pubnub::String>, Pubnub::StringComparer> receipts){};
    public_channel.stream_read_receipts(stream_receipts_lambda);


        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //CHANNEL GET MESSAGE REPORTS HISTORY
    std::cout << "Channel: get message reports history" << std::endl;

    chat_access_manager.set_auth_token(get_channel_read_token(token_access_manager, "PUBNUB_INTERNAL_MODERATION_" + public_channel_id));
    public_channel.get_messsage_reports_history(Pubnub::get_now_timetoken(), "00000000000000000");

        std::this_thread::sleep_for(std::chrono::milliseconds(500));;

    std::cout << "Channel: Stream Message Reports" << std::endl;

    chat_access_manager.set_auth_token(get_channel_connect_token(token_access_manager, "PUBNUB_INTERNAL_MODERATION_" + public_channel_id));
    auto stream_reports_lambda = [](Pubnub::Event event){};
    public_channel.stream_message_reports(stream_reports_lambda);



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;






    //UPDATE MEMBERSHIP
    std::cout << "Update Membership" << std::endl;

    chat_access_manager.set_auth_token(get_membership_update_token(token_access_manager, group_channel_id, invitee_user_id));
    invitee_membership.update("{\"abc\": \"dca\"}");


    std::this_thread::sleep_for(std::chrono::milliseconds(500));;


    //SET LAST READ MESSAGE TIMETOKEN
    std::cout << "Set last read message timetoken" << std::endl;

    chat_access_manager.set_auth_token(get_membership_set_last_read_message_token(token_access_manager, group_channel_id, invitee_user_id));
    invitee_membership.set_last_read_message_timetoken(Pubnub::get_now_timetoken());


    std::this_thread::sleep_for(std::chrono::milliseconds(500));


    //GET UNREAD MESSAGE COUNTS
    std::cout << "Get unread message counts" << std::endl;

    chat_access_manager.set_auth_token(get_channel_read_token(token_access_manager, group_channel_id));
    invitee_membership.get_unread_messages_count();


    std::this_thread::sleep_for(std::chrono::milliseconds(500));;


    //MEMBERSHIP STREAM UPDATES
    std::cout << "Membership: Stream Updates" << std::endl;

    chat_access_manager.set_auth_token(get_channel_connect_token(token_access_manager, group_channel_id));
    auto membership_stream_lambda = [](Pubnub::Membership membership){};
    auto membership_updates_stop_streaming = invitee_membership.stream_updates(membership_stream_lambda);


    std::this_thread::sleep_for(std::chrono::milliseconds(500));


    //MEMBERSHIP STREAM UPDATES ON
    std::cout << "Membership: Stream Updates On" << std::endl;

    auto membership_stream_on_lambda = [](Pubnub::Vector<Pubnub::Membership> memberships){};
    auto membership_updates_on_stop_streaming = invitee_membership.stream_updates_on(Pubnub::Vector<Pubnub::Membership>{invitee_membership}, membership_stream_on_lambda);


    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    membership_updates_stop_streaming.close();
    membership_updates_on_stop_streaming.close();


    std::this_thread::sleep_for(std::chrono::milliseconds(500));



    auto message_to_test = history_messages[0];

    //EDIT MESSAGE
    std::cout << "Edit Message" << std::endl;

    chat_access_manager.set_auth_token(get_channel_write_token(token_access_manager, message_to_test.message_data().channel_id));
    message_to_test = message_to_test.edit_text("new message text" + Pubnub::get_now_timetoken());

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //DELETE MESSAGE
    std::cout << "Delete Message" << std::endl;

    message_to_test = message_to_test.delete_message();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //RESTORE MESSAGE
    std::cout << "Restore Message" << std::endl;

    chat_access_manager.set_auth_token(get_delete_channel_token(token_access_manager, message_to_test.message_data().channel_id));
    message_to_test.restore();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //PIN MESSAGE
    std::cout << "Pin Message" << std::endl;

    chat_access_manager.set_auth_token(get_message_pin_token(token_access_manager, message_to_test.message_data().channel_id));
    message_to_test.pin();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //UNPIN MESSAGE
    std::cout << "Unpin Message" << std::endl;

    chat_access_manager.set_auth_token(get_message_unpin_token(token_access_manager, message_to_test.message_data().channel_id));
    message_to_test.unpin();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //TOGGLE REACTION
    std::cout << "Toggle reaction" << std::endl;

    chat_access_manager.set_auth_token(get_message_toggle_reaction_token(token_access_manager, message_to_test.message_data().channel_id));
    message_to_test = message_to_test.toggle_reaction("happy");

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    message_to_test = message_to_test.toggle_reaction("happy");


    std::this_thread::sleep_for(std::chrono::milliseconds(500));


    //REPORT MESSAGE
    std::cout << "Report message" << std::endl;

    chat_access_manager.set_auth_token(get_channel_write_token(token_access_manager, "PUBNUB_INTERNAL_MODERATION_" + message_to_test.message_data().channel_id));
    message_to_test.report("for cpp test");
    

    std::this_thread::sleep_for(std::chrono::milliseconds(500));


    //MESSAGE STREAM UPDATES
    std::cout << "Message: Stream Updates" << std::endl;

    chat_access_manager.set_auth_token(get_channel_connect_token(token_access_manager, message_to_test.message_data().channel_id));
    auto message_stream_lambda = [](Pubnub::Message message){};
    auto message_updates_stop_streaming = message_to_test.stream_updates(message_stream_lambda);


    std::this_thread::sleep_for(std::chrono::milliseconds(500));


    //MESSAGE STREAM UPDATES ON
    std::cout << "Message: Stream Updates On" << std::endl;

    auto message_stream_on_lambda = [](Pubnub::Vector<Pubnub::Message> messages){};
    auto message_updates_on_stop_streaming = message_to_test.stream_updates_on(Pubnub::Vector<Pubnub::Message>{message_to_test}, message_stream_on_lambda);


    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    message_updates_stop_streaming.close();
    message_updates_on_stop_streaming.close();

*/

    //MESSAGE DRAFT
    std::cout << "Create message draft" << std::endl;

    chat_access_manager.set_auth_token(get_empty_token(token_access_manager));
    Pubnub::MessageDraftConfig draft_config;
    draft_config.is_typing_indicator_triggered = true;
    auto message_draft = public_channel.create_message_draft(draft_config);

    std::cout << "Create message draft listeners" << std::endl;

    auto add_change_lambda = [](Pubnub::Vector<Pubnub::MessageElement> elements)
    {
        std::cout << "Draft Change, no suggestions " << std::endl;
        for(auto element : elements)
        {
            std::cout << "Element: " << element.text << std::endl;
        }

    };

    auto add_change_suggestions_lambda = [&message_draft](Pubnub::Vector<Pubnub::MessageElement> elements ,Pubnub::Vector<Pubnub::SuggestedMention> suggested_mentions)
    {
        std::cout << "Draft Change, with suggestions " << std::endl;
        for(auto element : elements)
        {
            std::cout << "Element: " << element.text << std::endl;
        }

        if(suggested_mentions.size() > 0)
        {
            message_draft.insert_suggested_mention(suggested_mentions[0], "inserted mention");
        }

    };
    message_draft.add_change_listener(add_change_lambda);
    message_draft.add_change_listener(add_change_suggestions_lambda);
    

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //MESSAGE DRAFT INSERT TEXT
    std::cout << "Message draft insert text" << std::endl;

    message_draft.insert_text(0, "cpp_test draft text");
    message_draft.insert_text(5, "@cpp_test");
    message_draft.insert_text(15, "#cpp_test");

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //MESSAGE DRAFT REMOVE TEXT
    std::cout << "Message draft remove text" << std::endl;

    message_draft.remove_text(2, 2);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //MESSAGE DRAFT ADD MENTION
    std::cout << "Message draft add mention" << std::endl;

    Pubnub::MentionTarget mention_target;
    mention_target.channel(group_channel_id);
    message_draft.add_mention(9, 10, mention_target);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //MESSAGE DRAFT UPDATE
    std::cout << "Message draft update" << std::endl;

    message_draft.update("cpp_test draft updated");

    //MESSAGE DRAFT INSERT SUGGESTED MENTION
    std::cout << "Message draft insert suggested mention" << std::endl;

    Pubnub::SuggestedMention suggested_mention;
    suggested_mention.target = mention_target;
    suggested_mention.offset = 0;
    suggested_mention.replace_from = "cpp_test";
    suggested_mention.replace_to = "inserted_mention";

    message_draft.insert_suggested_mention(suggested_mention, "inserted_mention");

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //MESSAGE DRAFT REMOVE MENTION
    std::cout << "Message draft remove mention" << std::endl;
    message_draft.remove_mention(9);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //MESSAGE DRAFT SEND
    std::cout << "Message draft send" << std::endl;

    chat_access_manager.set_auth_token(get_send_text_token(token_access_manager, public_channel_id, user_id));
    message_draft.send();



    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    //DELETE USER
    std::cout << "Delete User" << std::endl;

    chat_access_manager.set_auth_token(get_delete_user_token(token_access_manager, create_user_id));
    chat.delete_user(create_user_id);

    chat_access_manager.set_auth_token(get_delete_user_token(token_access_manager, another_user_id));
    chat.delete_user(another_user_id);



        std::this_thread::sleep_for(std::chrono::milliseconds(500));;



    //DELETE CHANNEL
    std::cout << "Delete Channel" << std::endl;

    chat_access_manager.set_auth_token(get_delete_channel_token(token_access_manager, direct_channel_id));
    chat.delete_channel(direct_channel_id);
    


        std::this_thread::sleep_for(std::chrono::milliseconds(500));;




    std::cout << "End of Tests" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "End of main" << std::endl;

    return 0;
}


Pubnub::String get_init_chat_token(Pubnub::AccessManager& token_access_manager, Pubnub::String user_id)
{
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::UserPermissions user_permissions;
    user_permissions.get = true;
    user_permissions.update = true;
    permission_object.users.push_back(user_id);
    permission_object.user_permissions.push_back(user_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_empty_token(Pubnub::AccessManager& token_access_manager)
{
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::UserPermissions user_permissions;
    user_permissions.update = true;
    permission_object.users.push_back("aaaaaaaaaaaaaa");
    permission_object.user_permissions.push_back(user_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_create_pub_conversation_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id)
{
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.update = true;
    permission_object.channels.push_back(channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_create_user_token(Pubnub::AccessManager& token_access_manager, Pubnub::String user_id)
{
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::UserPermissions user_permissions;
    user_permissions.get = true;
    user_permissions.update = true;
    permission_object.users.push_back(user_id);
    permission_object.user_permissions.push_back(user_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_get_user_token(Pubnub::AccessManager& token_access_manager, Pubnub::String user_id)
{
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::UserPermissions user_permissions;
    user_permissions.get = true;
    permission_object.users.push_back(user_id);
    permission_object.user_permissions.push_back(user_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_get_users_token(Pubnub::AccessManager& token_access_manager, std::vector<Pubnub::String> user_ids)
{
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::UserPermissions user_permissions;
    user_permissions.get = true;
    for(auto user_id : user_ids) {permission_object.users.push_back(user_id);}
    permission_object.user_permissions.push_back(user_permissions);

    return token_access_manager.grant_token(permission_object);
}


Pubnub::String get_create_dir_conversation_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String current_user_id, Pubnub::String invitee_id)
{
    //ChannelID: UPDATE, JOIN, WRITE, GET
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.update = true;
    channel_permissions.join = true;
    channel_permissions.write = true;
    channel_permissions.get = true;
    permission_object.channels.push_back(channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    //Current User ID: UPDATE, GET
    Pubnub::UserPermissions invitee_permissions;
    invitee_permissions.update = true;
    invitee_permissions.get = true;
    permission_object.users.push_back(current_user_id);

    //Invitee ID: UPDATE, GET
    permission_object.users.push_back(invitee_id);
    permission_object.user_permissions.push_back(invitee_permissions);

    //Channel(Invitee_IDs): WRITE
    Pubnub::ChannelPermissions channel_invitee_permissions;
    channel_invitee_permissions.write = true;
    permission_object.channels.push_back(invitee_id);
    permission_object.channel_permissions.push_back(channel_invitee_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_create_group_conversation_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String current_user_id, std::vector<Pubnub::String> invitee_ids)
{
    //ChannelID: UPDATE, JOIN, MANAGE, WRITE
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.update = true;
    channel_permissions.manage = true;
    channel_permissions.join = true;
    channel_permissions.write = true;
    permission_object.channels.push_back(channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    //Current User ID: UPDATE, GET
    Pubnub::UserPermissions invitee_permissions;
    invitee_permissions.update = true;
    invitee_permissions.get = true;
    permission_object.users.push_back(current_user_id);

    //Invitees IDs: UPDATE, GET
    for(auto invitee_id : invitee_ids) {permission_object.users.push_back(invitee_id);}

    permission_object.user_permissions.push_back(invitee_permissions);

    //Channel(Invitees_IDs): WRITE
    Pubnub::ChannelPermissions channel_invitee_permissions;
    channel_invitee_permissions.write = true;
    for(auto invitee_id : invitee_ids) {
        permission_object.channels.push_back(invitee_id);
        permission_object.channel_permissions.push_back(channel_invitee_permissions);
    }

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_get_channel_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id)
{
    //Channel ID: GET
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.get = true;
    permission_object.channels.push_back(channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_read_channel_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id)
{
    //Channel ID: READ
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.read = true;
    permission_object.channels.push_back(channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_get_channels_token(Pubnub::AccessManager& token_access_manager, std::vector<Pubnub::String> channel_ids)
{
    //Channel IDs: GET
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.get = true;
    for(auto channel_id : channel_ids) {permission_object.channels.push_back(channel_id);}
    permission_object.channel_permissions.push_back(channel_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_update_channel_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id)
{
    //Channel ID: UPDATE
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.update = true;
    permission_object.channels.push_back(channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_delete_channel_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id)
{
    //Channel ID: DELETE
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.del = true;
    permission_object.channels.push_back(channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_update_user_token(Pubnub::AccessManager& token_access_manager, Pubnub::String user_id)
{
    //User ID: UPDATE
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::UserPermissions user_permissions;
    user_permissions.update = true;
    permission_object.users.push_back(user_id);
    permission_object.user_permissions.push_back(user_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_delete_user_token(Pubnub::AccessManager& token_access_manager, Pubnub::String user_id)
{
    //User ID: DELETE
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::UserPermissions user_permissions;
    user_permissions.del = true;
    permission_object.users.push_back(user_id);
    permission_object.user_permissions.push_back(user_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_join_channel_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String user_id)
{
    //Channel ID: READ, JOIN, WRITE
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.read = true;
    channel_permissions.join = true;
    channel_permissions.write = true;
    permission_object.channels.push_back(channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    //Channel ID + -pnpres: READ
    Pubnub::ChannelPermissions pres_channel_permissions;
    pres_channel_permissions.read = true;
    permission_object.channels.push_back(channel_id + Pubnub::String("-pnpres"));
    permission_object.channel_permissions.push_back(pres_channel_permissions);

    //User ID: UPDATE, GET
    Pubnub::UserPermissions user_permissions;
    user_permissions.update = true;
    user_permissions.get = true;
    permission_object.users.push_back(user_id);
    permission_object.user_permissions.push_back(user_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_leave_channel_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String user_id)
{
    //Channel ID: JOIN, READ
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.join = true;
    channel_permissions.read = true;
    permission_object.channels.push_back(channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    //Channel ID + -pnpres: READ
    Pubnub::ChannelPermissions pres_channel_permissions;
    pres_channel_permissions.read = true;
    permission_object.channels.push_back(channel_id + Pubnub::String("-pnpres"));
    permission_object.channel_permissions.push_back(pres_channel_permissions);

    //User ID: UPDATE
    Pubnub::UserPermissions user_permissions;
    user_permissions.update = true;
    permission_object.users.push_back(user_id);
    permission_object.user_permissions.push_back(user_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_channel_read_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id)
{
    //Channel ID: READ
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.read = true;
    permission_object.channels.push_back(channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_channel_write_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id)
{
    //Channel ID: WRITE
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.write = true;
    permission_object.channels.push_back(channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_channel_connect_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id)
{
    //Channel ID: READ
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.read = true;
    permission_object.channels.push_back(channel_id);
    //Channel ID + -pnpres: READ
    permission_object.channels.push_back(channel_id + Pubnub::String("-pnpres"));
    permission_object.channel_permissions.push_back(channel_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_send_text_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String mentioned_user_id)
{
    //Channel ID: WRITE
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.write = true;
    permission_object.channels.push_back(channel_id);
    //Channel mentioned_user_id: WRITE
    permission_object.channels.push_back(mentioned_user_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_set_restrictions_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String user_id)
{
    //Channel PUBNUB_INTERNAL_MODERATION_{CHANNEL_ID}: MANAGE, UPDATE
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.manage = true;
    channel_permissions.update = true;
    permission_object.channels.push_back(Pubnub::String("PUBNUB_INTERNAL_MODERATION_") + channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    //Channel user_id: WRITE
    Pubnub::ChannelPermissions user_channel_permissions;
    user_channel_permissions.write = true;
    permission_object.channels.push_back(Pubnub::String("PUBNUB_INTERNAL_MODERATION.") + user_id);
    permission_object.channel_permissions.push_back(user_channel_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_invite_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String invitee_id)
{
    //ChannelID: GET, JOIN, WRITE
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.get = true;
    channel_permissions.join = true;
    channel_permissions.write = true;
    permission_object.channels.push_back(channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    //User invitee_id: UPDATE
    Pubnub::UserPermissions invitee_permissions;
    invitee_permissions.update = true;
    invitee_permissions.get = true;
    permission_object.users.push_back(invitee_id);

    permission_object.user_permissions.push_back(invitee_permissions);

    //Channel(Invitee_IDs): WRITE
    Pubnub::ChannelPermissions channel_invitee_permissions;
    channel_invitee_permissions.write = true;
    permission_object.channels.push_back(invitee_id);
    permission_object.channel_permissions.push_back(channel_invitee_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_invite_multiple_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, std::vector<Pubnub::String> invitee_ids)
{
    //ChannelID: JOIN, WRITE, MANAGE
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.manage = true;
    channel_permissions.join = true;
    channel_permissions.write = true;
    permission_object.channels.push_back(channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    //Invitees IDs: UPDATE, GET
    Pubnub::UserPermissions invitee_permissions;
    invitee_permissions.update = true;
    //invitee_permissions.get = true;

    for(auto invitee_id : invitee_ids) {permission_object.users.push_back(invitee_id);}

    permission_object.user_permissions.push_back(invitee_permissions);

    //Channel(Invitees_IDs): WRITE
    Pubnub::ChannelPermissions channel_invitee_permissions;
    channel_invitee_permissions.write = true;
    for(auto invitee_id : invitee_ids) {
        permission_object.channels.push_back(invitee_id);
        permission_object.channel_permissions.push_back(channel_invitee_permissions);
        }

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_unread_messages_counts_token(Pubnub::AccessManager& token_access_manager, Pubnub::String current_user_id, std::vector<Pubnub::String> channels)
{
    //Channels: READ
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.read = true;
    for(auto channel : channels) {permission_object.channels.push_back(channel);}
    permission_object.channel_permissions.push_back(channel_permissions);
   
    //Current user: GET
    Pubnub::UserPermissions user_permissions;
    user_permissions.get = true;
    permission_object.users.push_back(current_user_id);
    permission_object.user_permissions.push_back(user_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_mark_all_messages_as_read_token(Pubnub::AccessManager& token_access_manager, Pubnub::String current_user_id, std::vector<Pubnub::String> channels)
{
    //Channels: JOIN, WRITE
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.join = true;
    channel_permissions.write = true;
    for(auto channel : channels) {permission_object.channels.push_back(channel);}
    permission_object.channel_permissions.push_back(channel_permissions);
   
    //Current user: GET, UPDATE
    Pubnub::UserPermissions user_permissions;
    user_permissions.get = true;
    user_permissions.update = true;
    permission_object.users.push_back(current_user_id);
    permission_object.user_permissions.push_back(user_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_thread_send_text_token(Pubnub::AccessManager& token_access_manager, Pubnub::String thread_channel_id, Pubnub::String parent_channel_id, Pubnub::String mentioned_user_id)
{
    //Parent Channel ID: WRITE
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.write = true;
    permission_object.channels.push_back(parent_channel_id);

    //Channel mentioned_user_id: WRITE
    permission_object.channels.push_back(mentioned_user_id);
    permission_object.channel_permissions.push_back(channel_permissions);
    permission_object.channel_permissions.push_back(channel_permissions);

    //Thread Channel ID: WRITE, UPDATE
    Pubnub::ChannelPermissions thread_channel_permissions;
    thread_channel_permissions.write = true;
    thread_channel_permissions.update = true;
    permission_object.channel_permissions.push_back(thread_channel_permissions);
    permission_object.channels.push_back(thread_channel_id);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_remove_thread_token(Pubnub::AccessManager& token_access_manager, Pubnub::String thread_channel_id, Pubnub::String parent_channel_id)
{
    //Parent Channel ID: DELETE
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.del = true;
    permission_object.channels.push_back(parent_channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    //Thread Channel ID: GET, DELETE
    Pubnub::ChannelPermissions thread_channel_permissions;
    thread_channel_permissions.get = true;
    thread_channel_permissions.del = true;
    permission_object.channel_permissions.push_back(thread_channel_permissions);
    permission_object.channels.push_back(thread_channel_id);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_channel_stream_receipts_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id)
{
    //Channel ID: READ, GET
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.read = true;
    channel_permissions.get = true;
    permission_object.channels.push_back(channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    //Channel ID + -pnpres: READ
    Pubnub::ChannelPermissions pres_channel_permissions;
    pres_channel_permissions.read = true;
    permission_object.channels.push_back(channel_id + Pubnub::String("-pnpres"));
    permission_object.channel_permissions.push_back(pres_channel_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_membership_update_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String user_id)
{
    //ChannelID: JOIN
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.join = true;
    permission_object.channels.push_back(channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    //User user_id: UPDATE
    Pubnub::UserPermissions user_permissions;
    user_permissions.update = true;
    permission_object.users.push_back(user_id);
    permission_object.user_permissions.push_back(user_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_membership_set_last_read_message_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String user_id)
{
    //ChannelID: JOIN, WRITE
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.write = true;
    channel_permissions.join = true;
    permission_object.channels.push_back(channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    //User user_id: UPDATE
    Pubnub::UserPermissions user_permissions;
    user_permissions.update = true;
    permission_object.users.push_back(user_id);
    permission_object.user_permissions.push_back(user_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_message_pin_token(Pubnub::AccessManager& token_access_manager, Pubnub::String message_channel_id)
{
    //ChannelID: GET, UPDATE
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.get = true;
    channel_permissions.update = true;
    permission_object.channels.push_back(message_channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_message_unpin_token(Pubnub::AccessManager& token_access_manager, Pubnub::String message_channel_id)
{
    //ChannelID: GET, UPDATE, READ
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.get = true;
    channel_permissions.update = true;
    channel_permissions.read = true;
    permission_object.channels.push_back(message_channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_message_toggle_reaction_token(Pubnub::AccessManager& token_access_manager, Pubnub::String message_channel_id)
{
    //ChannelID: DELETE, WRITE
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.del = true;
    channel_permissions.write = true;
    permission_object.channels.push_back(message_channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    return token_access_manager.grant_token(permission_object);
}