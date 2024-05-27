# CMakeLists.txt for the C-core library of Pubnub.
cmake_minimum_required(VERSION 3.12)

project(pubnub-chat VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 11)

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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

include(FetchContent)

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

include_directories(${CURRENT_SOURCE_DIR}/include)

include_directories(${c-core_SOURCE_DIR}/posix)
include_directories(${c-core_SOURCE_DIR}/core)
include_directories(${c-core_SOURCE_DIR})

include_directories(${json_SOURCE_DIR})
include_directories(${json_SOURCE_DIR}/include)

add_library(pubnub-chat
    SHARED
    src/lib.cpp
    src/chat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/chat/channel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/chat/user.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/chat/message.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/string.cpp
    # TODO: compile flag for c_functions
    ${CMAKE_CURRENT_SOURCE_DIR}/src/c_functions/c_chat.cpp)

target_link_libraries(pubnub-chat PRIVATE pubnub)

add_executable(pubnub-chat-example example/main.cpp)

target_link_libraries(pubnub-chat-example PUBLIC pubnub-chat)

