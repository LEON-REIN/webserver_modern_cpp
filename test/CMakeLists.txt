file(GLOB headers CONFIGURE_DEPENDS include/*.h)

# Adds a test.
# Usage: add_fmt_test(name extra_link_libs)
function(add_my_test name)
    add_executable(test_${name} ${headers} test_${name}.cc)
    target_include_directories(test_${name} PRIVATE include)

    # common link libs
    target_link_libraries(test_${name}
            PRIVATE Threads::Threads
            PRIVATE fmt::fmt
            PRIVATE utils
            )

    # extra link libs
    math(EXPR _num "${ARGC}-1")
    message("Test ${name} with ${_num} libs -> ${ARGN}")
    foreach(extra_lib ${ARGN})  # args without name
        target_link_libraries(test_${name} PRIVATE ${extra_lib})
    endforeach()

    if (WITH_TBB)
        target_compile_definitions(test_${name} PRIVATE WITH_TBB)
        target_link_libraries(test_${name} PRIVATE TBB::tbb)
    endif()

    add_test(NAME test_${name} COMMAND test_${name})
endfunction()

add_my_test(dynamic_pool ThreadPool)
add_my_test(steady_pool ThreadPool)
