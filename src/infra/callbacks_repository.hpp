#ifndef PN_CHAT_CALLBACKS_REPOSITORY_HPP
#define PN_CHAT_CALLBACKS_REPOSITORY_HPP

#include "message.hpp"
#include "channel.hpp"
#include "user.hpp"
#include "membership.hpp"
#include "infra/repository.hpp"
#include "string.hpp"

class CallbacksRepository {
    template <typename T>
    using FunctionRepository = Repository<Pubnub::String, std::function<void(T)>, Pubnub::StringComparer>;
    template <typename T, typename U>
    using TupleFunctionRepository = Repository<Pubnub::String, std::tuple<T, std::function<void(U)>>, Pubnub::StringComparer>;

    public:
        CallbacksRepository() = default;

        FunctionRepository<Pubnub::Message>& get_message_callbacks();
        TupleFunctionRepository<Pubnub::String, Pubnub::Message>& get_message_update_callbacks();
        FunctionRepository<Pubnub::Channel>& get_channel_callbacks();
        TupleFunctionRepository<Pubnub::pubnub_chat_event_type, Pubnub::String>& get_event_callbacks();
        FunctionRepository<Pubnub::User>& get_user_callbacks();
        FunctionRepository<std::vector<Pubnub::String>>& get_channel_presence_callbacks();
        TupleFunctionRepository<Pubnub::String, Pubnub::Membership>& get_membership_callbacks();

    private:
        FunctionRepository<Pubnub::Message> message_callbacks;
        TupleFunctionRepository<Pubnub::String, Pubnub::Message> message_update_callbacks;
        FunctionRepository<Pubnub::Channel> channel_callbacks;
        TupleFunctionRepository<Pubnub::pubnub_chat_event_type, Pubnub::String> event_callbacks;
        FunctionRepository<Pubnub::User> user_callbacks;
        FunctionRepository<std::vector<Pubnub::String>> channel_presence_callbacks;
        TupleFunctionRepository<Pubnub::String, Pubnub::Membership> membership_callbacks;
};

#endif // PN_CHAT_CALLBACKS_REPOSITORY_HPP
