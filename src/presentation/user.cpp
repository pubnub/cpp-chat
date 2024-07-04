#include "presentation/user.hpp"
#include "application/user_service.hpp"

using namespace Pubnub;

User::User(Pubnub::String user_id, std::shared_ptr<ChatService> chat_service, std::shared_ptr<UserService> user_service) :
user_id_internal(user_id),
chat_service(chat_service),
user_service(user_service)
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
    return this->user_service->where_present(user_id_internal);
}

bool User::is_present_on(Pubnub::String channel_id)
{
    std::vector<String> channels = this->where_present();
    //TODO: we should us std::count here, but it didn't work
    int count = 0;
    for( auto channel : channels)
    {
        if(channel_id == channel)
        {
            count = 1;
            break;
        }
    }
    //int count = std::count(channels.begin(), channels.end(), channel_id);
    return count > 0;
}