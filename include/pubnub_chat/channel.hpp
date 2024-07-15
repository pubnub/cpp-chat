#ifndef PN_CHAT_CHANNEL_HPP
#define PN_CHAT_CHANNEL_HPP

#include "string.hpp"
#include "helpers/export.hpp"
#include "restrictions.hpp"
#include "enums.hpp"
#include <memory>
#include <vector>
#include <functional>

#include "message_draft_config.hpp"

class ChannelService; 
class ChatService;
class PresenceService;
class RestrictionsService;
class MessageService;
class MembershipService;

namespace Pubnub 
{
    class Message;
    class Membership;
    class User;
    class MessageDraft;

    struct ChatChannelData
    {
        Pubnub::String channel_name = "";
        Pubnub::String description = "";
        Pubnub::String custom_data_json = "";
        Pubnub::String updated = "";
        Pubnub::String status = "";
        Pubnub::String type = "";
    };

    class Channel {
        public:
            PN_CHAT_EXPORT inline Pubnub::String channel_id(){return channel_id_internal;};
            PN_CHAT_EXPORT Pubnub::ChatChannelData channel_data();

            PN_CHAT_EXPORT Pubnub::Channel update(ChatChannelData in_additional_channel_data);
            PN_CHAT_EXPORT void connect(std::function<void(Message)> message_callback);
            PN_CHAT_EXPORT void disconnect();
            PN_CHAT_EXPORT void join(std::function<void(Message)> message_callback, Pubnub::String additional_params = "");
            PN_CHAT_EXPORT void leave();
            PN_CHAT_EXPORT void delete_channel();
            PN_CHAT_EXPORT void send_text(Pubnub::String message, Pubnub::pubnub_chat_message_type message_type = Pubnub::pubnub_chat_message_type::PCMT_TEXT, Pubnub::String meta_data = "");
            PN_CHAT_EXPORT std::vector<Pubnub::String> who_is_present();
            PN_CHAT_EXPORT bool is_present(Pubnub::String user_id);
            PN_CHAT_EXPORT void set_restrictions(Pubnub::String user_id, Pubnub::Restriction restrictions);
            PN_CHAT_EXPORT Pubnub::Restriction get_user_restrictions(Pubnub::String user_id, Pubnub::String channel_id, int limit, Pubnub::String start, Pubnub::String end);
            PN_CHAT_EXPORT std::vector<Pubnub::Message> get_history(Pubnub::String start_timetoken, Pubnub::String end_timetoken, int count);
            PN_CHAT_EXPORT Pubnub::Message get_message(Pubnub::String timetoken);
            PN_CHAT_EXPORT std::vector<Pubnub::Membership> get_members(int limit, Pubnub::String start_timetoken, Pubnub::String end_timetoken);
            PN_CHAT_EXPORT Pubnub::Membership invite(Pubnub::User user);
            PN_CHAT_EXPORT std::vector<Pubnub::Membership> invite_multiple(std::vector<Pubnub::User> users);
            PN_CHAT_EXPORT void start_typing();
            PN_CHAT_EXPORT void stop_typing();
            PN_CHAT_EXPORT void get_typing(std::function<void(std::vector<Pubnub::String>)> typing_callback);
            PN_CHAT_EXPORT Pubnub::Channel pin_message(Pubnub::Message message);
            PN_CHAT_EXPORT Pubnub::Channel unpin_message();
            PN_CHAT_EXPORT Pubnub::Message get_pinned_message();
            PN_CHAT_EXPORT void forward_message(Pubnub::Message message);


            PN_CHAT_EXPORT void stream_updates(std::function<void(Pubnub::Channel)> channel_callback);
            PN_CHAT_EXPORT void stream_updates_on(std::vector<Pubnub::Channel> channels, std::function<void(Pubnub::Channel)> channel_callback);
            PN_CHAT_EXPORT void stream_presence(std::function<void(std::vector<Pubnub::String>)> presence_callback);

            PN_CHAT_EXPORT Pubnub::MessageDraft create_message_draft(Pubnub::MessageDraftConfig message_draft_config = Pubnub::MessageDraftConfig());

        protected:
            PN_CHAT_EXPORT Channel(Pubnub::String channel_id, std::shared_ptr<ChatService> chat_service, std::shared_ptr<ChannelService> channel_service, std::shared_ptr<PresenceService> presence_service, 
                                   std::shared_ptr<RestrictionsService> restrictions_service, std::shared_ptr<MessageService> message_service, std::shared_ptr<MembershipService> membership_service);
            
            Pubnub::String channel_id_internal;

            std::shared_ptr<ChannelService> channel_service;
            std::shared_ptr<ChatService> chat_service;
            std::shared_ptr<PresenceService> presence_service;
            std::shared_ptr<RestrictionsService> restrictions_service;
            std::shared_ptr<MessageService> message_service;
            std::shared_ptr<MembershipService> membership_service;

        friend class ::ChannelService;
    };
};

#endif // PN_CHAT_CHANNEL_HPP
