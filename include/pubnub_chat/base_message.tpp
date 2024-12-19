#ifndef PN_CHAT_BASE_MESSAGE_TPP
#define PN_CHAT_BASE_MESSAGE_TPP

#include <memory>

#include "application/dao/message_dao.hpp"
#include "base_message.hpp"
#include "channel.hpp"
#include "enums.hpp"

namespace Pubnub {
    template<typename MessageType>
    BaseMessage<MessageType>::BaseMessage() :
        timetoken_internal(""),
        chat_service(nullptr),
        message_service(nullptr),
        channel_service(nullptr),
        restrictions_service(nullptr),
        data(nullptr) {}

    template<typename MessageType>
    BaseMessage<MessageType>::BaseMessage(
        String timetoken,
        std::shared_ptr<const ChatService> chat_service,
        std::shared_ptr<const MessageService> message_service,
        std::shared_ptr<const ChannelService> channel_service,
        std::shared_ptr<const RestrictionsService> restrictions_service,
        std::unique_ptr<MessageDAO> data
    ) :
        timetoken_internal(timetoken),
        chat_service(chat_service),
        message_service(message_service),
        channel_service(channel_service),
        restrictions_service(restrictions_service),
        data(std::move(data)) {}

    template<typename MessageType>
    BaseMessage<MessageType>::BaseMessage(const BaseMessage<MessageType>& other) :
        timetoken_internal(other.timetoken_internal),
        chat_service(other.chat_service),
        message_service(other.message_service),
        channel_service(other.channel_service),
        restrictions_service(other.restrictions_service),
        data(std::make_unique<MessageDAO>(other.data->to_message_data())) {}

    template<typename MessageType>
    BaseMessage<MessageType>&
        BaseMessage<MessageType>::operator=(const BaseMessage<MessageType>& other) {
        if (this == &other) {
            return *this;
        }

        timetoken_internal = other.timetoken_internal;
        chat_service = other.chat_service;
        message_service = other.message_service;
        channel_service = other.channel_service;
        restrictions_service = other.restrictions_service;
        data = std::make_unique<MessageDAO>(other.data->to_message_data());

        return *this;
    }

    template<typename MessageType>
    BaseMessage<MessageType>::~BaseMessage() = default;

    template<typename MessageType>
    String BaseMessage<MessageType>::timetoken() const {
        return timetoken_internal;
    }

    template<typename MessageType>
    ChatMessageData BaseMessage<MessageType>::message_data() const {
        return data->to_message_data();
    }

    template<typename MessageType>
    String BaseMessage<MessageType>::text() const {
        return this->message_service->text(*this->data);
    }

    template<typename MessageType>
    bool BaseMessage<MessageType>::delete_message_hard() const {
        return this->message_service->delete_message_hard(*this);
    }

    template<typename MessageType>
    bool BaseMessage<MessageType>::deleted() const {
        return this->message_service->deleted(*this->data);
    }

    template<typename MessageType>
    pubnub_chat_message_type BaseMessage<MessageType>::type() const {
        return this->message_data().type;
    }

    template<typename MessageType>
    void BaseMessage<MessageType>::pin() const {
        this->channel_service->get_channel(this->message_data().channel_id).pin_message(*this);
    }

    template<typename MessageType>
    void BaseMessage<MessageType>::unpin() const {
        auto channel = this->channel_service->get_channel(this->message_data().channel_id);

        // TODO: consider optional approach
        try {
            channel.get_pinned_message();
        } catch (const std::exception& e) {
            if (Pubnub::String(e.what()).find("no any pinned")) {
                return;
            }

            throw e;
        }

        channel.unpin_message();
    }

    template<typename MessageType>
    Pubnub::Vector<MessageAction> BaseMessage<MessageType>::reactions() const {
        return Pubnub::Vector<MessageAction>(
            std::move(this->message_service->get_message_reactions(*this->data))
        );
    }

    template<typename MessageType>
    bool BaseMessage<MessageType>::has_user_reaction(const String& reaction) const {
        auto message_reactions = this->message_service->get_message_reactions(*this->data);

        return std::find_if(
                   message_reactions.begin(),
                   message_reactions.end(),
                   [&reaction](const MessageAction& message_reaction) {
                       return message_reaction.value == reaction;
                   }
               )
            != message_reactions.end();
    }

    template<typename MessageType>
    void BaseMessage<MessageType>::forward(const String& channel_id) const {
        return this->message_service->forward_message(*this, channel_id);
    }

    template<typename MessageType>
    ThreadChannel BaseMessage<MessageType>::create_thread() const {
        return this->channel_service->create_thread_channel(*this);
    }

    template<typename MessageType>
    ThreadChannel BaseMessage<MessageType>::get_thread() const {
        return this->channel_service->get_thread_channel(*this);
    }

    template<typename MessageType>
    bool BaseMessage<MessageType>::has_thread() const {
        return this->channel_service->has_thread_channel(*this);
    }

    template<typename MessageType>
    void BaseMessage<MessageType>::remove_thread() const {
        this->channel_service->remove_thread_channel(*this);
    }

    template<typename MessageType>
    Pubnub::Vector<MentionedUser> BaseMessage<MessageType>::mentioned_users() const {
        auto users_map = this->data->to_entity().get_mentioned_users();
        std::vector<MentionedUser> users_vector;
        for (auto it = users_map.begin(); it != users_map.end(); it++) {
            users_vector.push_back(it->second);
        }
        return Pubnub::Vector<MentionedUser>(std::move(users_vector));
    }

    template<typename MessageType>
    Pubnub::Vector<ReferencedChannel> BaseMessage<MessageType>::referenced_channels() const {
        auto channels_map = this->data->to_entity().get_referenced_channels();
        std::vector<ReferencedChannel> channels_vector;
        for (auto it = channels_map.begin(); it != channels_map.end(); it++) {
            channels_vector.push_back(it->second);
        }
        return Pubnub::Vector<ReferencedChannel>(std::move(channels_vector));
    }

    template<typename MessageType>
    Pubnub::Vector<TextLink> BaseMessage<MessageType>::text_links() const {
        auto text_links = this->data->to_entity().get_text_links();
        return Pubnub::Vector<TextLink>(std::move(text_links));
    }

}  // namespace Pubnub

#endif  // PN_CHAT_BASE_MESSAGE_TPP
