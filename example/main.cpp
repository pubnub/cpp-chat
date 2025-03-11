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

    Pubnub::String create_user_id = Pubnub::String("cpp_chat_test_created_user");

    chat_access_manager.set_auth_token(get_create_user_token(token_access_manager, create_user_id));
    auto created_user = chat.create_user(create_user_id, Pubnub::ChatUserData());


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

    chat_access_manager.set_auth_token(get_delete_channel_token(token_access_manager, group_channel_id));
    chat.delete_channel(group_channel_id);



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



    //CHANNEL JOIN
    std::cout << "Channel.Join" << std::endl;

    chat_access_manager.set_auth_token(get_join_channel_token(token_access_manager, public_channel_id, user_id));
    auto join_lambda = [](Pubnub::Message message){};
    public_channel.join(join_lambda);



    std::this_thread::sleep_for(std::chrono::seconds(1));


    
    //THESE FUNCTIONS DON'T REQUIRE PERMISSIONS
    chat_access_manager.set_auth_token(get_get_user_token(token_access_manager, "blabla"));
    std::cout << "Get Channel Suggestions" << std::endl;
    chat.get_channel_suggestions("cpp");
    std::cout << "Get User Suggestions" << std::endl;
    chat.get_user_suggestions("cpp");
    std::cout << "Where Present" << std::endl;
    chat.where_present(user_id);
    std::cout << "Is present" << std::endl;
    chat.is_present(user_id, public_channel_id);


    std::this_thread::sleep_for(std::chrono::seconds(1));



    //GET USER SUGGESTIONS
    std::cout << "Get User Suggestions" << std::endl;

    //This shouldn't require any permissions
    chat.get_user_suggestions("cpp");



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
    //ChannelID: UPDATE, JOIN, WRITE
    Pubnub::GrantTokenPermissionObject permission_object;
    permission_object.authorized_user = TOKEN_AUTH_USER_ID;
    permission_object.ttl_minutes = TOKEN_TTL;
    Pubnub::ChannelPermissions channel_permissions;
    channel_permissions.update = true;
    channel_permissions.join = true;
    channel_permissions.write = true;
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