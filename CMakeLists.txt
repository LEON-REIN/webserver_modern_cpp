### --- Basic settings for this project ---
cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 17 Required)
set(CMAKE_CXX_EXTENSIONS ON)  # enable `gcc` extensions
set(BUILD_SHARED_LIBS OFF)  # build static lib or shared

# Include guards
if (PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR
            "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
            )
endif ()

IF (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
ENDIF ()

project(webserver_modern_cpp
        LANGUAGES C CXX
        VERSION 0.1.0
        )

# Export compile commands to a json, which may be useful for `clangd`
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Make compiling faster with cache
if (NOT MSVC)
    find_program(CCACHE_PROGRAM ccache)
    if (CCACHE_PROGRAM)
        message(STATUS "Found CCache: ${CCACHE_PROGRAM}")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_PROGRAM})
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE_PROGRAM})
    endif ()
endif ()


### --- Add 3-rd libraries ---
# fmt
add_subdirectory(third_party/fmt-9.1.0)

# Threads
find_package(Threads REQUIRED)

# Libevent
find_package(Libevent REQUIRED COMPONENTS core)

# TBB
# option(WITH_TBB "..." ON)  # WILL BE CACHED! Should be modified through terminal: `cmake ... -DWITH_TBB=OFF`
set(WITH_TBB OFF CACHE BOOL "set to ON to enable TBB")  # FORCE: cannot be modified through terminal
# set(TBB_DIR /opt/intel/oneapi/tbb/latest/lib/cmake/tbb)  # better be set through terminal (export env or cmake -D)
if (WITH_TBB)
    find_package(TBB REQUIRED COMPONENTS tbb)
endif ()


### --- Add my submodules ---
add_subdirectory(utils)
add_subdirectory(threadpool)


### --- Add main module with executable ---
add_subdirectory(webserver)


### --- Add tests ---
option(TEST "Generate the test target." OFF)
if (TEST)
    enable_testing()
    add_subdirectory(test)
endif ()


### --- Tests ---
message(\n)
message(STATUS "********** MY MESSAGES ************")
message("CMAKE_SYSTEM: [${CMAKE_SYSTEM}]")
message("CMAKE_BUILD_TYPE: [${CMAKE_BUILD_TYPE}]")
message("PROJECT_NAME: [${PROJECT_NAME}]; VERSION: [${${PROJECT_NAME}_VERSION}]")
message(CMAKE_BUILD_TOOL:\ [${CMAKE_BUILD_TOOL}])  # build tool: ninja/make/nmake ...
message("PROJECT_SOURCE_DIR [${PROJECT_SOURCE_DIR}]")  # the nearest project root with `project(...)` command
message("Same as above: [${webserver_modern_cpp_SOURCE_DIR}]")
message("PROJECT_BINARY_DIR [${PROJECT_BINARY_DIR}]")  # the building dir passed to `cmake -B build-dir `
message("CMAKE_CURRENT_SOURCE_DIR [${CMAKE_CURRENT_SOURCE_DIR}]")  # where is the current CMakeList.txt
message("TBB_IMPORTED_TARGETS: [${TBB_IMPORTED_TARGETS}]")
message("CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
message(\n)
