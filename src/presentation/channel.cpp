#include "presentation/channel.hpp"
#include "presentation/message.hpp"
#include "application/channel_service.hpp"

using namespace Pubnub;

Channel::Channel(Pubnub::String channel_id, std::shared_ptr<ChatService> chat_service, std::shared_ptr<ChannelService> channel_service) :
channel_id_internal(channel_id),
chat_service(chat_service),
channel_service(channel_service)
{}

Pubnub::ChatChannelData Channel::channel_data()
{
    return channel_service->get_channel_data(channel_id_internal);
}

Pubnub::Channel Channel::update(ChatChannelData in_additional_channel_data)
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

void Channel::join(std::function<void(Message)> message_callback, Pubnub::String additional_params)
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

std::vector<Pubnub::String> Channel::who_is_present()
{
    return this->channel_service->who_is_present(channel_id_internal);
}

bool Channel::is_present(Pubnub::String user_id)
{
    std::vector<String> users = this->who_is_present();
    //TODO: we should us std::count here, but it didn't work
    int count = 0;
    for( auto user : users)
    {
        if(user_id == user)
        {
            count = 1;
            break;
        }
    }
    //int count = std::count(channels.begin(), channels.end(), channel_id);
    return count > 0;
}