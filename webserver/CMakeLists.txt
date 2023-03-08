# CONFIGURE_DEPENDS: Automatically renew ${srcs} when new files added; Otherwise `cmake -B build-dir` is needed
# Adding an extra folder in include dir will avoiding ambiguity of header file with same name;
# Usage: `# include <folder/xxx.h>`
file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)  # without .h, `MSVS` can't recognize header files
# Generate the executable
add_executable(webserver ${srcs})


target_include_directories(webserver PUBLIC include)
target_compile_definitions(webserver PUBLIC WITH_TBB)  # can be seen in cpp files as a MACRO `#define WITH_TBB 1`
target_link_libraries(webserver
        PUBLIC fmt::fmt
        PUBLIC Threads::Threads
        PUBLIC utils
        PUBLIC libevent::core
        PUBLIC ThreadPool
        )
set_target_properties(webserver PROPERTIES  # set multiple target properties like this; Not necessary here
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        )
if (WITH_TBB)
    target_link_libraries(webserver PUBLIC TBB::tbb)
    message(STATUS "TBB_IMPORTED_TARGETS: [${TBB_IMPORTED_TARGETS}]")
endif ()

