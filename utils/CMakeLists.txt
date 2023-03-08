file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)
add_library(utils STATIC ${srcs})

target_include_directories(utils PUBLIC include)
target_link_libraries(utils PUBLIC fmt::fmt)