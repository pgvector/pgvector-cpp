cmake_minimum_required(VERSION 3.18)

project(example)

set(CMAKE_CXX_STANDARD 17)

# for libpqxx
set(CMAKE_CXX_FLAGS "-Wno-unknown-attributes")
set(SKIP_BUILD_TEST ON)

include(FetchContent)

FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git GIT_TAG 1.11.1)
FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG v3.11.3)
FetchContent_Declare(libpqxx GIT_REPOSITORY https://github.com/jtv/libpqxx.git GIT_TAG 7.10.0)
FetchContent_MakeAvailable(cpr json libpqxx)

add_executable(example example.cpp)
target_include_directories(example PRIVATE ${CMAKE_SOURCE_DIR}/../../include)
target_link_libraries(example PRIVATE cpr::cpr nlohmann_json::nlohmann_json pqxx)
