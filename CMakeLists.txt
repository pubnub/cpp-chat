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

#If bebug symbols should be created (mostly related to release configuration)
option(CREATE_DEBUG_SYMBOLS "Enable debugging" OFF)
option(DEBUG "Enable debugging" OFF)
if(${DEBUG})
    message(STATUS "Enabling debugging")
    set(CMAKE_BUILD_TYPE Debug)
else()
    set(CMAKE_BUILD_TYPE Release)
    if(${CREATE_DEBUG_SYMBOLS})
    message(STATUS "Creating debug symbols enabled")
        if(MSVC)
            # For MSVC, use /Zi to generate PDB files
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
            set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
            # Enable separate PDB files
            set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
            set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
        elseif(APPLE)
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -gline-tables-only")
            set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,-S")  # -S strips debug info
        endif()
    endif()
endif()

option(ENABLE_C_ABI "Enable C ABI" OFF)
if(${ENABLE_C_ABI})
    message(STATUS "Enabling C ABI functions")
endif()

option(RUN_TESTS "Run tests" OFF)

option(COMPILE_EXAMPLES "Compile examples" OFF)

#Unreal Engine on Linux uses libc++ insted of libstdc++. This is required to build it correctly for UE (requires libc++ to be installed)
option(LINUX_FOR_UNREAL "Using Linux for Unreal Engine" OFF)
if(${LINUX_FOR_UNREAL})
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostdinc++ -I/usr/include/c++/v1")
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/lib/llvm-14/lib -Wl,-rpath,/usr/lib/llvm-14/lib -lc++ -lc++abi")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/pubnub_chat)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src) # not public imports

include(FetchContent)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(USE_GRANT_TOKEN_API ON)
set(USE_SYNC_API ON)
set(USE_CALLBACK_API ON)
set(USE_SET_DNS_SERVERS ON)
set(USE_IPV6 ON)
set(USE_SUBSCRIBE_EVENT_ENGINE ON)
set(USE_LOG_CALLBACK ON)
set(USE_NTF_RUNTIME_SELECTION ON)

if(WIN32 OR WIN64 OR MSVC)
    set(CUSTOM_BOOL_TYPE _Bool)
endif()

FetchContent_Declare(
  pubnub
  GIT_REPOSITORY https://github.com/pubnub/c-core.git
  GIT_TAG v5.0.0
  GIT_SHALLOW TRUE
  GIT_PROGRESS ON
  SYSTEM
)

FetchContent_MakeAvailable(pubnub)

option(RUN_GOOGLE_TESTS "Run Google Tests" OFF)

if(${RUN_GOOGLE_TESTS})
    message(STATUS "Enabling Google Tests")
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/a6ce08a.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# TODO: this flag is okay until we release the Cpp-Chat as an independent library
if(${ENABLE_C_ABI})
    target_compile_options(pubnub PUBLIC -DPUBNUB_SDK_VERSION_SUFFIX=\"/CA-Unity/0.4.3\")
else()
    target_compile_options(pubnub PUBLIC -DPUBNUB_SDK_VERSION_SUFFIX=\"/CA-Unreal/0.4.0\")
endif()

set(CCORE_COMPILE_OPTIONS -DPUBNUB_NTF_RUNTIME_SELECTION -DPUBNUB_USE_SUBSCRIBE_EVENT_ENGINE=1 -DPUBNUB_USE_SUBSCRIBE_V2=1 -DPUBNUB_CALLBACK_API=1 -DPUBNUB_SET_DNS_SERVERS=1 -DPUBNUB_USE_IPV6=1 -DPUBNUB_USE_LOG_CALLBACK=1)
target_compile_options(pubnub PUBLIC ${CCORE_COMPILE_OPTIONS})

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
  GIT_SHALLOW TRUE
  GIT_PROGRESS ON
  SYSTEM
)

FetchContent_MakeAvailable(json)

FetchContent_Declare(
    diff-match-patch
    GIT_REPOSITORY https://github.com/leutloff/diff-match-patch-cpp-stl
    GIT_TAG ba3bbe3cdba5d461e241b756a50085c3602a3b5a
    GIT_SHALLOW TRUE
    GIT_PROGRESS ON 
    SYSTEM 
)

#TODO no need to build tests
FetchContent_MakeAvailable(diff-match-patch)

include_directories(${pubnub_SOURCE_DIR}/posix)
include_directories(${pubnub_SOURCE_DIR}/core)
include_directories(${pubnub_SOURCE_DIR})

include_directories(${json_SOURCE_DIR})
include_directories(${json_SOURCE_DIR}/include)

include_directories(${diff-match-patch_SOURCE_DIR})

include_directories(${CURRENT_SOURCE_DIR}/include)

set(INFRA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra/entity_repository.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra/callbacks_repository.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra/serialization.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra/timer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra/pubnub.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra/interval_task.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra/rate_limiter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra/logger.cpp)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/timetoken.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/restrictions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/message_draft_entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/quotes.cpp)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/draft_service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/dao/channel_dao.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/dao/user_dao.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/dao/message_dao.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/dao/membership_dao.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/dao/message_draft_dao.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/subscription.cpp)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/presentation/callback_handle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/chat_helpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/string.cpp)

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
        ${CMAKE_CURRENT_SOURCE_DIR}/c_abi/src/c_thread_message.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/c_abi/src/c_callback_handle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/c_abi/src/c_response.cpp)
endif()

set(SOURCES
    ${SOURCES}
    ${INFRA_SOURCES}
    ${DOMAIN_SOURCES}
    ${APP_SOURCES})


add_library(pubnub-chat SHARED ${SOURCES})

target_compile_options(pubnub-chat PRIVATE ${CCORE_COMPILE_OPTIONS})

target_link_libraries(pubnub-chat PRIVATE -lpthread pubnub)

# TODO: kept for debugging purposes - should be deleted before release
add_executable(pubnub-chat-example example/main.cpp)
target_compile_options(pubnub-chat-example PRIVATE ${CCORE_COMPILE_OPTIONS})
target_link_libraries(pubnub-chat-example PUBLIC -lpthread pubnub-chat)

if(${COMPILE_EXAMPLES})
    function(example name)
        add_executable(pubnub_${name} example/${name}.cpp)
        target_compile_options(pubnub_${name} PRIVATE ${CCORE_COMPILE_OPTIONS})
        target_link_libraries(pubnub_${name} PRIVATE pubnub-chat)
    endfunction()

    if(NOT ${ENABLE_C_ABI})
        # It uses different mechanism systems for callbacks
        # in C++ and C, so it's not possible to run this example
        example(message_draft)
    endif()
    example(hello_world)
    example(forward_message)
    example(report_user)
    example(thread_channel)
    example(message_reactions)
    example(delete_message)
    example(typing)
    example(entity_update)
    example(read_receipts)
    example(restrictions)
endif()

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

    add_test(NAME diff-match-patch-test COMMAND ${diff-match-patch_SOURCE_DIR}/../diffmatchpatch-build/diff_match_patch_test_string)

    test_suite(string-unit ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/string_unit.cpp)
    test_suite(json-unit ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/json_unit.cpp)
    test_suite(typing-unit ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/typing_unit.cpp)
    test_suite(presence-unit ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/presence_unit.cpp)
    test_suite(timetoken-unit ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/timetoken_unit.cpp)
    test_suite(restrictions-unit ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/restrictions_unit.cpp)
    test_suite(user-entity-unit ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/user_entity_unit.cpp)
    test_suite(channel-entity-unit ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/channel_entity_unit.cpp)
    test_suite(prarsers-unit ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/parsers_unit.cpp)
    test_suite(member-entity-unit ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/membership_entity_unit.cpp)
    test_suite(message-draft-entity-unit ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/message_draft_entity_unit.cpp)
    test_suite(quotes-unit ${CMAKE_CURRENT_SOURCE_DIR}/test/unit/quotes_unit.cpp)
endif()

if(${RUN_GOOGLE_TESTS})
    include(GoogleTest)

    function(prepare_e2e_test test_file_name)
    
        add_executable(
          ${test_file_name}
          ${CMAKE_CURRENT_SOURCE_DIR}/test/e2e/${test_file_name}.cpp
        )
        target_link_libraries(
          ${test_file_name}
          GTest::gtest_main
        )
        target_link_libraries(${test_file_name} pubnub-chat)

        gtest_discover_tests(${test_file_name})

    endfunction()

    enable_testing()
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/test/e2e)

    prepare_e2e_test(message_draft_e2e)
    prepare_e2e_test(channel_e2e)
    prepare_e2e_test(threads_e2e)
    prepare_e2e_test(user_e2e)
    prepare_e2e_test(message_e2e)
endif()
