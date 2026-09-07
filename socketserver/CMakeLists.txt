cmake_minimum_required(VERSION 3.16)
project(boost_websocket LANGUAGES CXX)
option(ENABLE_CASSANDRA "Build with Cassandra persistence" ON)

# Use installed packages on Linux; allow the existing staged Boost build on macOS.
if(APPLE)
    find_program(HOMEBREW_EXECUTABLE brew)
    if(HOMEBREW_EXECUTABLE)
        execute_process(COMMAND "${HOMEBREW_EXECUTABLE}" --prefix
            OUTPUT_VARIABLE HOMEBREW_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
        list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}")
    endif()
    set(_local_boost "${CMAKE_CURRENT_SOURCE_DIR}/../boost/boost_1_79_0/stage")
    if(NOT Boost_DIR AND NOT Boost_ROOT AND NOT DEFINED ENV{Boost_ROOT}
       AND NOT DEFINED ENV{BOOST_ROOT} AND EXISTS "${_local_boost}/lib/cmake/Boost-1.79.0")
        list(PREPEND CMAKE_PREFIX_PATH "${_local_boost}")
    endif()
endif()
find_package(Boost 1.79 CONFIG REQUIRED COMPONENTS thread system json)
find_package(Threads REQUIRED)

add_executable(boost_websocket server.cpp message.hpp user.hpp cassconnect.hpp)
target_compile_features(boost_websocket PRIVATE cxx_std_11)
set_target_properties(boost_websocket PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF NO_SYSTEM_FROM_IMPORTED ON)
target_link_libraries(boost_websocket PRIVATE Boost::thread Boost::system Boost::json
    Threads::Threads)

if(ENABLE_CASSANDRA)
    find_path(CASSANDRA_INCLUDE_DIR cassandra.h)
    find_library(CASSANDRA_LIBRARY NAMES cassandra)
    if(NOT CASSANDRA_INCLUDE_DIR OR NOT CASSANDRA_LIBRARY)
        message(FATAL_ERROR "Cassandra driver required with ENABLE_CASSANDRA=ON. Install it, set CMAKE_PREFIX_PATH, or select ENABLE_CASSANDRA=OFF.")
    endif()
    target_compile_definitions(boost_websocket PRIVATE ENABLE_CASSANDRA=1)
    target_include_directories(boost_websocket PRIVATE "${CASSANDRA_INCLUDE_DIR}")
    target_link_libraries(boost_websocket PRIVATE "${CASSANDRA_LIBRARY}")
endif()

# Boost 1.79 MPL triggers a newer Clang diagnostic while instantiating enum traits.
# Limit the compatibility flag to affected Boost releases and compilers that accept it.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND Boost_VERSION VERSION_LESS "1.85.0")
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-Wno-enum-constexpr-conversion" HAS_ENUM_CONSTEXPR_WORKAROUND)
    if(HAS_ENUM_CONSTEXPR_WORKAROUND)
        target_compile_options(boost_websocket PRIVATE -Wno-enum-constexpr-conversion)
    endif()
endif()

include(CTest)
if(BUILD_TESTING)
    find_package(Python3 3.6 REQUIRED COMPONENTS Interpreter)
    # Always test routing without database writes, even in an ON build directory.
    add_executable(socketserver_smoke_server server.cpp)
    add_executable(socketserver_smoke_client tests/smoke_client.cpp)
    foreach(test_target socketserver_smoke_server socketserver_smoke_client)
        set_target_properties(${test_target} PROPERTIES CXX_STANDARD 11
            CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF NO_SYSTEM_FROM_IMPORTED ON)
        target_link_libraries(${test_target} PRIVATE Boost::thread Boost::system
            Boost::json Threads::Threads)
        target_compile_options(${test_target} PRIVATE
            $<TARGET_PROPERTY:boost_websocket,COMPILE_OPTIONS>)
    endforeach()
    add_test(NAME socketserver.smoke
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/run_smoke.py"
            $<TARGET_FILE:socketserver_smoke_server>
            $<TARGET_FILE:socketserver_smoke_client>)
    set_tests_properties(socketserver.smoke PROPERTIES TIMEOUT 40
        RESOURCE_LOCK socketserver_port_1234)
endif()

if(BUILD_TESTING)
    add_test(NAME socketserver.shutdown
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/run_shutdown.py"
            $<TARGET_FILE:socketserver_smoke_server>
            $<TARGET_FILE:socketserver_smoke_client>)
    set_tests_properties(socketserver.shutdown PROPERTIES TIMEOUT 120
        RESOURCE_LOCK socketserver_port_1234)
endif()
