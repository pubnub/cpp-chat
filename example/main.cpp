#include "pubnub_chat/message.hpp"
#include "pubnub_chat/chat.hpp"
#include "pubnub_chat/chat_helpers.hpp"
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
Pubnub::String get_get_channels_token(Pubnub::AccessManager& token_access_manager, std::vector<Pubnub::String> channel_ids);
Pubnub::String get_update_channel_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id);
Pubnub::String get_delete_channel_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id);
Pubnub::String get_update_user_token(Pubnub::AccessManager& token_access_manager, Pubnub::String user_id);
Pubnub::String get_delete_user_token(Pubnub::AccessManager& token_access_manager, Pubnub::String user_id);
Pubnub::String get_join_channel_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String user_id);
Pubnub::String get_leave_channel_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String user_id);
Pubnub::String get_channel_read_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id);
Pubnub::String get_channel_write_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id);
Pubnub::String get_listen_for_events_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id);
Pubnub::String get_send_text_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String mentioned_user_id);
Pubnub::String get_set_restrictions_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String user_id);
Pubnub::String get_invite_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String invitee_id);
Pubnub::String get_invite_multiple_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, std::vector<Pubnub::String> invitee_ids);
Pubnub::String get_unread_messages_counts_token(Pubnub::AccessManager& token_access_manager, Pubnub::String current_user_id, std::vector<Pubnub::String> channels);


int main() {


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



    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto chat_access_manager = chat.access_manager();



    //CURRENT USER
    std::cout << "Current User Test" << std::endl;

    chat_access_manager.set_auth_token(get_get_user_token(token_access_manager, user_id));
    auto current_user = chat.current_user();



    std::this_thread::sleep_for(std::chrono::seconds(1));



    //CREATE USER
    std::cout << "Create User Test" << std::endl;

    Pubnub::String create_user_id = Pubnub::String("cpp_chat_test_created_user");

    chat_access_manager.set_auth_token(get_create_user_token(token_access_manager, create_user_id));
    auto created_user = chat.create_user(create_user_id, Pubnub::ChatUserData());

    Pubnub::String another_user_id = Pubnub::String("cpp_chat_test_another_user");

    chat_access_manager.set_auth_token(get_create_user_token(token_access_manager, another_user_id));
    auto another_user = chat.create_user(another_user_id, Pubnub::ChatUserData());


    std::this_thread::sleep_for(std::chrono::seconds(1));



    //GET USER 
    std::cout << "Get User Test" << std::endl;

    Pubnub::String invitee_user_id = "cpp_chat_test_invited_user";

    chat_access_manager.set_auth_token(get_get_user_token(token_access_manager, invitee_user_id));
    auto invitee_user = chat.get_user(invitee_user_id);



    std::this_thread::sleep_for(std::chrono::seconds(1));



    //GET USERS
    std::cout << "Get Users Test" << std::endl;

    //This shouldn't require any permissions
    auto users_from_get = chat.get_users("id LIKE \"cpp_chat_test\"", "", 5);



    std::this_thread::sleep_for(std::chrono::seconds(1));



    //CREATE PUBLIC CONVERSATION
    std::cout << "Create Public Conversation Test" << std::endl;

    Pubnub::String public_channel_id = "cpp_chat_test_public_channel";

    chat_access_manager.set_auth_token(get_create_pub_conversation_token(token_access_manager, public_channel_id));
    auto public_channel = chat.create_public_conversation(public_channel_id, Pubnub::ChatChannelData());



    std::this_thread::sleep_for(std::chrono::seconds(1));



    //CREATE DIRECT CONVERSATION
    std::cout << "Create Direct Conversation Test" << std::endl;

    Pubnub::String direct_channel_id = "cpp_chat_test_direct_channel";

    chat_access_manager.set_auth_token(get_create_dir_conversation_token(token_access_manager, direct_channel_id, current_user.user_id(), invitee_user_id));
    auto direct_channel = chat.create_direct_conversation(invitee_user, direct_channel_id, Pubnub::ChatChannelData());



    std::this_thread::sleep_for(std::chrono::seconds(1));



    //CREATE GROUP CONVERSATION
    std::cout << "Create Group Conversation Test" << std::endl;

    Pubnub::String group_channel_id = "cpp_chat_test_group_channel";

    chat_access_manager.set_auth_token(get_create_group_conversation_token(token_access_manager, group_channel_id, current_user.user_id(), std::vector<Pubnub::String>{invitee_user_id, create_user_id}));
    auto group_channel = chat.create_group_conversation(std::vector<Pubnub::User>{invitee_user, created_user}, group_channel_id, Pubnub::ChatChannelData());
    


    std::this_thread::sleep_for(std::chrono::seconds(1));



    //GET CHANNEL
    std::cout << "Get Channel Test" << std::endl;

    chat_access_manager.set_auth_token(get_get_channel_token(token_access_manager, public_channel_id));
    auto channel_from_get = chat.get_channel(public_channel_id);
        


    std::this_thread::sleep_for(std::chrono::seconds(1));



    //GET CHANNELS
    std::cout << "Get Channels" << std::endl;

    //This shouldn't require any permissions
    auto channels_from_get = chat.get_channels("id LIKE \"cpp_chat_test\"", "", 5);



    std::this_thread::sleep_for(std::chrono::seconds(1));



    //UPDATE CHANNEL
    std::cout << "Update Channel" << std::endl;

    chat_access_manager.set_auth_token(get_update_channel_token(token_access_manager, public_channel_id));
    auto new_channel_data = Pubnub::ChatChannelData();
    new_channel_data.description = Pubnub::get_now_timetoken();
    chat.update_channel(public_channel_id, new_channel_data);



    std::this_thread::sleep_for(std::chrono::seconds(1));



    //DELETE CHANNEL
    std::cout << "Delete Channel" << std::endl;

    chat_access_manager.set_auth_token(get_delete_channel_token(token_access_manager, direct_channel_id));
    chat.delete_channel(direct_channel_id);



    std::this_thread::sleep_for(std::chrono::seconds(1));



    //UPDATE USER
    std::cout << "Update User" << std::endl;

    chat_access_manager.set_auth_token(get_update_user_token(token_access_manager, invitee_user_id));
    auto new_user_data = Pubnub::ChatUserData();
    new_user_data.external_id = Pubnub::get_now_timetoken();
    chat.update_user(invitee_user_id, new_user_data);



    std::this_thread::sleep_for(std::chrono::seconds(1));



    //DELETE USER
    std::cout << "Delete User" << std::endl;

    chat_access_manager.set_auth_token(get_delete_user_token(token_access_manager, create_user_id));
    chat.delete_user(create_user_id);
    


    std::this_thread::sleep_for(std::chrono::seconds(1));

/*

    //CHANNEL JOIN
    std::cout << "Channel.Join" << std::endl;

    chat_access_manager.set_auth_token(get_join_channel_token(token_access_manager, public_channel_id, user_id));
    auto join_lambda = [](Pubnub::Message message){};
    public_channel.join(join_lambda);



    std::this_thread::sleep_for(std::chrono::seconds(1));


    
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



    std::this_thread::sleep_for(std::chrono::seconds(1));


    //CHANNEL LEAVE
    std::cout << "Channel.Leave" << std::endl;

    chat_access_manager.set_auth_token(get_leave_channel_token(token_access_manager, public_channel_id, user_id));
    public_channel.leave();



    std::this_thread::sleep_for(std::chrono::seconds(1));



    //EMIT CHAT EVENT
    std::cout << "Emit chat event" << std::endl;

    chat_access_manager.set_auth_token(get_channel_write_token(token_access_manager, public_channel_id));
    chat.emit_chat_event(Pubnub::pubnub_chat_event_type::PCET_CUSTOM, public_channel_id, "{\"abc\": \"bcd\"}");




    std::this_thread::sleep_for(std::chrono::seconds(1));



    //WHO IS PRESENT
    std::cout << "Who is present" << std::endl;

    chat_access_manager.set_auth_token(get_channel_read_token(token_access_manager, public_channel_id));
    chat.who_is_present(public_channel_id);
    public_channel.who_is_present();

    //GET EVENTS HISTORY
    std::cout << "Get events history" << std::endl;
    //the same permissions as who is present
    chat.get_events_history(public_channel_id, Pubnub::get_now_timetoken(), "16417695239108460", 10);



    std::this_thread::sleep_for(std::chrono::seconds(1));



    //LISTEN FOR EVENTS
    std::cout << "Listen for events" << std::endl;
    chat_access_manager.set_auth_token(get_listen_for_events_token(token_access_manager, public_channel_id));
    auto listen_for_events_lambda = [](Pubnub::Event event){};
    auto stop_listening = chat.listen_for_events(public_channel_id, Pubnub::pubnub_chat_event_type::PCET_CUSTOM, listen_for_events_lambda);



    std::this_thread::sleep_for(std::chrono::seconds(1));
    stop_listening();
    std::this_thread::sleep_for(std::chrono::seconds(1));



    //CHANNEL CONNECT

    std::cout << "Channel Connect" << std::endl;
    chat_access_manager.set_auth_token(get_listen_for_events_token(token_access_manager, group_channel_id));
    auto connect_lambda = [](Pubnub::Message message){};
    group_channel.created_channel.connect(connect_lambda);



    std::this_thread::sleep_for(std::chrono::seconds(1));



    //CHANNEL DISCONNECT

    std::cout << "Channel Disconnect" << std::endl;
    //chat_access_manager.set_auth_token(get_empty_token(token_access_manager));
    group_channel.created_channel.disconnect();


    std::this_thread::sleep_for(std::chrono::seconds(1));



    //CHANNEL SEND TEXT

    std::cout << "Send text" << std::endl;

    Pubnub::String mentioned_user_id = "cpp_chat_test_user_to_mention";

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



    std::this_thread::sleep_for(std::chrono::seconds(1));



    //CHANNEL GET HISTORY
    std::cout << "Get history" << std::endl;
    chat_access_manager.set_auth_token(get_channel_read_token(token_access_manager, public_channel_id));

    auto history_messages = public_channel.get_history(Pubnub::get_now_timetoken(), "16417695239108460");



    std::this_thread::sleep_for(std::chrono::seconds(1));



    //CHANNEL PIN/UNPIN MESSAGE
    std::cout << "Pin/unpin message" << std::endl;

    chat_access_manager.set_auth_token(get_update_channel_token(token_access_manager, public_channel_id));
    chat.pin_message_to_channel(history_messages[0], public_channel);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    chat.unpin_message_from_channel(public_channel);


    
    std::this_thread::sleep_for(std::chrono::seconds(1));



    //FORWARD MESSAGE
    std::cout << "Forward message" << std::endl;

    chat_access_manager.set_auth_token(get_channel_write_token(token_access_manager, group_channel_id));
    chat.forward_message(history_messages[0], group_channel.created_channel);



    std::this_thread::sleep_for(std::chrono::seconds(1));



    //GET MESSAGE
    std::cout << "Channel Get Message" << std::endl;

    chat_access_manager.set_auth_token(get_channel_read_token(token_access_manager, public_channel_id));
    public_channel.get_message(history_messages[0].timetoken());



    std::this_thread::sleep_for(std::chrono::seconds(1));



    //GET MEMBERS
    std::cout << "Channel Get Members" << std::endl;

    chat_access_manager.set_auth_token(get_get_channel_token(token_access_manager, public_channel_id));
    public_channel.get_members();



    std::this_thread::sleep_for(std::chrono::seconds(1));
*/


    //RESTRICTIONS
    std::cout << "Set restrictions" << std::endl;

    chat_access_manager.set_auth_token(get_set_restrictions_token(token_access_manager, public_channel_id, user_id));

    Pubnub::Restriction ban_restriction;
    ban_restriction.ban = true;
    chat.set_restrictions(user_id, public_channel_id, ban_restriction);

    std::this_thread::sleep_for(std::chrono::seconds(1));

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



    std::this_thread::sleep_for(std::chrono::seconds(1));


    
    //INVITE
    std::cout << "Invite not member" << std::endl;

    chat_access_manager.set_auth_token(get_invite_token(token_access_manager, group_channel_id, another_user_id));
    group_channel.created_channel.invite(another_user);

    std::cout << "Invite member" << std::endl;
    chat_access_manager.set_auth_token(get_invite_token(token_access_manager, group_channel_id, invitee_user_id));
    group_channel.created_channel.invite(invitee_user);

    std::cout << "Invite multiple" << std::endl;

    chat_access_manager.set_auth_token(get_invite_multiple_token(token_access_manager, group_channel_id, std::vector<Pubnub::String>{user_id, another_user_id}));
    group_channel.created_channel.invite_multiple(std::vector<Pubnub::User>{current_user, another_user});



    std::this_thread::sleep_for(std::chrono::seconds(1));


    
    //GET UNREAD MESSAGES COUNT
    std::cout << "Get Unread Messages Count" << std::endl;


    chat_access_manager.set_auth_token(get_unread_messages_counts_token(token_access_manager, user_id, std::vector<Pubnub::String>{group_channel_id}));
    chat.get_unread_messages_counts();



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

Pubnub::String get_listen_for_events_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id)
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