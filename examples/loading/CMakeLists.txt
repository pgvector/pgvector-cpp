cmake_minimum_required(VERSION 3.18)

project(example)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(libpqxx GIT_REPOSITORY https://github.com/jtv/libpqxx.git GIT_TAG 7.10.1)
FetchContent_MakeAvailable(libpqxx)

add_subdirectory("${PROJECT_SOURCE_DIR}/../.." pgvector)

add_executable(example example.cpp)
target_link_libraries(example PRIVATE libpqxx::pqxx pgvector::pgvector)
