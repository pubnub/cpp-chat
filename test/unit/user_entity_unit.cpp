#include "domain/timetoken.hpp"
#include "domain/user_entity.hpp"
#include "cgreen/cgreen.h"
#include "cgreen/unit.h"
#include "test_helpers.hpp"

using namespace cgreen;

Describe(UserEntities);
BeforeEach(UserEntities) {/* no setup needed */}
AfterEach(UserEntities) {/* no cleanup needed */}

Ensure(UserEntities, should_return_json_with_metadata) {
    UserEntity sut{
        "name",
        "id",
        "profile",
        "email",
        "{\"custom\":\"data\"}",
        "status",
        "type"
    };

    auto result = trim_whitespaces(sut.get_user_metadata_json_string("user_id"));

    assert_true(result.find("\"id\":\"user_id\"") != std::string::npos);
    assert_true(result.find("\"name\":\"name\"") != std::string::npos);
    assert_true(result.find("\"externalId\":\"id\"") != std::string::npos);
    assert_true(result.find("\"profileUrl\":\"profile\"") != std::string::npos);
    assert_true(result.find("\"custom\":{\"custom\":\"data\"}") != std::string::npos);
    assert_true(result.find("\"email\":\"email\"") != std::string::npos);
    assert_true(result.find("\"status\":\"status\"") != std::string::npos);
    assert_true(result.find("\"type\":\"type\"") != std::string::npos);
}

Ensure(UserEntities, should_return_json_with_only_provided_data) {
    UserEntity sut;
    sut.user_name = "name";

    auto result = trim_whitespaces(sut.get_user_metadata_json_string("user_id"));

    assert_true(result.find("\"id\":\"user_id\"") != std::string::npos);
    assert_true(result.find("\"name\":\"name\"") != std::string::npos);
    assert_false(result.find("\"externalId\"") != std::string::npos);
    assert_false(result.find("\"profileUrl\"") != std::string::npos);
    assert_false(result.find("\"custom\"") != std::string::npos);
    assert_false(result.find("\"email\"") != std::string::npos);
    assert_false(result.find("\"status\"") != std::string::npos);
    assert_false(result.find("\"type\"") != std::string::npos);
}

Ensure(UserEntities, should_handle_users_activity) {
    UserEntity sut;

    assert_false(sut.is_active(1000));
    assert_false(sut.get_last_active_timestamp().has_value());

    // TODO: mock time
    auto now = Timetoken::now();
    sut.set_last_active_timestamp(now);

    assert_false(sut.is_active(0));
    assert_true(sut.is_active(999999));
    assert_true(sut.get_last_active_timestamp().has_value());
    assert_string_equal(sut.get_last_active_timestamp().value().c_str(), now.c_str());
}

Ensure(UserEntities, should_deserialized_from_json) {
    Json json = Json::parse("{\"id\":\"user_id\",\"name\":\"name\",\"externalId\":\"id\",\"profileUrl\":\"profile\",\"custom\":{\"custom\":\"data\"},\"email\":\"email\",\"status\":\"status\",\"type\":\"type\"}");

    auto sut = UserEntity::from_json(json);

    assert_string_equal(sut.user_name.c_str(), "name");
    assert_string_equal(sut.external_id.c_str(), "id");
    assert_string_equal(sut.profile_url.c_str(), "profile");
    assert_string_equal(sut.custom_data_json.c_str(), "{\"custom\":\"data\"}");
    assert_string_equal(sut.email.c_str(), "email");
    assert_string_equal(sut.status.c_str(), "status");
    assert_string_equal(sut.type.c_str(), "type");
}

Ensure(UserEntities, should_deserialized_from_json_with_missing_fields) {
    Json json = Json::parse("{\"id\":\"user_id\",\"name\":\"name\"}");

    auto sut = UserEntity::from_json(json);

    assert_string_equal(sut.user_name.c_str(), "name");
    assert_string_equal(sut.external_id.c_str(), "");
    assert_string_equal(sut.profile_url.c_str(), "");
    assert_string_equal(sut.custom_data_json.c_str(), "");
    assert_string_equal(sut.email.c_str(), "");
    assert_string_equal(sut.status.c_str(), "");
    assert_string_equal(sut.type.c_str(), "");
}

Ensure(UserEntities, should_deserialized_from_json_with_empty_fields) {
    Json json = Json::parse("{}");

    auto sut = UserEntity::from_json(json);

    assert_string_equal(sut.user_name.c_str(), "");
    assert_string_equal(sut.external_id.c_str(), "");
    assert_string_equal(sut.profile_url.c_str(), "");
    assert_string_equal(sut.custom_data_json.c_str(), "");
    assert_string_equal(sut.email.c_str(), "");
    assert_string_equal(sut.status.c_str(), "");
    assert_string_equal(sut.type.c_str(), "");
}

Ensure(UserEntities, should_deserialized_from_user_response) {
    Json json = Json::parse("{\"name\":\"name\",\"externalId\":\"id\",\"profileUrl\":\"profile\",\"email\":\"email\",\"custom\":{\"custom\":\"data\"},\"status\":\"status\",\"type\":\"type\"}");

    auto sut = UserEntity::from_user_response(json);

    assert_string_equal(sut.user_name.c_str(), "name");
    assert_string_equal(sut.external_id.c_str(), "id");
    assert_string_equal(sut.profile_url.c_str(), "profile");
    assert_string_equal(sut.custom_data_json.c_str(), "{\"custom\":\"data\"}");
    assert_string_equal(sut.email.c_str(), "email");
    assert_string_equal(sut.status.c_str(), "status");
    assert_string_equal(sut.type.c_str(), "type");
}

Ensure(UserEntities, should_deserialized_from_user_response_with_missing_fields) {
    Json json = Json::parse("{\"name\":\"name\"}");

    auto sut = UserEntity::from_user_response(json);

    assert_string_equal(sut.user_name.c_str(), "name");
    assert_string_equal(sut.external_id.c_str(), "");
    assert_string_equal(sut.profile_url.c_str(), "");
    assert_string_equal(sut.custom_data_json.c_str(), "");
    assert_string_equal(sut.email.c_str(), "");
    assert_string_equal(sut.status.c_str(), "");
    assert_string_equal(sut.type.c_str(), "");
}

Ensure(UserEntities, should_deserialized_from_user_response_with_empty_fields) {
    Json json = Json::parse("{}");

    auto sut = UserEntity::from_user_response(json);

    assert_string_equal(sut.user_name.c_str(), "");
    assert_string_equal(sut.external_id.c_str(), "");
    assert_string_equal(sut.profile_url.c_str(), "");
    assert_string_equal(sut.custom_data_json.c_str(), "");
    assert_string_equal(sut.email.c_str(), "");
    assert_string_equal(sut.status.c_str(), "");
    assert_string_equal(sut.type.c_str(), "");
}

Ensure(UserEntities, should_deserialized_from_user_list_resposne) {
    Json json = Json::parse("{\"data\":[{\"id\":\"user_id\",\"name\":\"name\",\"externalId\":\"id\",\"profileUrl\":\"profile\",\"custom\":{\"custom\":\"data\"},\"email\":\"email\",\"status\":\"status\",\"type\":\"type\"}]}");

    auto sut = UserEntity::from_user_list_response(json);

    assert_that(sut.size(), is_equal_to(1));
    assert_string_equal(sut[0].second.user_name.c_str(), "name");
    assert_string_equal(sut[0].second.external_id.c_str(), "id");
    assert_string_equal(sut[0].second.profile_url.c_str(), "profile");
    assert_string_equal(sut[0].second.custom_data_json.c_str(), "{\"custom\":\"data\"}");
    assert_string_equal(sut[0].second.email.c_str(), "email");
    assert_string_equal(sut[0].second.status.c_str(), "status");
    assert_string_equal(sut[0].second.type.c_str(), "type");
}

Ensure(UserEntities, should_deserialized_from_user_list_resposne_with_missing_fields) {
    Json json = Json::parse("{\"data\":[{\"id\":\"user_id\",\"name\":\"name\"}]}");

    auto sut = UserEntity::from_user_list_response(json);

    assert_that(sut.size(), is_equal_to(1));
    assert_string_equal(sut[0].second.user_name.c_str(), "name");
    assert_string_equal(sut[0].second.external_id.c_str(), "");
    assert_string_equal(sut[0].second.profile_url.c_str(), "");
    assert_string_equal(sut[0].second.custom_data_json.c_str(), "");
    assert_string_equal(sut[0].second.email.c_str(), "");
    assert_string_equal(sut[0].second.status.c_str(), "");
    assert_string_equal(sut[0].second.type.c_str(), "");
}

Ensure(UserEntities, should_deserialized_from_user_list_resposne_with_empty) {
    Json json = Json::parse("{\"data\":[]}");

    auto sut = UserEntity::from_user_list_response(json);

    assert_that(sut.size(), is_equal_to(0));
}

Ensure(UserEntities, should_update) {
    UserEntity original;
    original.user_name = "name";
    original.external_id = "id";

    UserEntity updated;
    updated.user_name = "new name";
    updated.email = "email";

    auto sut = UserEntity::from_base_and_updated_user(original, updated);

    assert_string_equal(sut.user_name.c_str(), "new name");
    assert_string_equal(sut.external_id.c_str(), "id");
    assert_string_equal(sut.email.c_str(), "email");
}
