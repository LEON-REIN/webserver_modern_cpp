find_package(Threads REQUIRED)  # Threads

file(GLOB_RECURSE srcs CONFIGURE_DEPENDS include/*.h)
add_library(ThreadPool STATIC ${srcs})

target_include_directories(ThreadPool PUBLIC include)
set_target_properties(ThreadPool PROPERTIES  # set multiple target properties like this;
        LINKER_LANGUAGE CXX
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        )

target_link_libraries(ThreadPool PUBLIC Threads::Threads)
