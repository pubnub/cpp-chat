#include "presentation/channel.hpp"
#include "presentation/message.hpp"
#include "application/channel_service.hpp"
#include "application/presence_service.hpp"
#include "application/restrictions_service.hpp"
#include "application/message_service.hpp"

using namespace Pubnub;

Channel::Channel(String channel_id, std::shared_ptr<ChatService> chat_service, std::shared_ptr<ChannelService> channel_service, std::shared_ptr<PresenceService> presence_service, 
                std::shared_ptr<RestrictionsService> restrictions_service, std::shared_ptr<MessageService> message_service, std::shared_ptr<MembershipService> membership_service) :
channel_id_internal(channel_id),
chat_service(chat_service),
channel_service(channel_service),
presence_service(presence_service),
restrictions_service(restrictions_service),
message_service(message_service),
membership_service(membership_service)
{}

ChatChannelData Channel::channel_data()
{
    return channel_service->get_channel_data(channel_id_internal);
}

Channel Channel::update(ChatChannelData in_additional_channel_data)
{
    return this->channel_service->update_channel(channel_id_internal, in_additional_channel_data);
}

void Channel::connect(std::function<void(Message)> message_callback) 
{
    this->channel_service->connect(channel_id_internal, message_callback);
}

void Channel::disconnect()
{
    this->channel_service->disconnect(channel_id_internal);
}

void Channel::join(std::function<void(Message)> message_callback, String additional_params)
{
    this->channel_service->join(channel_id_internal, message_callback, additional_params);
}

void Channel::leave()
{
    this->channel_service->leave(channel_id_internal);
}

void Channel::delete_channel()
{
    this->channel_service->delete_channel(channel_id_internal);
}

void Channel::send_text(String message, pubnub_chat_message_type message_type, String meta_data)
{
    this->channel_service->send_text(channel_id_internal, message, message_type, meta_data);
}

std::vector<String> Channel::who_is_present()
{
    return this->presence_service->who_is_present(channel_id_internal);
}

bool Channel::is_present(String user_id)
{
    return this->presence_service->is_present(user_id, channel_id_internal);
}

void Channel::set_restrictions(String user_id, Restriction restrictions)
{
    this->restrictions_service->set_restrictions(user_id, channel_id_internal, restrictions);
}

Restriction Channel::get_user_restrictions(String user_id, String channel_id, int limit, String start, String end)
{
    return this->restrictions_service->get_user_restrictions(user_id, channel_id, limit, start, end);
}

std::vector<Message> Channel::get_history(String start_timetoken, String end_timetoken, int count)
{
    return this->message_service->get_channel_history(channel_id(), start_timetoken, end_timetoken, count);
}

Message Channel::get_message(String timetoken)
{
    return this->message_service->get_message(timetoken, channel_id());
}
