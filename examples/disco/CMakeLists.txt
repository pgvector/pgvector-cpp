cmake_minimum_required(VERSION 3.18)

project(example)

set(CMAKE_CXX_STANDARD 20)

# for libpqxx
set(CMAKE_CXX_FLAGS "-Wno-unknown-attributes")
set(SKIP_BUILD_TEST ON)

include(FetchContent)

FetchContent_Declare(disco GIT_REPOSITORY https://github.com/ankane/disco-cpp.git GIT_TAG v0.1.1)
FetchContent_Declare(libpqxx GIT_REPOSITORY https://github.com/jtv/libpqxx.git GIT_TAG 7.10.0)
FetchContent_MakeAvailable(disco libpqxx)

add_executable(example example.cpp)
target_include_directories(example PRIVATE ${disco_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/../../include)
target_link_libraries(example PRIVATE libpqxx::pqxx)
