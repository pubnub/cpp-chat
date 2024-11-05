#include "draft_service.hpp"
#include "application/chat_service.hpp"
#include "domain/message_draft_entity.hpp"
#include <memory>

DraftService::DraftService(std::shared_ptr<const UserService> user_service, std::shared_ptr<const ChannelService> channel_service) :
    channel_service(channel_service),
    user_service(user_service) {}

void DraftService::insert_text_to_message(MessageDraftDAO& dao, std::size_t position, const Pubnub::String& text_to_insert) const {
    dao.update_entity(dao.get_entity().insert_text(position, text_to_insert));
    this->fire_message_elements_changed(dao);
}

void DraftService::remove_text_from_message(MessageDraftDAO& dao, std::size_t position, std::size_t length) const {
    dao.update_entity(dao.get_entity().remove_text(position, length));
    this->fire_message_elements_changed(dao);
}

void DraftService::insert_suggested_mention_to_message(MessageDraftDAO& dao, const Pubnub::SuggestedMention& suggested_mention, const Pubnub::String& text) const {
    auto domain_mention = DraftService::convert_suggested_mention_to_domain(suggested_mention);
    auto entity = dao.get_entity();

    if (!entity.validate_suggested_mention(domain_mention)) {
        throw std::invalid_argument("Invalid suggested mention!");
    }
    
    dao.update_entity(entity.insert_suggested_mention(domain_mention, text));
    this->fire_message_elements_changed(dao);
}

void DraftService::add_mention_to_message(MessageDraftDAO& dao, std::size_t position, std::size_t length, const Pubnub::MentionTarget& target) const {
    dao.update_entity(dao.get_entity().add_mention(position, length, DraftService::convert_mention_target_to_domain(target)));
    this->fire_message_elements_changed(dao);
}

void DraftService::remove_mention_from_message(MessageDraftDAO& dao, std::size_t position) const {
    dao.update_entity(dao.get_entity().remove_mention(position));
    this->fire_message_elements_changed(dao);
}

void DraftService::update_message(MessageDraftDAO& dao, const Pubnub::String& text) const {
    dao.update_entity(dao.get_entity().update(text));
    this->fire_message_elements_changed(dao);
}

void DraftService::fire_message_elements_changed(MessageDraftDAO& dao) const {
    auto entity = dao.get_entity();

    if (dao.should_search_for_suggestions()) {
        auto raw_mentions = entity.suggest_raw_mentions();

        if (!raw_mentions.empty()) {
            for (const auto& raw_mention : raw_mentions) {
                auto suggestions = this->user_service
                    ->get_users_suggestions(raw_mention.target.target);
            }
        }


    }

    // TODO: elements and mentions
    dao.call_callbacks({}, {});
}

MessageDraftSuggestedMentionEntity DraftService::convert_suggested_mention_to_domain(const Pubnub::SuggestedMention& suggested_mention) {
    return MessageDraftSuggestedMentionEntity{
        suggested_mention.offset,
        suggested_mention.replace_from,
        suggested_mention.replace_to,
        DraftService::convert_mention_target_to_domain(suggested_mention.target)
    };
}

MessageDraftMentionTargetEntity DraftService::convert_mention_target_to_domain(const Pubnub::MentionTarget& mention_target) {
    auto type = mention_target.get_type();

    return MessageDraftMentionTargetEntity{
        mention_target.get_target(),
        type == Pubnub::MentionTarget::Type::USER 
            ? MessageDraftMentionTargetEntity::Type::USER 
            : type == Pubnub::MentionTarget::Type::CHANNEL 
                ? MessageDraftMentionTargetEntity::Type::CHANNEL 
                : MessageDraftMentionTargetEntity::Type::URL
    };
}


