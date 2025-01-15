cmake_minimum_required(VERSION 3.18)

project(pgvector VERSION 0.2.1 LANGUAGES CXX)

include(GNUInstallDirs)

add_library(pgvector INTERFACE)
add_library(pgvector::pgvector ALIAS pgvector)

target_compile_features(pgvector INTERFACE cxx_std_17)

target_include_directories(
    pgvector
    INTERFACE
    "${PROJECT_SOURCE_DIR}/include/"
)

install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
