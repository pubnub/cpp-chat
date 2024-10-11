#include "channel_dao.hpp"
#include "chat_helpers.hpp"
#include "domain/channel_entity.hpp"

ChannelDAO::ChannelDAO(const ChannelEntity channel_entity) :
    channel_entity(std::move(channel_entity))
{}

ChannelDAO::ChannelDAO(const Pubnub::ChatChannelData& channel_data) :
    channel_entity(entity_from_channel_data(channel_data))
{}

Pubnub::ChatChannelData ChannelDAO::to_channel_data() const {
    Pubnub::ChatChannelData channel_data;
    channel_data.channel_name = this->channel_entity.channel_name;
    channel_data.description = this->channel_entity.description;
    channel_data.custom_data_json = this->channel_entity.custom_data_json;
    channel_data.updated = this->channel_entity.updated;
    channel_data.status = this->channel_entity.status;
    channel_data.type = this->channel_entity.type;

    return channel_data;
}

ChannelEntity ChannelDAO::to_entity() const {
    return this->channel_entity;
}

const ChannelEntity& ChannelDAO::get_entity() const {
    return this->channel_entity;
}

void ChannelDAO::start_typing(int miliseconds) const  {
    this->typing.lock()->start();

    *this->typing_timer.lock() = Timer(miliseconds, [this]() {
        this->typing.lock()->stop();
    });
}

void ChannelDAO::stop_typing() const {
    this->typing_timer.lock()->stop();
    this->typing.lock()->stop();
}

void ChannelDAO::start_typing_indicator(const Pubnub::String& user_id, int miliseconds, std::function<void(const std::vector<Pubnub::String>&)> on_timeout) const {
    auto indicators_timers = this->indicators_timers.lock();
    (*indicators_timers)[user_id] = Timer(5000, [this, user_id, on_timeout]() {
        this->remove_typing_indicator(user_id);
        on_timeout(this->typing.lock()->get_typing_indicators());
    });

    this->typing.lock()->add_typing_indicator(user_id);
}

void ChannelDAO::stop_typing_indicator(const Pubnub::String& user_id) const {
    (*this->indicators_timers.lock())[user_id].stop();
}

void ChannelDAO::remove_typing_indicator(const Pubnub::String& user_id) const {
    this->typing.lock()->remove_typing_indicator(user_id);
}

bool ChannelDAO::is_typing_sent() const {
    return this->typing.lock()->is_sent();
}

bool ChannelDAO::contains_typing_indicator(const Pubnub::String& user_id) const {
    return this->typing.lock()->contains_typing_indicator(user_id);
}

std::vector<Pubnub::String> ChannelDAO::get_typing_indicators() const {
    return this->typing.lock()->get_typing_indicators();
}

ChannelEntity ChannelDAO::entity_from_channel_data(const Pubnub::ChatChannelData& channel_data) {
    ChannelEntity entity;
    entity.channel_name = channel_data.channel_name;
    entity.description = channel_data.description;
    entity.custom_data_json = channel_data.custom_data_json;
    entity.updated = channel_data.updated;
    entity.status = channel_data.status;
    entity.type = channel_data.type;

    return entity;
}
