#include "callbacks_repository.hpp"

CallbacksRepository::FunctionRepository<Pubnub::Message>& CallbacksRepository::get_message_callbacks()
{
    return this->message_callbacks;
}

CallbacksRepository::FunctionRepository<Pubnub::ThreadMessage>& CallbacksRepository::get_thread_message_update_callbacks()
{
    return this->thread_message_callbacks;
}

CallbacksRepository::FunctionRepository<Pubnub::Message>& CallbacksRepository::get_message_update_callbacks()
{
    return this->message_update_callbacks;
}

CallbacksRepository::FunctionRepository<Pubnub::Channel>& CallbacksRepository::get_channel_callbacks()
{
    return this->channel_callbacks;
}

CallbacksRepository::TupleFunctionRepository<Pubnub::pubnub_chat_event_type, Pubnub::Event>& CallbacksRepository::get_event_callbacks()
{
    return this->event_callbacks;
}

CallbacksRepository::FunctionRepository<Pubnub::User>& CallbacksRepository::get_user_callbacks()
{
    return this->user_callbacks;
}

CallbacksRepository::FunctionRepository<std::vector<Pubnub::String>>& CallbacksRepository::get_channel_presence_callbacks()
{
    return this->channel_presence_callbacks;
}

CallbacksRepository::TupleFunctionRepository<Pubnub::String, Pubnub::Membership>& CallbacksRepository::get_membership_callbacks()
{
    return this->membership_callbacks;
}
