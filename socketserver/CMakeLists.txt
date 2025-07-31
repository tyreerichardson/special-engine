cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0144 NEW) # Use *_ROOT environment variables
cmake_policy(SET CMP0167 OLD) # Allow FindBoost

project(boost_websocket)

set(CMAKE_CXX_STANDARD 17)

# Paths to Boost (assumes you downloaded and built Boost in a 'boost/' folder)
set(BOOST_ROOT $ENV{BOOST_ROOT})
set(BOOST_INCLUDEDIR $ENV{BOOST_INCLUDEDIR})
set(BOOST_LIBRARYDIR $ENV{BOOST_LIBRARYDIR})

# Force CMake to ignore system boost
set(Boost_NO_SYSTEM_PATHS ON)

find_package(Boost 1.78.0 REQUIRED COMPONENTS thread system json
  HINTS ${BOOST_ROOT} ${BOOST_INCLUDEDIR} ${BOOST_LIBRARYDIR})


message(STATUS "Boost include dir: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")

# Include Boost headers
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${BOOST_LIBRARYDIR})

# Add executable
add_executable(boost_websocket_server socketserver/server.cpp)

# Link Boost libraries
target_link_libraries(boost_websocket_server
    ${Boost_LIBRARIES}
)


#### Previous CMAKE File below #####
#cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
# Set Boost path manually if it's not installed system-wide
# this location is needed to be on your device
# set(Boost_DEBUG ON)
# set(BOOST_ROOT "${CMAKE_SOURCE_DIR}/../boost")
# set(BOOST_INCLUDEDIR "${BOOST_ROOT}")
# set(BOOST_LIBRARYDIR "${BOOST_ROOT}/stage/lib")

# Optionally help with linking
# set(Boost_USE_STATIC_LIBS        ON)
# set(Boost_USE_MULTITHREADED      ON)
# set(Boost_USE_STATIC_RUNTIME     OFF)

# Find Boost components
# find_package(Boost 1.78.0 REQUIRED COMPONENTS json thread system)

#Add Boost headers
# include_directories(${Boost_INCLUDE_DIRS})

#if(NOT Boost_FOUND)
#    message(FATAL_ERROR "Could not find Boost")
#endif()

# add_executable(boost_websocket_server server.cpp message.hpp)

# link_directories(${Boost_LIBRARY_DIRS})

# Include Boost headers (These headers were manually added)
# target_include_directories(boost_websocket_server PRIVATE "${BOOST_INCLUDEDIR}")
# target_link_libraries(boost_websocket_server ${Boost_LIBRARIES})

# For Windows/Mingw you might need to link extra libraries
#if(MINGW)
#    target_link_libraries(boost_websocket_server PRIVATE ws2_32 mswsock)
#endif()

