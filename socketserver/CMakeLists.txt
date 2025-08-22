cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0144 NEW) # Use *_ROOT environment variables

project(boost_websocket)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Paths to Boost (assumes you downloaded and built Boost in a 'boost/' folder)
set(BOOST_ROOT $ENV{BOOST_ROOT})
set(BOOST_INCLUDEDIR $ENV{BOOST_INCLUDEDIR})
set(BOOST_LIBRARYDIR $ENV{BOOST_LIBRARYDIR})


# Force CMake to ignore system boost
set(Boost_NO_SYSTEM_PATHS ON)
# Make CMake use the old FindBoost mode
set(Boost_NO_BOOST_CMAKE ON)

find_package(Boost 1.79.0 REQUIRED COMPONENTS thread system json)

message(STATUS "Boost include dir: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")

# Include Boost headers
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${BOOST_LIBRARYDIR})

# Add executable
add_executable(boost_websocket server.cpp message.hpp cassconnect.hpp)

# Find Casssandra header
find_path(CASSANDRA_INCLUDE_DIR cassandra.h
    PATHS /usr/include /usr/local/include
)

# Find Cassandra library (-lcassandra)
find_library(CASSANDRA_LIB cassandra
    PATHS /usr/lib /usr/local/lib
)

if (NOT CASSANDRA_INCLUDE_DIR)
    message(FATA_ERROR "cassandra.h not found. Install libcassandra-dev or the DataStax driver.")
endif()

if (NOT CASSANDRA_LIB)
    message(FATAL_ERROR "libcassandra not found. Install libcassandra-dev or the DataStax driver.")
endif()

target_link_directories(boost_websocket PRIVATE ${CASSANDRA_INCLUDE_DIR})

# Link Boost libraries
target_link_libraries(boost_websocket
    PRIVATE ${Boost_LIBRARIES}
    ${CASSANDRA_LIB}
)
