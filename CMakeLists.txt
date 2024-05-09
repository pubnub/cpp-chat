# CMakeLists.txt for the C-core library of Pubnub.
cmake_minimum_required(VERSION 3.12)

project(pubnub-chat VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

include(FetchContent)

FetchContent_Declare(
  pubnub
  GIT_REPOSITORY https://github.com/pubnub/c-core.git
  GIT_TAG f800ebeddfd24b6c838976a851c1ff6a88915a14
  GIT_SHALLOW TRUE
  GIT_PROGRESS ON
  SYSTEM
  OPTIONS "WITH_CPP ON"
)

FetchContent_MakeAvailable(pubnub)

include_directories(${CURRENT_SOURCE_DIR}/include)

include_directories(${c-core_SOURCE_DIR}/posix)
include_directories(${c-core_SOURCE_DIR}/core)
include_directories(${c-core_SOURCE_DIR})

add_library(pubnub-chat SHARED src/lib.cpp src/chat.cpp)

target_link_libraries(pubnub-chat PRIVATE pubnub)

add_executable(pubnub-chat-example example/main.cpp)

target_link_libraries(pubnub-chat-example PUBLIC pubnub-chat)

