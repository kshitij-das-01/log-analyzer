cmake_minimum_required(VERSION 3.16)

project(LogAnalyzerCpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files will be added here as implementation progresses
# set(SOURCE_FILES
#     src/LogEntry.cpp
#     src/InMemoryIndex.cpp
# )

# add_library(log_analyzer_core ${SOURCE_FILES})

# GoogleTest will be added here when tests are implemented
# find_package(GTest REQUIRED)
# add_executable(log_analyzer_tests tests/test_LogEntry.cpp)
# target_link_libraries(log_analyzer_tests GTest::gtest_main)