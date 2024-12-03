# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and language
project(DeribitTradingSystem LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(DeribitTradingSystem 
    src/main.cpp
    src/api/api_utils.cpp
    src/api/deribit_api.cpp
    src/websocket/websocket_server.cpp
)

# Include directories
target_include_directories(DeribitTradingSystem PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api
    ${CMAKE_CURRENT_SOURCE_DIR}/src/websocket
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    /opt/homebrew/Cellar/websocketpp/0.8.2/include  # Add Homebrew WebSocket++ include path
)

# Link libraries
find_package(CURL REQUIRED)
target_link_libraries(DeribitTradingSystem PRIVATE CURL::libcurl)

find_package(Threads REQUIRED)
target_link_libraries(DeribitTradingSystem PRIVATE Threads::Threads)

message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "Build Directory: ${CMAKE_BINARY_DIR}")
