#include "presentation/user.hpp"
#include "application/user_service.hpp"
#include "application/presence_service.hpp"

using namespace Pubnub;

User::User(Pubnub::String user_id, std::shared_ptr<ChatService> chat_service, std::shared_ptr<UserService> user_service, std::shared_ptr<PresenceService> presence_service) :
user_id_internal(user_id),
chat_service(chat_service),
user_service(user_service),
presence_service(presence_service)
{}

User User::update(ChatUserData user_data)
{
    return this->user_service->update_user(user_id_internal, user_data);
}

void User::delete_user()
{
    this->user_service->delete_user(user_id_internal);
}

std::vector<Pubnub::String> User::where_present()
{
    return this->presence_service->where_present(user_id_internal);
}

bool User::is_present_on(Pubnub::String channel_id)
{
    return this->presence_service->is_present(user_id_internal, channel_id);
}