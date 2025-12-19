
# libvpsc tests

function(add_vpsc_test)

    set(options)

    set(oneValueArgs
        NAME
    )

    set(multiValueArgs
      SOURCES
    )

    cmake_parse_arguments(
        VPSC_TEST 
        "${options}" "${oneValueArgs}"
        "${multiValueArgs}" ${ARGN}
    )

    add_executable(${VPSC_TEST_NAME}
        ${VPSC_TEST_SOURCES}
    )

    target_include_directories(${VPSC_TEST_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}/cola
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_link_libraries(${VPSC_TEST_NAME}
        PUBLIC
        vpsc
    )

    add_test(NAME ${VPSC_TEST_NAME}
         COMMAND ${VPSC_TEST_NAME}
    )

endfunction()

add_vpsc_test(NAME satisfy_inc SOURCES satisfy_inc.cpp)
add_vpsc_test(NAME block SOURCES block.cpp)
add_vpsc_test(NAME rectangleoverlap SOURCES rectangleoverlap.cpp)


