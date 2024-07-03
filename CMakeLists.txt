cmake_minimum_required(VERSION 3.18)

project(pgvector VERSION 0.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/"
  DESTINATION "include/pgvector"
  FILES_MATCHING
  PATTERN "*.hpp"
)
