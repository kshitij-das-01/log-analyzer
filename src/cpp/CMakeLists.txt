cmake_minimum_required(VERSION 3.16)

project(LogAnalyzerCpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCE_FILES
    src/LogLevel.cpp
    src/LogEntry.cpp
    src/InMemoryIndex.cpp
)

add_library(log_analyzer_core ${SOURCE_FILES})

# pybind11 bindings module for Python integration
find_package(pybind11 REQUIRED)

add_library(log_analyzer_bindings SHARED
    src/bindings/bindings.cpp
)
set_target_properties(log_analyzer_bindings PROPERTIES
    PYBIND11_MODULE log_analyzer_bindings
    CXX_STANDARD 17
)
target_link_libraries(log_analyzer_bindings PRIVATE log_analyzer_core pybind11::pybind11)
target_include_directories(log_analyzer_bindings PRIVATE ${CMAKE_SOURCE_DIR}/include)

# GoogleTest — fetch and build
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

# Test executable
add_executable(log_analyzer_tests
    tests/test_LogLevel.cpp
    tests/test_LogEntry.cpp
    tests/test_InMemoryIndex.cpp
)
target_link_libraries(log_analyzer_tests PRIVATE log_analyzer_core GTest::gtest_main)

# Enable testing
include(CTest)
add_test(NAME log_analyzer_tests COMMAND log_analyzer_tests)
