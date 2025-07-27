cmake_minimum_required(VERSION 3.10)
project(boost_websocket)

# Use C++17 for Boost.Beast 
# I have not started using boost beast since my device lacks the compatibility for now
set(CMAKE_CXX_STANDARD 11)

# Specify your toolchain and generator: use MinGW Makefiles when running cmake:
#cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..

# Set Boost path manually if it's not installed system-wide
# this location is needed to be on your device
set(Boost_DEBUG ON)
set(BOOST_ROOT "C:/Boost/boost_1_78_0")
set(BOOST_INCLUDEDIR "${BOOST_ROOT}")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/stage/lib")

# Optionally help with linking
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     OFF)

# Find Boost components
#find_package(Boost 1.78.0 REQUIRED COMPONENTS thread system)

#Add Boost headers
include_directories(${Boost_INCLUDE_DIRS})

#if(NOT Boost_FOUND)
#    message(FATAL_ERROR "Could not find Boost")
#endif()

add_executable(boost_websocket_server server.cpp)

# Include Boost headers (These headers were manually added)
target_include_directories(boost_websocket_server PRIVATE "${BOOST_INCLUDEDIR}")
target_link_libraries(boost_websocket_server
    "${BOOST_ROOT}/stage/lib/libboost_chrono-mgw8-mt-x32-1_78.a"
    "${BOOST_ROOT}/stage/lib/libboost_chrono-mgw8-mt-d-x32-1_78.a"
    "${BOOST_ROOT}/stage/lib/libboost_system-mgw8-mt-x32-1_78.a"
    "${BOOST_ROOT}/stage/lib/libboost_system-mgw8-mt-d-x32-1_78.a"
    "${BOOST_ROOT}/stage/lib/libboost_thread-mgw8-mt-x32-1_78.a"
    "${BOOST_ROOT}/stage/lib/libboost_thread-mgw8-mt-d-x32-1_78.a"
    ws2_32
)

# Link Boost libraries
target_link_libraries(boost_websocket_server PRIVATE ${Boost_LIBRARIES})

# For Windows/Mingw you might need to link extra libraries
#if(MINGW)
#    target_link_libraries(boost_websocket_server PRIVATE ws2_32 mswsock)
#endif()

