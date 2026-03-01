cmake_minimum_required(VERSION 3.18)

project(example)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(disco GIT_REPOSITORY https://github.com/ankane/disco-cpp.git GIT_TAG v0.1.3)
FetchContent_Declare(libpqxx GIT_REPOSITORY https://github.com/jtv/libpqxx.git GIT_TAG 8.0.0)
FetchContent_MakeAvailable(disco libpqxx)

add_subdirectory("${PROJECT_SOURCE_DIR}/../.." pgvector)

add_executable(example example.cpp)
target_link_libraries(example PRIVATE disco::disco libpqxx::pqxx pgvector::pgvector)
