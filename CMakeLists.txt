cmake_minimum_required(VERSION 3.10)

project(pgvector VERSION 0.1.1 LANGUAGES CXX)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/"
  DESTINATION "include/pgvector"
  FILES_MATCHING
  PATTERN "*.hpp"
)

include(CTest)
set(CMAKE_CXX_STANDARD 17)
add_executable(TestPqxx test/pqxx_test.cpp)
find_package(libpqxx REQUIRED)
target_link_libraries(TestPqxx PRIVATE pq pqxx)
add_test(NAME TestPqxx COMMAND TestPqxx)
