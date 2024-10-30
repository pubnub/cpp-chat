#include "domain/message_draft_entity.hpp"
#include "cgreen/cgreen.h"
#include "cgreen/unit.h"

using namespace cgreen;

Describe(MessageDraftEntities);
BeforeEach(MessageDraftEntities) {/* no setup needed */}
AfterEach(MessageDraftEntities) {/* no cleanup needed */}

Ensure(MessageDraftEntities, should_insert_plain_text) {
    MessageDraftEntity sut;

    auto result = sut.insert_text(0, "text><text")
        .insert_text(5, "text");

    assert_string_equal(result.value.c_str(), "text>text<text");
}

Ensure(MessageDraftEntities, should_append_plain_text) {
    MessageDraftEntity sut;

    auto result = sut.insert_text(0, "Hello")
        .insert_text(5, " World");

    assert_string_equal(result.value.c_str(), "Hello World");
}

Ensure(MessageDraftEntities, should_remove_plain_text) {
    MessageDraftEntity sut;

    auto result = sut.insert_text(0, "Hello World")
        .remove_text(5, 6);

    assert_string_equal(result.value.c_str(), "Hello");
}

Ensure(MessageDraftEntities, should_move_mention_properly_on_insert) {
    MessageDraftEntity sut;
    sut.value = "Hello @user1 !";
    sut.mentions = {{6, 6, {"user1", MessageDraftMentionTargetEntity::Type::USER}}};

    auto result = sut.insert_text(6, "my dear ");

    assert_string_equal(result.value.c_str(), "Hello my dear @user1 !");
    assert_that(result.mentions[0].start, is_equal_to(14));
    assert_that(result.mentions[0].length, is_equal_to(6));
}

Ensure(MessageDraftEntities, should_remove_mention_when_mention_has_been_changed) {
    MessageDraftEntity sut;
    sut.value = "Hello @user1 !";
    sut.mentions = {{6, 6, {"user1", MessageDraftMentionTargetEntity::Type::USER}}};

    auto result = sut.insert_text(8, "XXX");

    assert_string_equal(result.value.c_str(), "Hello @uXXXser1 !");
    assert_that(result.mentions.size(), is_equal_to(0));
}

Ensure(MessageDraftEntities, should_handle_insert_between_two_mentions) {
    MessageDraftEntity sut;
    sut.value = "Hello @user1 @user2 !";
    sut.mentions = {
        {6, 6, {"user1", MessageDraftMentionTargetEntity::Type::USER}},
        {13, 6, {"user2", MessageDraftMentionTargetEntity::Type::USER}}
    };

    auto result = sut.insert_text(12, " and");

    assert_string_equal(result.value.c_str(), "Hello @user1 and @user2 !");
    assert_that(result.mentions[0].start, is_equal_to(6));
    assert_that(result.mentions[0].length, is_equal_to(6));
    assert_that(result.mentions[1].start, is_equal_to(17));
    assert_that(result.mentions[1].length, is_equal_to(6));
}

Ensure(MessageDraftEntities, should_remove_mention_when_mention_is_removed) {
    MessageDraftEntity sut;
    sut.value = "Hello @user1 !";
    sut.mentions = {{6, 6, {"user1", MessageDraftMentionTargetEntity::Type::USER}}};

    auto result = sut.remove_text(6, 7);

    assert_string_equal(result.value.c_str(), "Hello !");
    assert_that(result.mentions.size(), is_equal_to(0));
}

Ensure(MessageDraftEntities, should_not_remove_mentions_when_they_are_not_changed) {
    MessageDraftEntity sut;
    sut.value = "Hello @user1 !";
    sut.mentions = {{6, 6, {"user1", MessageDraftMentionTargetEntity::Type::USER}}};

    auto result = sut.remove_text(0, 5);

    assert_string_equal(result.value.c_str(), " @user1 !");
    assert_that(result.mentions.size(), is_equal_to(1));
    assert_that(result.mentions[0].start, is_equal_to(1));
    assert_that(result.mentions[0].length, is_equal_to(6));
}

Ensure(MessageDraftEntities, should_remove_mentions_only_when_they_are_removed) {
    MessageDraftEntity sut;
    sut.value = "Hello @user1 @user2 !";
    sut.mentions = {
        {6, 6, {"user1", MessageDraftMentionTargetEntity::Type::USER}},
        {13, 6, {"user2", MessageDraftMentionTargetEntity::Type::USER}}
    };

    auto result = sut.remove_text(6, 7);

    assert_string_equal(result.value.c_str(), "Hello @user2 !");
    assert_that(result.mentions.size(), is_equal_to(1));
    assert_that(result.mentions[0].start, is_equal_to(6));
    assert_that(result.mentions[0].length, is_equal_to(6));
}

Ensure(MessageDraftEntities, should_add_mentions_directly) {
    MessageDraftEntity sut;

    auto result = sut.add_mention(0, 6, {"user1", MessageDraftMentionTargetEntity::Type::USER});

    assert_that(result.mentions.size(), is_equal_to(1));
    assert_that(result.mentions[0].start, is_equal_to(0));
    assert_that(result.mentions[0].length, is_equal_to(6));
    assert_string_equal(result.mentions[0].target.target, "user1");
}

Ensure(MessageDraftEntities, should_remove_mentions_directlu) {
    MessageDraftEntity sut;
    sut.mentions = {{0, 6, {"user1", MessageDraftMentionTargetEntity::Type::USER}}};

    auto result = sut.remove_mention(0);

    assert_that(result.mentions.size(), is_equal_to(0));
}

Ensure(MessageDraftEntities, should_validate_their_mentions_for_intersections) {
    MessageDraftEntity sut;

    assert_that(sut.validate_mentions(), is_true);

    sut = sut.add_mention(0, 6, {"user1", MessageDraftMentionTargetEntity::Type::USER});
    assert_that(sut.validate_mentions(), is_true);

    sut = sut.add_mention(10, 6, {"user2", MessageDraftMentionTargetEntity::Type::USER});
    assert_that(sut.validate_mentions(), is_true);

    sut = sut.add_mention(5, 6, {"user3", MessageDraftMentionTargetEntity::Type::USER});
    assert_that(sut.validate_mentions(), is_false);
}

Ensure(MessageDraftEntities, should_insert_suggested_mention) {
    MessageDraftEntity sut;
    sut.value = "Hello @user1 !";
    sut.mentions = {{6, 6, {"user1", MessageDraftMentionTargetEntity::Type::USER}}};

    auto result = sut.insert_suggested_mention({6, "@user1", "@some_uuid", {"user1", MessageDraftMentionTargetEntity::Type::USER}}, "MyCloseFriend");

    assert_string_equal(result.value.c_str(), "Hello MyCloseFriend !");
    assert_that(result.mentions.size(), is_equal_to(1));
    assert_that(result.mentions[0].start, is_equal_to(6));
    assert_that(result.mentions[0].length, is_equal_to(13));
    assert_string_equal(result.mentions[0].target.target.c_str(), "user1");
}

Ensure(MessageDraftEntities, should_validate_suggested_mention) {
    MessageDraftEntity sut;
    sut.value = "Hello @user1 !";

    assert_that(sut.validate_suggested_mention({6, "@user1", "@some_uuid", {"user1", MessageDraftMentionTargetEntity::Type::USER}}), is_true);
    assert_that(sut.validate_suggested_mention({6, "@user2", "@some_uuid", {"user2", MessageDraftMentionTargetEntity::Type::USER}}), is_false);
}

Ensure(MessageDraftEntities, should_properly_update_text) {
    MessageDraftEntity sut;
    sut.value = "Hello @user1 and @user2, I hope you are doing well. I wanted to tell you that I am going to be in town next week. Let's meet for a coffee! hello @user0 !";

    auto result = sut.update("Hello my good friend @user1 and my darling @user3, I wanted to inform you that I am going to be in town next week. Let's meet for a good coffee! @user0 hello!");

    assert_string_equal(result.value.c_str(), "Hello my good friend @user1 and my darling @user3, I wanted to inform you that I am going to be in town next week. Let's meet for a good coffee! @user0 hello!");
}

Ensure(MessageDraftEntities, should_suggest_mentions) {
    MessageDraftEntity sut;
    sut.value = "@Hello @user1 @user2 on #channel1 !";
    sut.mentions = {
        {14, 6, {"user2", MessageDraftMentionTargetEntity::Type::USER}},
    };

    auto result = sut.suggest_raw_mentions();

    assert_that(result.size(), is_equal_to(3));

    assert_string_equal(result[0].target.target.c_str(), "@Hello");
    assert_that(result[0].start, is_equal_to(0));
    assert_that(result[0].length, is_equal_to(6));
    assert_that(result[0].target.type, is_equal_to(MessageDraftMentionTargetEntity::Type::USER));

    assert_string_equal(result[1].target.target.c_str(), "@user1");
    assert_that(result[1].start, is_equal_to(7));
    assert_that(result[1].length, is_equal_to(6));
    assert_that(result[1].target.type, is_equal_to(MessageDraftMentionTargetEntity::Type::USER));

    assert_string_equal(result[2].target.target.c_str(), "#channel1");
    assert_that(result[2].start, is_equal_to(24));
    assert_that(result[2].length, is_equal_to(9));
    assert_that(result[2].target.type, is_equal_to(MessageDraftMentionTargetEntity::Type::CHANNEL));
}

