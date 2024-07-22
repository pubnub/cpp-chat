#include "presence.hpp"
#include "string.hpp"
#include "json.hpp"

std::vector<Pubnub::String> Presence::users_from_response(const Json &response) {
    std::vector<Pubnub::String> users;

    for(auto user : response["uuids"]) {
        users.push_back(user);
    }

    return users;
}

std::vector<Pubnub::String> Presence::channels_from_response(const Json &response) {
    std::vector<Pubnub::String> channels;

    for(auto channel : response["payload"]["channels"]) {
        channels.push_back(channel);
    }

    return channels;
}
