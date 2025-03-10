#include "pubnub_chat/message.hpp"
#include "pubnub_chat/chat.hpp"
#include "pubnub_chat/chat_helpers.hpp"
#include <chrono>
#include <thread>
#include <vector>

#define TOKEN_TTL 10
#define TOKEN_AUTH_USER_ID "pam_ccp_chat_user"

Pubnub::String get_init_chat_token(Pubnub::AccessManager& token_access_manager, Pubnub::String user_id);
Pubnub::String get_create_pub_conversation_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id);
Pubnub::String get_create_user_token(Pubnub::AccessManager& token_access_manager, Pubnub::String user_id);
Pubnub::String get_get_user_token(Pubnub::AccessManager& token_access_manager, Pubnub::String user_id);
Pubnub::String get_create_dir_conversation_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String current_user_id, Pubnub::String invitee_id);
Pubnub::String get_create_group_conversation_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String current_user_id, std::vector<Pubnub::String> invitee_ids);
Pubnub::String get_get_channel_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id);
Pubnub::String get_get_channels_token(Pubnub::AccessManager& token_access_manager, std::vector<Pubnub::String> channel_ids);


int main() {
    auto publish_key = "pub-c-7236aefc-bc22-43db-bdb3-1c96a57004ab";
    auto subscribe_key = "sub-c-7e0de38f-77e0-402b-91de-d26acd3be135";
    auto secret_key = "sec-c-ZDU1ZmZjNzItZTA1NS00MDU5LWE3NmItM2M3ZDNhOWQ2MDRm";

    //This token is needed to init chat
    auto initial_auth_token = "p0F2AkF0GmfOvB5DdHRsGajAQ3Jlc6VEY2hhbqBDZ3JwoENzcGOgQ3VzcqBEdXVpZKFxcGFtX2NjcF9jaGF0X3VzZXIYaENwYXSlRGNoYW6gQ2dycKBDc3BjoEN1c3KgRHV1aWSgRG1ldGGgQ3NpZ1ggUA6E8e1Jps1cPGkW6lpzOKYzc-bjUtlLDyeUdiyv_uw=";
    
    auto config = Pubnub::ChatConfig();
    config.auth_key = initial_auth_token;

    auto token_chat = Pubnub::Chat::init(publish_key, subscribe_key, TOKEN_AUTH_USER_ID, config);
    auto token_access_manager = token_chat.access_manager();
    token_access_manager.set_secret_key(secret_key);


    //INIT CHAT
    std::cout << "Init Test" << std::endl;

    auto user_id = "ccp_chat_user";

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

    Pubnub::String create_user_id = Pubnub::String("cpp_chat_test_created_user_") + Pubnub::get_now_timetoken();

    chat_access_manager.set_auth_token(get_create_user_token(token_access_manager, create_user_id));
    auto created_user = chat.create_user(create_user_id, Pubnub::ChatUserData());


    std::this_thread::sleep_for(std::chrono::seconds(1));



    //GET USER 
    std::cout << "Get User Test" << std::endl;

    Pubnub::String invitee_user_id = "cpp_chat_test_invited_user";

    chat_access_manager.set_auth_token(get_get_user_token(token_access_manager, invitee_user_id));
    auto invitee_user = chat.get_user(invitee_user_id);



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

    chat_access_manager.set_auth_token(get_get_channels_token(token_access_manager, std::vector<Pubnub::String>{public_channel_id, group_channel_id, direct_channel_id}));
    auto channels_from_get = chat.get_channels("id LIKE \"cpp_chat_test\"", "", 3);






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

Pubnub::String get_create_dir_conversation_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String current_user_id, Pubnub::String invitee_id)
{
    //ChannelID: Update, Join, Manage, Write
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.update = true;
    channel_permissions.join = true;
    channel_permissions.write = true;
    permission_object.channels.push_back(channel_id);
    permission_object.channel_permissions.push_back(channel_permissions);

    //Current User ID: Update, Get
    Pubnub::UserPermissions invitee_permissions;
    invitee_permissions.update = true;
    invitee_permissions.get = true;
    permission_object.users.push_back(current_user_id);

    //Invitee ID: Update, Get
    permission_object.users.push_back(invitee_id);
    permission_object.user_permissions.push_back(invitee_permissions);

    //Channel(Invitee_IDs): Write
    Pubnub::ChannelPermissions channel_invitee_permissions;
    channel_invitee_permissions.write = true;
    permission_object.channels.push_back(invitee_id);
    permission_object.channel_permissions.push_back(channel_invitee_permissions);

    return token_access_manager.grant_token(permission_object);
}

Pubnub::String get_create_group_conversation_token(Pubnub::AccessManager& token_access_manager, Pubnub::String channel_id, Pubnub::String current_user_id, std::vector<Pubnub::String> invitee_ids)
{
    //ChannelID: Update, Join, Manage, Write
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

    //Current User ID: Update, Get
    Pubnub::UserPermissions invitee_permissions;
    invitee_permissions.update = true;
    invitee_permissions.get = true;
    permission_object.users.push_back(current_user_id);

    //Invitees IDs: Update, Get
    for(auto invitee_id : invitee_ids) {permission_object.users.push_back(invitee_id);}

    permission_object.user_permissions.push_back(invitee_permissions);

    //Channel(Invitees_IDs): Write
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
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.get = true;
    for(auto channel_id : channel_ids) {permission_object.channels.push_back(channel_id);}
    permission_object.channel_permissions.push_back(channel_permissions);

    return token_access_manager.grant_token(permission_object);
}