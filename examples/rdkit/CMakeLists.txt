cmake_minimum_required(VERSION 3.18)

project(example)

set(CMAKE_CXX_STANDARD 17)

find_package(RDKit REQUIRED)
find_package(Boost COMPONENTS iostreams serialization system REQUIRED)

include(FetchContent)

FetchContent_Declare(libpqxx GIT_REPOSITORY https://github.com/jtv/libpqxx.git GIT_TAG 7.10.5)
FetchContent_MakeAvailable(libpqxx)

add_subdirectory("${PROJECT_SOURCE_DIR}/../.." pgvector)

add_executable(example example.cpp)
target_link_libraries(example PRIVATE libpqxx::pqxx pgvector::pgvector RDKit::Fingerprints RDKit::SmilesParse)
