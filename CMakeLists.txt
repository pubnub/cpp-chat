# CMakeLists.txt for the C-core library of Pubnub.
cmake_minimum_required(VERSION 3.12)

project(pubnub-chat VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(${ENABLE_ASAN})
    message(STATUS "Enabling AddressSanitizer")
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-fsanitize=address")
endif()

option(COMPILE_COMMANDS "Generate compile_commands.json" OFF)
if(${COMPILE_COMMANDS})
    message(STATUS "Generating compile_commands.json")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
endif()

option(DEBUG "Enable debugging" OFF)
if(${DEBUG})
    message(STATUS "Enabling debugging")
    set(CMAKE_BUILD_TYPE Debug)
endif()

option(ENABLE_C_ABI "Enable C ABI" OFF)
if(${ENABLE_C_ABI})
    message(STATUS "Enabling C ABI functions")
endif()

option(RUN_TESTS "Run tests" OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/pubnub_chat)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src) # not public imports

include(FetchContent)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(USE_GRANT_TOKEN_API ON)

if(WIN32 OR WIN64 OR MSVC)
    set(CUSTOM_BOOL_TYPE _Bool)
endif()

# TODO: pnsdk should be different for each SDK (C++, EU, Unity, etc.)
set(PUBNUB_SDK_VERSION_SUFFIX "/CA-SDKC++/${PROJECT_VERSION}")

FetchContent_Declare(
  pubnub
  GIT_REPOSITORY https://github.com/pubnub/c-core.git
  GIT_TAG master
  GIT_SHALLOW TRUE
  GIT_PROGRESS ON
  SYSTEM
)

FetchContent_MakeAvailable(pubnub)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
  GIT_SHALLOW TRUE
  GIT_PROGRESS ON
  SYSTEM
)

FetchContent_MakeAvailable(json)

include_directories(${pubnub_SOURCE_DIR}/posix)
include_directories(${pubnub_SOURCE_DIR}/core)
include_directories(${pubnub_SOURCE_DIR})

include_directories(${json_SOURCE_DIR})
include_directories(${json_SOURCE_DIR}/include)

include_directories(${CURRENT_SOURCE_DIR}/include)

set(INFRA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra/entity_repository.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra/callbacks_repository.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra/serialization.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra/timer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra/pubnub.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra/interval_task.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra/rate_limiter.cpp)

set(DOMAIN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/chat_entity.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/channel_entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/user_entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/message_entity.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/membership_entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/parsers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/json.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/presence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/access_manager_logic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/typing.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/timetoken.cpp)

set(APP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/chat_service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/channel_service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/user_service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/message_service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/presence_service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/restrictions_service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/membership_service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/callback_service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/access_manager_service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/dao/channel_dao.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/dao/user_dao.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/dao/message_dao.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/dao/membership_dao.cpp)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/presentation/chat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/presentation/channel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/presentation/user.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/presentation/message.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/presentation/message_draft.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/presentation/membership.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/presentation/thread_channel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/presentation/thread_message.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/presentation/access_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/chat_helpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/string.cpp
)

if(${ENABLE_C_ABI})
    add_compile_definitions(PN_CHAT_C_ABI)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/c_abi/include)
    set(SOURCES
        ${SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/c_abi/src/c_chat.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/c_abi/src/c_channel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/c_abi/src/c_user.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/c_abi/src/c_errors.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/c_abi/src/c_message.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/c_abi/src/c_message_draft.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/c_abi/src/c_membership.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/c_abi/src/c_serialization.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/c_abi/src/c_thread_channel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/c_abi/src/c_thread_message.cpp)
endif()

set(SOURCES
    ${SOURCES}
    ${INFRA_SOURCES}
    ${DOMAIN_SOURCES}
    ${APP_SOURCES})

add_library(pubnub-chat SHARED ${SOURCES})

target_link_libraries(pubnub-chat PRIVATE pubnub)

add_executable(pubnub-chat-example example/main.cpp)

target_link_libraries(pubnub-chat-example PUBLIC pubnub-chat)

if(${RUN_TESTS})
    message(STATUS "Running tests")
    enable_testing()

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/test)

    # TODO: use installed runner instead (at least for MacOS there is an issue with RPATH)
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cgreen)
        message(FATAL_ERROR "CGreen not found! Clone the repository and build it inside the root of the project!")
    endif()

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cgreen/include)

    function(test_suite name source)
        add_library(${name} SHARED ${source})
        target_link_libraries(${name} PRIVATE pubnub-chat ${CMAKE_CURRENT_SOURCE_DIR}/cgreen/build/src/libcgreen.dylib)

        add_test(NAME ${name} COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/cgreen/build/tools/cgreen-runner ${CMAKE_CURRENT_SOURCE_DIR}/build/lib${name}.dylib)
    endfunction()

    test_suite(string-unit ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/string_unit.cpp)
    test_suite(json-unit ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/json_unit.cpp)
    test_suite(typing-unit ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/typing_unit.cpp)
    test_suite(presence-unit ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/presence_unit.cpp)
endif()
