cmake_minimum_required(VERSION 3.18)

project(test)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unknown-attributes -Werror")
set(SKIP_BUILD_TEST ON)

include(FetchContent)

FetchContent_Declare(libpqxx GIT_REPOSITORY https://github.com/jtv/libpqxx.git GIT_TAG 7.10.0)
FetchContent_MakeAvailable(libpqxx)

add_subdirectory("${PROJECT_SOURCE_DIR}/.." pgvector)

add_executable(pqxx_test pqxx_test.cpp)
target_link_libraries(pqxx_test PRIVATE libpqxx::pqxx pgvector::pgvector)
