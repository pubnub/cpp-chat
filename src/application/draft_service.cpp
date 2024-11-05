#include "draft_service.hpp"
#include "application/chat_service.hpp"
#include "domain/message_draft_entity.hpp"
#include "message_draft.hpp"
#include "vector.hpp"
#include <memory>

#define PN_MIN_MENTION_LENGTH 3

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

    Pubnub::Vector<Pubnub::MessageElement>message_elements(this->get_message_elements(entity));

    if (!dao.should_search_for_suggestions()) {
        dao.call_callbacks(message_elements);
        return;
    }

    auto raw_mentions = entity.suggest_raw_mentions();

    std::vector<Pubnub::SuggestedMention> suggestions;
    for (const auto& raw_mention : raw_mentions) {
        auto resolved_suggestions = this->resolve_suggestions(raw_mention);
        suggestions.insert(suggestions.end(), resolved_suggestions.begin(), resolved_suggestions.end());
    }

    dao.call_callbacks(
            message_elements,
            Pubnub::Vector<Pubnub::SuggestedMention>(std::move(suggestions))
    );
}

std::vector<Pubnub::SuggestedMention> DraftService::resolve_suggestions(const MessageDraftMentionEntity& suggestion) const {
    if (suggestion.target.type == MessageDraftMentionTargetEntity::Type::USER) {
        if (suggestion.target.target.length() < PN_MIN_MENTION_LENGTH) {
            return {};
        }

        auto suggested_users = this->get_users_suggestions(suggestion.target.target.substring(1, suggestion.target.target.length() - 1));

        std::vector<Pubnub::SuggestedMention> suggestions;

        for (const auto& suggested_user : suggested_users) {
            suggestions.push_back(Pubnub::SuggestedMention{
                suggestion.start,
                suggestion.target.target,
                suggested_user.user_data().user_name.empty() ? suggested_user.user_id() : suggested_user.user_data().user_name,
                Pubnub::MentionTarget::user(suggested_user.user_id())
            });
        }

        return suggestions;
    } else if (suggestion.target.type == MessageDraftMentionTargetEntity::Type::CHANNEL) {
        if (suggestion.target.target.length() < PN_MIN_MENTION_LENGTH) {
            return {};
        }

        auto suggested_channels = this->get_channels_suggestions(suggestion.target.target.substring(1, suggestion.target.target.length() - 1));

        std::vector<Pubnub::SuggestedMention> suggestions;

        for (const auto& suggested_channel : suggested_channels) {
            suggestions.push_back(Pubnub::SuggestedMention{
                suggestion.start,
                suggestion.target.target,
                suggested_channel.channel_id(),
                Pubnub::MentionTarget::channel(suggested_channel.channel_id())
            });
        }

        return suggestions;
    }

    return {};
}

std::vector<Pubnub::User> DraftService::get_users_suggestions(const Pubnub::String& query) const {
    if (this->draft_config.user_suggestion_source == Pubnub::MessageDraftConfig::MessageDraftSuggestionSource::Channel) {
        // TODO: implement
        //return this->channel_service->get_user_suggestions_for_channel(query, this->draft_config.user_limit);
    }

    return this->user_service->get_users_suggestions(query, this->draft_config.user_limit);
}

std::vector<Pubnub::Channel> DraftService::get_channels_suggestions(const Pubnub::String& query) const {
    return this->channel_service->get_channel_suggestions(query, this->draft_config.channel_limit);
}

std::vector<Pubnub::MessageElement> DraftService::get_message_elements(const MessageDraftEntity& entity) const {
    auto elements = entity.get_message_elements();
    std::vector<Pubnub::MessageElement> message_elements;

    std::transform(
        elements.begin(),
        elements.end(),
        std::back_inserter(message_elements),
        DraftService::convert_message_element_to_presentation
    );

    return message_elements;
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

Pubnub::MessageElement DraftService::convert_message_element_to_presentation(const MessageDraftMessageElementEntity& element) {
    return element.target.has_value()
        ? Pubnub::MessageElement::link(element.text, Pubnub::MentionTarget::url(element.target.value().target))
        : Pubnub::MessageElement::plain_text(element.text);
}
