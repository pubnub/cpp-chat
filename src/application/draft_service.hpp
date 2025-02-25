#ifndef PN_CHAT_DRAFT_SERVICE_HPP
#define PN_CHAT_DRAFT_SERVICE_HPP

#include "application/channel_service.hpp"
#include "application/chat_service.hpp"
#include "application/dao/message_draft_dao.hpp"
#include "application/user_service.hpp"
#include "domain/message_draft_entity.hpp"
#include "message_draft.hpp"
#include "message_draft_config.hpp"
#include "message_elements.hpp"
#include "send_text_params.hpp"
#include "string.hpp"
#include "user.hpp"
#include <memory>
#include <utility>

class DraftService {
    public:
        // TODO: consider weak_ptrs
        DraftService(std::shared_ptr<const UserService> user_service, std::shared_ptr<const ChannelService> channel_service);
        ~DraftService() = default;

        void insert_text_to_message(MessageDraftDAO& dao, std::size_t position, const Pubnub::String& text_to_insert) const; 
        void remove_text_from_message(MessageDraftDAO& dao, std::size_t position, std::size_t length) const;
        void insert_suggested_mention_to_message(MessageDraftDAO& dao, const Pubnub::SuggestedMention& suggested_mention, const Pubnub::String& text) const;
        void add_mention_to_message(MessageDraftDAO& dao, std::size_t position, std::size_t length, const Pubnub::MentionTarget& target) const;
        void remove_mention_from_message(MessageDraftDAO& dao, std::size_t position) const;
        void update_message(MessageDraftDAO& dao, const Pubnub::String& text) const;

        std::pair<Pubnub::SendTextParams, Pubnub::String> prepare_sending_data(
                const MessageDraftDAO& dao,
                const Pubnub::SendTextParams& params 
        ) const;
    private:
        void fire_message_elements_changed(MessageDraftDAO& dao) const;
        std::vector<Pubnub::SuggestedMention> resolve_suggestions(const MessageDraftMentionEntity& suggestion) const;
        std::vector<Pubnub::User> get_users_suggestions(const Pubnub::String& query) const;
        std::vector<Pubnub::Channel> get_channels_suggestions(const Pubnub::String& query) const;
        std::vector<Pubnub::MessageElement> get_message_elements(const MessageDraftEntity& entity) const;
        static MessageDraftSuggestedMentionEntity convert_suggested_mention_to_domain(const Pubnub::SuggestedMention& suggested_mention);
        static MessageDraftMentionTargetEntity convert_mention_target_to_domain(const Pubnub::MentionTarget& target);
        static Pubnub::MessageElement convert_message_element_to_presentation(const MessageDraftMessageElementEntity& element);
        static Pubnub::MentionedUser convert_mentioned_user_to_presentation(const MessageDraftEntity& entity, const MessageDraftMentionEntity& user);
        static Pubnub::MentionTarget convert_mention_target_to_presentation(const MessageDraftMentionTargetEntity& target);

        std::shared_ptr<const ChannelService> channel_service;
        std::shared_ptr<const UserService> user_service;
        Pubnub::MessageDraftConfig draft_config;
};

#endif // PN_CHAT_DRAFT_SERVICE_HPP
