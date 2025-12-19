
# libtopology tests

function(add_topology_test)

    set(options)

    set(oneValueArgs
        NAME
    )

    set(multiValueArgs
      SOURCES
    )

    cmake_parse_arguments(
        TOPOLOGY_TEST 
        "${options}" "${oneValueArgs}"
        "${multiValueArgs}" ${ARGN}
    )

    add_executable(${TOPOLOGY_TEST_NAME}
        ${TOPOLOGY_TEST_SOURCES}
    )

    target_include_directories(${TOPOLOGY_TEST_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}/cola
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<${HAVE_CAIROMM}:${CAIROMM_INCLUDE_DIRS}>
    )

    target_link_directories(${TOPOLOGY_TEST_NAME}
        PUBLIC
        $<${HAVE_CAIROMM}:${CAIROMM_LIBRARY_DIRS}>
    )

    target_link_libraries(${TOPOLOGY_TEST_NAME}
        PUBLIC
        topology
        cola
        vpsc
        avoid
        $<${HAVE_CAIROMM}:${CAIROMM_LIBRARIES}>
    )

    add_test(NAME ${TOPOLOGY_TEST_NAME}
         COMMAND ${TOPOLOGY_TEST_NAME}
    )

endfunction()


file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/output)


add_topology_test(NAME simple_bend SOURCES simple_bend.cpp)
add_topology_test(NAME nooverlap SOURCES nooverlap.cpp)
add_topology_test(NAME nodedragging SOURCES nodedragging.cpp)
add_topology_test(NAME beautify SOURCES beautify.cpp)
add_topology_test(NAME orthogonalOpt SOURCES orthogonalOpt.cpp)
