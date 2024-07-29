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
# TODO: switch back to master branch
FetchContent_Declare(
  pubnub
  GIT_REPOSITORY https://github.com/pubnub/c-core.git
  GIT_TAG feat/new-objects-vol2
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra/pubnub.cpp)

set(DOMAIN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/chat_entity.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/channel_entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/user_entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/message_entity.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/membership_entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/parsers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/json.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/presence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/access_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/typing.cpp)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/chat_helpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/string.cpp
)

if(${ENABLE_C_ABI})
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-D PN_CHAT_C_ABI")
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
        ${CMAKE_CURRENT_SOURCE_DIR}/c_abi/src/c_serialization.cpp)
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

    find_package(cgreen REQUIRED)
    enable_testing()

    include_directories(${CGREEN_INCLUDE_DIRS})

    add_library(pubnub-test-string SHARED test/string_unit.c)
    target_link_libraries(pubnub-test-string PRIVATE pubnub-chat ${CGREEN_LIBRARIES})

    add_test(NAME pubnub-test-string COMMAND ${CGREEN_EXECUTABLE} pubnub-test-string)
endif()
