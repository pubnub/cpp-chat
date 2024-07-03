#include "presentation/user.hpp"

using namespace Pubnub;

User::User(std::shared_ptr<UserService> user_service, Pubnub::String user_id) :
user_service(user_service),
user_id_internal(user_id)
{}