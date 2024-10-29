#include "domain/parsers.hpp"
#include "cgreen/cgreen.h"
#include "cgreen/unit.h"
#include "enums.hpp"
#include "pubnub_helper.h"
#include "test_helpers.hpp"

using namespace cgreen;

Describe(Parser);
BeforeEach(Parser) {/* no setup needed */}
AfterEach(Parser) {/* no cleanup needed */}

Ensure(Parser, should_validate_if_json_contains_message) {
    auto message = Parsers::PubnubJson::is_message("{\"text\":\"Hello\", \"type\":\"text\"}");
    auto not_message = Parsers::PubnubJson::is_message("{\"text\":\"Hello\"}");

    assert_true(message);
    assert_false(not_message);
}

Ensure(Parser, should_validate_if_json_contains_message_update) {
    auto update = Parsers::PubnubJson::is_message_update("{\"source\":\"actions\", \"data\":\"<some_update>\"}");
    auto not_fully_update = Parsers::PubnubJson::is_message_update("{\"source\":\"actions\"}");
    auto unknown_source = Parsers::PubnubJson::is_message_update("{\"source\":\"unknown\", \"data\":\"<some_update>\"}");
    auto not_update = Parsers::PubnubJson::is_message_update("{\"text\":\"Hello\", \"type\":\"text\"}");

    assert_true(update);
    assert_false(not_fully_update);
    assert_false(unknown_source);
    assert_false(not_update);
}

Ensure(Parser, should_validate_if_json_contains_channel_update) {
    auto update = Parsers::PubnubJson::is_channel_update("{\"source\":\"objects\", \"type\":\"channel\", \"event\":\"<event_type>\"}");
    auto not_fully_update = Parsers::PubnubJson::is_channel_update("{\"source\":\"objects\", \"type\":\"<update_type>\"}");
    auto unknown_source = Parsers::PubnubJson::is_channel_update("{\"source\":\"unknown\", \"type\":\"<update_type>\", \"event\":\"<event_type>\"}");
    auto wrong_type = Parsers::PubnubJson::is_channel_update("{\"source\":\"objects\", \"type\":\"MESSAGE\", \"event\":\"<event_type>\"}");
    auto not_update = Parsers::PubnubJson::is_channel_update("{\"text\":\"Hello\", \"type\":\"text\"}");

    assert_true(update);
    assert_false(not_fully_update);
    assert_false(unknown_source);
    assert_false(wrong_type);
    assert_false(not_update);
}

Ensure(Parser, should_validate_if_json_contains_user_update) {
    auto update = Parsers::PubnubJson::is_user_update("{\"source\":\"objects\", \"type\":\"uuid\", \"event\":\"<event_type>\"}");
    auto not_fully_update = Parsers::PubnubJson::is_user_update("{\"source\":\"objects\", \"type\":\"<update_type>\"}");
    auto unknown_source = Parsers::PubnubJson::is_user_update("{\"source\":\"unknown\", \"type\":\"<update_type>\", \"event\":\"<event_type>\"}");
    auto wrong_type = Parsers::PubnubJson::is_user_update("{\"source\":\"objects\", \"type\":\"CHANNEL\", \"event\":\"<event_type>\"}");
    auto not_update = Parsers::PubnubJson::is_user_update("{\"text\":\"Hello\", \"type\":\"text\"}");

    assert_true(update);
    assert_false(not_fully_update);
    assert_false(unknown_source);
    assert_false(wrong_type);
    assert_false(not_update);
}

Ensure(Parser, should_validate_if_json_contains_membership_update) {
    auto update = Parsers::PubnubJson::is_membership_update("{\"source\":\"objects\", \"type\":\"membership\", \"event\":\"<event_type>\"}");
    auto not_fully_update = Parsers::PubnubJson::is_membership_update("{\"source\":\"objects\", \"type\":\"<update_type>\"}");
    auto unknown_source = Parsers::PubnubJson::is_membership_update("{\"source\":\"unknown\", \"type\":\"<update_type>\", \"event\":\"<event_type>\"}");
    auto wrong_type = Parsers::PubnubJson::is_membership_update("{\"source\":\"objects\", \"type\":\"CHANNEL\", \"event\":\"<event_type>\"}");
    auto not_update = Parsers::PubnubJson::is_membership_update("{\"text\":\"Hello\", \"type\":\"text\"}");

    assert_true(update);
    assert_false(not_fully_update);
    assert_false(unknown_source);
    assert_false(wrong_type);
    assert_false(not_update);
}

Ensure(Parser, should_validate_if_json_contains_event) {
    // TODO: it seems like a possibility of being too generic
    //       we should be more specific what is an event
    auto mention = Parsers::PubnubJson::is_event("{\"type\":\"mention\"}");
    auto typing = Parsers::PubnubJson::is_event("{\"type\":\"typing\"}");
    auto report = Parsers::PubnubJson::is_event("{\"type\":\"report\"}");
    auto receipt = Parsers::PubnubJson::is_event("{\"type\":\"receipt\"}");
    auto invite = Parsers::PubnubJson::is_event("{\"type\":\"invite\"}");
    auto custom = Parsers::PubnubJson::is_event("{\"type\":\"custom\"}");
    auto moderation = Parsers::PubnubJson::is_event("{\"type\":\"moderation\"}");

    auto not_event = Parsers::PubnubJson::is_event("{\"text\":\"Hello\", \"type\":\"text\"}");

    assert_true(mention);
    assert_true(typing);
    assert_true(report);
    assert_true(receipt);
    assert_true(invite);
    assert_true(custom);
    assert_true(moderation);

    assert_false(not_event);
}

Ensure(Parser, should_validate_if_json_contains_presence) {
    // TODO: presence doesn't require action validation
    auto presence = Parsers::PubnubJson::is_presence("{\"action\":\"<BUG>\", \"uuid\":\"<uuid>\"}");
    auto not_fully_presence = Parsers::PubnubJson::is_presence("{\"uuid\":\"<uuid>\"}");
    auto not_presence = Parsers::PubnubJson::is_presence("{\"text\":\"Hello\", \"type\":\"text\"}");

    assert_true(presence);
    assert_false(not_fully_presence);
    assert_false(not_presence);
}

Ensure(Parser, should_parse_pn_message_to_message) {
    pubnub_v2_message pn_message;
    pn_message.tt = {const_cast<char*>("12345678901234567"), 17};
    pn_message.payload = {const_cast<char*>("{\"text\":\"Hello\", \"type\":\"text\"}"), 31};
    pn_message.channel = {const_cast<char*>("channel"), 7};
    pn_message.publisher = {const_cast<char*>("publisher"), 9};
    pn_message.metadata = {const_cast<char*>("{\"metadata\":\"data\"}"), 19};

    auto message = Parsers::PubnubJson::to_message(pn_message);
    auto timetoken = message.first;
    auto entity = message.second;

    assert_string_equal(timetoken.c_str(), "12345678901234567");
    assert_equal(entity.type, Pubnub::pubnub_chat_message_type::PCMT_TEXT);
    assert_string_equal(entity.current_text().c_str(), "Hello");
    assert_string_equal(entity.channel_id.c_str(), "channel");
    assert_string_equal(entity.user_id.c_str(), "publisher");
    assert_string_equal(entity.meta.c_str(), "{\"metadata\":\"data\"}");
}

Ensure(Parser, should_parse_pn_message_to_message_update) {
    pubnub_v2_message pn_message;
    pn_message.tt = {const_cast<char*>("12345678901234567"), 17};
    pn_message.payload = {const_cast<char*>("{\"source\":\"actions\", \"data\":{\"type\":\"edited\", \"value\":\"NEW TEXT\", \"uuid\":\"publisher\", \"actionTimetoken\":\"99999999999999999\",\"messageTimetoken\":\"12345678901234567\"}}"), 164};
    pn_message.channel = {const_cast<char*>("channel"), 7};
    pn_message.publisher = {const_cast<char*>("publisher"), 9};
    pn_message.metadata = {const_cast<char*>("{\"metadata\":\"data\"}"), 19};

    auto message = Parsers::PubnubJson::to_message_update(pn_message);
    auto timetoken = message.first;
    auto entity = message.second;

    assert_string_equal(timetoken.c_str(), "12345678901234567");
    assert_string_equal(entity.current_text().c_str(), "NEW TEXT");
    assert_string_equal(entity.channel_id.c_str(), "channel");
    assert_string_equal(entity.user_id.c_str(), "publisher");
    assert_string_equal(entity.meta.c_str(), "{\"metadata\":\"data\"}");
    assert_string_equal(entity.message_actions[0].timetoken.c_str(), "99999999999999999");
    assert_string_equal(entity.message_actions[0].user_id.c_str(), "publisher");
    assert_that(entity.message_actions[0].type, is_equal_to(Pubnub::pubnub_message_action_type::PMAT_Edited));
}

Ensure(Parser, should_parse_pn_message_to_channel) {
    pubnub_v2_message pn_message;
    pn_message.tt = {const_cast<char*>("12345678901234567"), 17};
    pn_message.payload = {const_cast<char*>("{\"data\": {\"id\":\"channel\",\"name\":\"name\",\"description\":\"description\",\"custom\":{\"custom\":\"data\"},\"updated\":\"updated\",\"status\":\"status\",\"type\":\"type\"}}"), 163};
    pn_message.channel = {const_cast<char*>("channel"), 6};
    pn_message.publisher = {const_cast<char*>("publisher"), 9};
    pn_message.metadata = {const_cast<char*>("{\"metadata\":\"data\"}"), 19};

    auto channel = Parsers::PubnubJson::to_channel(pn_message);

    auto channel_id = channel.first;
    auto entity = channel.second;

    assert_string_equal(channel_id.c_str(), "channel");
    assert_string_equal(entity.channel_name.c_str(), "name");
    assert_string_equal(entity.description.c_str(), "description");
    assert_string_equal(entity.custom_data_json.c_str(), "{\"custom\":\"data\"}");
    assert_string_equal(entity.updated.c_str(), "updated");
    assert_string_equal(entity.status.c_str(), "status");
    assert_string_equal(entity.type.c_str(), "type");
}

Ensure(Parser, should_parse_pn_message_to_user) {
    pubnub_v2_message pn_message;
    pn_message.tt = {const_cast<char*>("12345678901234567"), 17};
    pn_message.payload = {const_cast<char*>("{\"data\": {\"id\":\"user\",\"name\":\"name\",\"externalId\":\"id\",\"profileUrl\":\"profile\",\"email\":\"email\",\"custom\":{\"custom\":\"data\"},\"status\":\"status\",\"type\":\"type\"}}"), 163};
    pn_message.channel = {const_cast<char*>("channel"), 6};
    pn_message.publisher = {const_cast<char*>("publisher"), 9};
    pn_message.metadata = {const_cast<char*>("{\"metadata\":\"data\"}"), 19};

    auto user = Parsers::PubnubJson::to_user(pn_message);

    auto user_id = user.first;
    auto entity = user.second;

    assert_string_equal(user_id.c_str(), "user");
    assert_string_equal(entity.user_name.c_str(), "name");
    assert_string_equal(entity.external_id.c_str(), "id");
    assert_string_equal(entity.profile_url.c_str(), "profile");
    assert_string_equal(entity.email.c_str(), "email");
    assert_string_equal(entity.custom_data_json.c_str(), "{\"custom\":\"data\"}");
    assert_string_equal(entity.status.c_str(), "status");
    assert_string_equal(entity.type.c_str(), "type");
}

Ensure(Parser, should_parse_membership_from_string) {
    auto membership = Parsers::PubnubJson::membership_from_string("{\"data\":{\"custom\":{\"custom\":\"data\"}}}");

    assert_string_equal(membership.custom_field.c_str(), "{\"custom\":\"data\"}");
}

Ensure(Parser, should_parse_payload_from_pn_message) {
    pubnub_v2_message pn_message;
    pn_message.payload = {const_cast<char*>("{\"text\":\"Hello\", \"type\":\"text\"}"), 31};

    auto payload = Parsers::PubnubJson::to_string(pn_message);

    assert_string_equal(payload.c_str(), "{\"text\":\"Hello\", \"type\":\"text\"}");
}

Ensure(Parser, should_parse_pn_message_to_event) {
    pubnub_v2_message pn_message;
    pn_message.tt = {const_cast<char*>("12345678901234567"), 17};
    pn_message.payload = {const_cast<char*>("{\"type\":\"mention\"}"), 18};
    pn_message.channel = {const_cast<char*>("channel"), 7};
    pn_message.publisher = {const_cast<char*>("publisher"), 9};
    pn_message.metadata = {const_cast<char*>("{\"metadata\":\"data\"}"), 19};

    auto event = Parsers::PubnubJson::to_event(pn_message);

    assert_string_equal(event.timetoken.c_str(), "12345678901234567");
    assert_that(event.type, is_equal_to(Pubnub::pubnub_chat_event_type::PCET_MENTION));
    assert_string_equal(event.channel_id.c_str(), "channel");
    assert_string_equal(event.user_id.c_str(), "publisher");
    assert_string_equal(event.payload.c_str(), "{\"type\":\"mention\"}");
}

Ensure(Parser, should_parse_pn_message_to_events_type) {
    pubnub_v2_message pn_message;
    pn_message.payload = {const_cast<char*>("{\"type\":\"mention\"}"), 18};

    auto event_type = Parsers::PubnubJson::event_type("{\"type\":\"mention\"}");

    assert_string_equal(event_type.c_str(), "mention");
}

Ensure(Parser, should_parse_message_update_timetoken) {
    auto timetoken = Parsers::PubnubJson::message_update_timetoken("{\"data\":{\"messageTimetoken\":\"12345678901234567\"}}");

    assert_string_equal(timetoken.c_str(), "12345678901234567");
}

Ensure(Parser, should_parse_membership_channel) {
    auto channel_id = Parsers::PubnubJson::membership_channel("{\"data\":{\"channel\":{\"id\":\"channel\"}}}");

    assert_string_equal(channel_id.c_str(), "channel");
}

Ensure(Parser, should_parse_membership_user) {
    auto user_id = Parsers::PubnubJson::membership_user("{\"data\":{\"uuid\":{\"id\":\"user\"}}}");

    assert_string_equal(user_id.c_str(), "user");
}

Ensure(Parser, should_parse_membership_custom_field) {
    auto custom_field = Parsers::PubnubJson::membership_custom_field("{\"custom\":\"data\"}");

    assert_string_equal(custom_field.c_str(), "data");
}

Ensure(Parser, should_validate_if_json_contains_parent_message) {
    auto parent_message = Parsers::PubnubJson::contains_parent_message("{\"data\":{\"parentMessage\":\"12345678901234567\"}}");
    auto not_parent_message = Parsers::PubnubJson::contains_parent_message("{\"text\":\"Hello\", \"type\":\"text\"}");

    assert_true(parent_message);
    assert_false(not_parent_message);
}

