cmake_minimum_required(VERSION 3.16)
project(boost_websocket)

# Use C++17 for Boost.Beast 
# I have not started using boost beast since my device lacks the compatibility for now
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Specify your toolchain and generator: use MinGW Makefiles when running cmake:
#cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..

# Set Boost path manually if it's not installed system-wide
# this location is needed to be on your device
set(BOOST_ROOT "C:/Boost/boost_1_78_0")

# Find Boost components
find_package(Boost 1.78.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find Boost")
endif()

add_executable(boost_websocket_server server.cpp)
add_executable(boost_websocket_client client.cpp)

# Include Boost headers
target_include_directories(boost_websocket_server PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(boost_websocket_client PRIVATE ${Boost_INCLUDE_DIRS})

# Link Boost libraries
target_link_libraries(boost_websocket_server PRIVATE ${Boost_LIBRARIES})
target_link_libraries(boost_websocket_client PRIVATE ${Boost_LIBRARIES})

# For Windows/Mingw you might need to link extra libraries
if(MINGW)
    target_link_libraries(boost_websocket_server PRIVATE ws2_32 mswsock)
    target_link_libraries(boost_websocket_client PRIVATE ws2_32 mswsock)
endif()

