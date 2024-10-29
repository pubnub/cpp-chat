#include "message_draft_entity.hpp"
#include <algorithm>

std::size_t MessageDraftMentionEntity::end_exclusive() const {
    return this->start + this->length;
}

MessageDraftEntity MessageDraftEntity::insert_text(std::size_t position, const Pubnub::String& text) const {
    auto new_value = this->value;

    new_value.replace(position, 0, text);

    std::vector<MessageDraftMentionEntity> new_mentions;

    std::copy_if(
        this->mentions.begin(),
        this->mentions.end(),
        std::back_inserter(new_mentions),
        [position](const auto& mention) {
            return position <= mention.start || position >= mention.end_exclusive();
        }
    );

    std::for_each(
        new_mentions.begin(),
        new_mentions.end(),
        [position, text](auto& mention) {
            if (position <= mention.start) {
                mention.start += text.length();
            }
        }
    );
    
    return MessageDraftEntity{new_value, new_mentions};
}

MessageDraftEntity MessageDraftEntity::remove_text(std::size_t position, std::size_t length) const {
    auto new_value = this->value;

    new_value.erase(position, length);

    std::vector<MessageDraftMentionEntity> new_mentions;

    std::copy_if(
        this->mentions.begin(),
        this->mentions.end(),
        std::back_inserter(new_mentions),
        [position, length](const auto& mention) {
            return position > mention.end_exclusive() || position + length <= mention.start;
        }
    );

    std::for_each(
        new_mentions.begin(),
        new_mentions.end(),
        [position, length](auto& mention) {
            if (position < mention.start) {
                mention.start = std::min(length, mention.start - length);
            }
        }
    );

    return MessageDraftEntity{new_value, new_mentions};
}

MessageDraftEntity MessageDraftEntity::add_mention(std::size_t start, std::size_t length, const MessageDraftMentionTargetEntity& target) const {
    auto new_mentions = this->mentions;

    new_mentions.push_back(MessageDraftMentionEntity{start, length, target});

    std::sort(
        new_mentions.begin(),
        new_mentions.end(),
        [](const auto& a, const auto& b) {
            return a.start < b.start;
        }
    );

    return MessageDraftEntity{this->value, new_mentions};
}

MessageDraftEntity MessageDraftEntity::remove_mention(std::size_t start) const {
    std::vector<MessageDraftMentionEntity> new_mentions;

    std::copy_if(
        this->mentions.begin(),
        this->mentions.end(),
        std::back_inserter(new_mentions),
        [start](const auto& mention) {
            return start != mention.start;
        }
    );

    std::sort(
        new_mentions.begin(),
        new_mentions.end(),
        [](const auto& a, const auto& b) {
            return a.start < b.start;
        }
    );

    return MessageDraftEntity{this->value, new_mentions};
}

MessageDraftEntity MessageDraftEntity::insert_suggested_mention(
        const MessageDraftSuggestedMentionEntity& suggested_mention,
        const Pubnub::String& text)
const {
    return this->remove_text(suggested_mention.position, suggested_mention.replace_from.length())
        .insert_text(suggested_mention.position, text)
        .add_mention(suggested_mention.position, text.length(), suggested_mention.target);
}

bool MessageDraftEntity::validate_mentions() const {
    for (std::size_t i = 0; i < this->mentions.size(); i++) {
        if (i > 0 && this->mentions[i].start < this->mentions[i - 1].end_exclusive()) {
            return false;
        }
    }

    return true;
}

bool MessageDraftEntity::validate_suggested_mention(const MessageDraftSuggestedMentionEntity& suggested_mention) const {
    return this->value.substring(suggested_mention.position, suggested_mention.replace_from.length())
        == suggested_mention.replace_from;
}
