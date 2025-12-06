cmake_minimum_required(VERSION 3.18)

project(pgvector VERSION 0.2.4 LANGUAGES CXX)

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

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(BUILD_TESTING "" OFF)

    if(BUILD_TESTING)
        include(FetchContent)

        FetchContent_Declare(libpqxx GIT_REPOSITORY https://github.com/jtv/libpqxx.git GIT_TAG 7.10.4)
        FetchContent_MakeAvailable(libpqxx)

        add_executable(test test/halfvec_test.cpp test/main.cpp test/pqxx_test.cpp test/sparsevec_test.cpp test/vector_test.cpp)
        target_link_libraries(test PRIVATE libpqxx::pqxx pgvector::pgvector)
        if(NOT MSVC)
            target_compile_options(test PRIVATE -Wall -Wextra -Wpedantic -Werror)
        endif()
    endif()
endif()
