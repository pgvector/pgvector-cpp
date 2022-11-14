cmake_minimum_required(VERSION 3.10)

project(pgvector VERSION 0.1.1 LANGUAGES CXX)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/"
  DESTINATION "include/pgvector"
  FILES_MATCHING
  PATTERN "*.hpp"
)
