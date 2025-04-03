cmake_minimum_required(VERSION 3.18)

project(example)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git GIT_TAG 1.11.1)
FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG v3.11.3)
FetchContent_Declare(libpqxx GIT_REPOSITORY https://github.com/jtv/libpqxx.git GIT_TAG 7.10.1)
FetchContent_MakeAvailable(cpr json libpqxx)

add_subdirectory("${PROJECT_SOURCE_DIR}/../.." pgvector)

add_executable(example example.cpp)
target_link_libraries(example PRIVATE cpr::cpr libpqxx::pqxx nlohmann_json::nlohmann_json pgvector::pgvector)
