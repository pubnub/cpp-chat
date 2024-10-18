#include "domain/channel_entity.hpp"
#include "cgreen/cgreen.h"
#include "cgreen/unit.h"
#include "test_helpers.hpp"
#include <iostream>

using namespace cgreen;

Describe(ChannelEntities);
BeforeEach(ChannelEntities) {/* no setup needed */}
AfterEach(ChannelEntities) {/* no cleanup needed */}

Ensure(ChannelEntities, should_return_json_with_metadata) {
    ChannelEntity sut{
        "name",
        "description",
        "{\"custom\":\"data\"}",
        "updated",
        "status",
        "type",
    };

    auto result = trim_whitespaces(sut.get_channel_metadata_json_string("channel_id"));

    assert_true(result.find("\"id\":\"channel_id\"") != std::string::npos);
    assert_true(result.find("\"name\":\"name\"") != std::string::npos);
    assert_true(result.find("\"description\":\"description\"") != std::string::npos);
    assert_true(result.find("\"custom\":{\"custom\":\"data\"}") != std::string::npos);
    assert_true(result.find("\"updated\":\"updated\"") != std::string::npos);
    assert_true(result.find("\"status\":\"status\"") != std::string::npos);
    assert_true(result.find("\"type\":\"type\"") != std::string::npos);
}

Ensure(ChannelEntities, should_return_json_with_only_provided_data) {
    ChannelEntity sut;
    sut.channel_name = "name";

    auto result = trim_whitespaces(sut.get_channel_metadata_json_string("channel_id"));

    assert_true(result.find("\"id\":\"channel_id\"") != std::string::npos);
    assert_true(result.find("\"name\":\"name\"") != std::string::npos);
    assert_false(result.find("\"description\"") != std::string::npos);
    assert_false(result.find("\"custom\"") != std::string::npos);
    assert_false(result.find("\"updated\"") != std::string::npos);
    assert_false(result.find("\"status\"") != std::string::npos);
    assert_false(result.find("\"type\"") != std::string::npos);
}

Ensure(ChannelEntities, should_return_json_with_empty_fields) {
    ChannelEntity sut;

    auto result = trim_whitespaces(sut.get_channel_metadata_json_string("channel_id"));

    assert_true(result.find("\"id\":\"channel_id\"") != std::string::npos);
    assert_false(result.find("\"name\"") != std::string::npos);
    assert_false(result.find("\"description\"") != std::string::npos);
    assert_false(result.find("\"custom\"") != std::string::npos);
    assert_false(result.find("\"updated\"") != std::string::npos);
    assert_false(result.find("\"status\"") != std::string::npos);
    assert_false(result.find("\"type\"") != std::string::npos);
}

Ensure(ChannelEntities, should_handle_pinning_messages) {
    ChannelEntity sut;

    auto pinned_sut = sut.pin_message({"channel", "<timetoken>"});

    auto pinning_result = trim_whitespaces(pinned_sut.custom_data_json);
    assert_true(pinning_result.find("\"pinnedMessageChannelID\":\"channel\"") != std::string::npos);
    assert_true(pinning_result.find("\"pinnedMessageTimetoken\":\"<timetoken>\"") != std::string::npos);

    auto unpinned_sut = sut.unpin_message();

    auto unpinning_result = trim_whitespaces(unpinned_sut.custom_data_json);
    assert_false(unpinning_result.find("\"pinnedMessageChannelID\"") != std::string::npos);
    assert_false(unpinning_result.find("\"pinnedMessageTimetoken\"") != std::string::npos);
}

Ensure(ChannelEntities, should_be_deserialized_from_json) {
    Json json = Json::parse("{\"id\":\"channel_id\",\"name\":\"name\",\"description\":\"description\",\"custom\":{\"custom\":\"data\"},\"updated\":\"updated\",\"status\":\"status\",\"type\":\"type\"}");

    auto sut = ChannelEntity::from_json(json);

    assert_string_equal(sut.channel_name.c_str(), "name");
    assert_string_equal(sut.description.c_str(), "description");
    assert_string_equal(sut.custom_data_json.c_str(), "{\"custom\":\"data\"}");
    assert_string_equal(sut.updated.c_str(), "updated");
    assert_string_equal(sut.status.c_str(), "status");
    assert_string_equal(sut.type.c_str(), "type");
}

Ensure(ChannelEntities, should_be_deserialized_from_response) {
    Json json = Json::parse("{\"name\":\"name\",\"description\":\"description\",\"custom\":{\"custom\":\"data\"},\"updated\":\"updated\",\"status\":\"status\",\"type\":\"type\"}");

    auto sut = ChannelEntity::from_channel_response(json);

    assert_string_equal(sut.channel_name.c_str(), "name");
    assert_string_equal(sut.description.c_str(), "description");
    assert_string_equal(sut.custom_data_json.c_str(), "{\"custom\":\"data\"}");
    assert_string_equal(sut.updated.c_str(), "updated");
    assert_string_equal(sut.status.c_str(), "status");
    assert_string_equal(sut.type.c_str(), "type");
}

Ensure(ChannelEntities, should_update) {
    ChannelEntity base;
    base.channel_name = "name";
    base.description = "description";

    ChannelEntity updated;
    updated.channel_name = "new_name";
    updated.updated = "updated";

    auto sut = ChannelEntity::from_base_and_updated_channel(base, updated);

    assert_string_equal(sut.channel_name.c_str(), "new_name");
    assert_string_equal(sut.description.c_str(), "description");
    assert_string_equal(sut.updated.c_str(), "updated");
}

