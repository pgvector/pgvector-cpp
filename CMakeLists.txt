cmake_minimum_required(VERSION 3.18)

project(pgvector VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/include/"
    DESTINATION "include"
    FILES_MATCHING
    PATTERN "*.hpp"
)
