cmake_minimum_required(VERSION 3.14)
project(boilerplate-cli-ui-cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch cpp-httplib
include(FetchContent)
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.15.3
)
FetchContent_MakeAvailable(httplib)

# Add executable
add_executable(${PROJECT_NAME} src/main.cpp)

# Include directory
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Link httplib
target_link_libraries(${PROJECT_NAME} PRIVATE httplib::httplib)

# Compiler flags for size optimization
if(CMAKE_BUILD_TYPE EQUAL "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE -Os -s)
endif()
