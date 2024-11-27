#include "message_draft_entity.hpp"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <regex>
#include "diff_match_patch.h"

// TODO: this regex
static std::regex user_mention_regex(R"""(((?=\s?)@[a-zA-Z0-9_]+))""");
static std::regex channel_reference_regex(R"""(((?=\s?)#[a-zA-Z0-9_]+))""");

static Pubnub::String schema_user = "pn-user://";
static Pubnub::String schema_channel = "pn-channel://";

MessageDraftMentionTargetEntity MessageDraftMentionTargetEntity::user(const Pubnub::String& user_id) {
    return MessageDraftMentionTargetEntity{user_id, MessageDraftMentionTargetEntity::Type::USER};
}

MessageDraftMentionTargetEntity MessageDraftMentionTargetEntity::channel(const Pubnub::String& channel) {
    return MessageDraftMentionTargetEntity{channel, MessageDraftMentionTargetEntity::Type::CHANNEL};
}

MessageDraftMentionTargetEntity MessageDraftMentionTargetEntity::url(const Pubnub::String& url) {
    return MessageDraftMentionTargetEntity{url, MessageDraftMentionTargetEntity::Type::URL};
}

std::size_t MessageDraftMentionEntity::end_exclusive() const {
    return this->start + this->length;
}

MessageDraftMessageElementEntity MessageDraftMessageElementEntity::plain_text(const Pubnub::String& text) {
    return {text, {}};
}

MessageDraftMessageElementEntity MessageDraftMessageElementEntity::link(const Pubnub::String& text, const MessageDraftMentionTargetEntity& target) {
    return {text, {target}};
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
                mention.start -= std::min(length, mention.start - position);
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


MessageDraftEntity MessageDraftEntity::update(const Pubnub::String& text) const {
    diff_match_patch<std::string> matcher;

    auto diffs = matcher.diff_main(this->value, text);
    matcher.diff_cleanupSemantic(diffs);
    
    size_t consumed = 0;

    auto result = MessageDraftEntity{this->value, std::vector(this->mentions)};

    std::for_each(diffs.begin(), diffs.end(), [&result, &consumed](const auto& action) {
            switch(action.operation) {
                case diff_match_patch<std::string>::Operation::DELETE:
                    result = result.remove_text(consumed, action.text.length());
                    break;

                case diff_match_patch<std::string>::Operation::INSERT:
                    result = result.insert_text(consumed, action.text);
                    consumed += action.text.length();
                    break;

                case diff_match_patch<std::string>::Operation::EQUAL:
                    consumed += action.text.length();
                    break;
            }
    });

    return result;
}

MessageDraftEntity MessageDraftEntity::insert_suggested_mention(
        const MessageDraftSuggestedMentionEntity& suggested_mention,
        const Pubnub::String& text)
const {
    return this->remove_text(suggested_mention.position, suggested_mention.replace_from.length())
        .insert_text(suggested_mention.position, text)
        .add_mention(suggested_mention.position, text.length(), suggested_mention.target);
}

std::vector<MessageDraftMentionEntity> MessageDraftEntity::suggest_raw_mentions() const {
    auto std_value = this->value.to_std_string();

    std::regex_token_iterator<std::string::iterator> regex_end;
    std::regex_token_iterator<std::string::iterator> all_user_mentions(
        std_value.begin(),
        std_value.end(),
        user_mention_regex
    );

    std::regex_token_iterator<std::string::iterator> all_channel_mentions(
        std_value.begin(),
        std_value.end(),
        channel_reference_regex
    );

    std::vector<MessageDraftMentionEntity> user_mentions;
    for (auto it = all_user_mentions; it != regex_end; it++) {
        if (std::find_if(
            this->mentions.begin(),
            this->mentions.end(),
            [it, &std_value](const auto& mention) {
                return mention.start == it->first - std_value.begin();
            }
        ) == this->mentions.end()) {
            user_mentions.push_back(MessageDraftMentionEntity{
                static_cast<size_t>(it->first - std_value.begin()), 
                it->str().length(),
                MessageDraftMentionTargetEntity::user(it->str())
            });
        }
    }

    std::vector<MessageDraftMentionEntity> channel_mentions;
    for (auto it = all_channel_mentions; it != regex_end; it++) {
        if (std::find_if(
            this->mentions.begin(),
            this->mentions.end(),
            [it, &std_value](const auto& mention) {
                return mention.start == it->first - std_value.begin();
            }
        ) == this->mentions.end()) {
            channel_mentions.push_back(MessageDraftMentionEntity{
                static_cast<size_t>(it->first - std_value.begin()),
                it->str().length(),
                MessageDraftMentionTargetEntity::channel(it->str())
            });
        }
    }

    std::vector<MessageDraftMentionEntity> all_mentions;
    all_mentions.reserve(user_mentions.size() + channel_mentions.size());
    all_mentions.insert(all_mentions.end(), user_mentions.begin(), user_mentions.end());
    all_mentions.insert(all_mentions.end(), channel_mentions.begin(), channel_mentions.end());

    std::sort(
        all_mentions.begin(),
        all_mentions.end(),
        [](const auto& a, const auto& b) {
            return a.start < b.start;
        }
    );

    return all_mentions;
}

std::vector<MessageDraftMessageElementEntity> MessageDraftEntity::get_message_elements() const {
    std::vector<MessageDraftMessageElementEntity> elements;
    std::size_t last_position = 0;

    for (const auto& mention : this->mentions) {
        auto plain_text = this->value.substring(last_position, mention.start - last_position);
        if (!plain_text.empty()) {
            elements.push_back(MessageDraftMessageElementEntity::plain_text(plain_text));
        }

        elements.push_back(MessageDraftMessageElementEntity::link(
            this->value.substring(mention.start, mention.length),
            mention.target
        ));

        last_position = mention.end_exclusive();
    }

    auto remaining_text = this->value.substring(last_position, this->value.length());
    if (!remaining_text.empty()) {
        elements.push_back(MessageDraftMessageElementEntity::plain_text(remaining_text));
    }

    return elements;
}

static Pubnub::String escape_link_text(const Pubnub::String& text) {
    auto result = text;
    result.replace_all("\\", "\\\\");
    result.replace_all("]", "\\]");

    return result;
}

static Pubnub::String escape_link_url(const Pubnub::String& url) {
    auto result = url;
    result.replace_all("\\", "\\\\");
    result.replace_all(")", "\\)");

    return result;
}

Pubnub::String MessageDraftEntity::render() const {
    auto elements = this->get_message_elements();

    return std::accumulate(
        elements.begin(),
        elements.end(),
        Pubnub::String(""),
        [](const Pubnub::String& acc, const MessageDraftMessageElementEntity& element) {
            if (!element.target.has_value()) {
                return acc + element.text;
            }

            auto target = element.target.value();
            
            switch (target.type) {
                case MessageDraftMentionTargetEntity::Type::USER:
                    return acc + "[" + escape_link_text(element.text) + "](" + schema_user + escape_link_url(target.target) + ")";
                case MessageDraftMentionTargetEntity::Type::CHANNEL:
                    return acc + "[" + escape_link_text(element.text) + "](" + schema_channel + escape_link_url(target.target) + ")";
                case MessageDraftMentionTargetEntity::Type::URL:
                    return acc + "[" + escape_link_text(element.text) + "](" + escape_link_url(target.target) + ")";
            }
        }
    );
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
    auto substring = this->value.substring(suggested_mention.position, suggested_mention.replace_from.length());

    if (nullptr == substring.c_str()) {
        return false;
    }

    return substring == suggested_mention.replace_from;
}
