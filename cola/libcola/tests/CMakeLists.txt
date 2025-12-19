
# libcola tests

MESSAGE(STATUS "HAVE_CAIROMM: ${HAVE_CAIROMM}")
MESSAGE(STATUS "CAIROMM_INCLUDE_DIRS: ${CAIROMM_INCLUDE_DIRS}")
MESSAGE(STATUS "CAIROMM_LIBRARY_DIRS: ${CAIROMM_LIBRARY_DIRS}")
MESSAGE(STATUS "CAIROMM_LIBRARIES: ${CAIROMM_LIBRARIES}")

function(add_cola_test)

    set(options)

    set(oneValueArgs
        NAME
    )

    set(multiValueArgs
        SOURCES
    )

    cmake_parse_arguments(
        COLA_TEST 
        "${options}" "${oneValueArgs}"
        "${multiValueArgs}" ${ARGN}
    )

    add_executable(${COLA_TEST_NAME}
        ${COLA_TEST_SOURCES}
    )

    target_include_directories(${COLA_TEST_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}/cola
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<${HAVE_CAIROMM}:${CAIROMM_INCLUDE_DIRS}>
    )

    target_link_directories(${COLA_TEST_NAME}
        PUBLIC
        $<${HAVE_CAIROMM}:${CAIROMM_LIBRARY_DIRS}>
    )

    target_link_libraries(${COLA_TEST_NAME}
        PUBLIC
        vpsc
        topology
        cola
        avoid
        $<${HAVE_CAIROMM}:${CAIROMM_LIBRARIES}>
    )

    add_test(NAME ${COLA_TEST_NAME}
        COMMAND ${COLA_TEST_NAME}
    )

endfunction()


file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/output)

if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/data)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

add_cola_test(NAME initialOverlap SOURCES initialOverlap.cpp)
add_cola_test(NAME overlappingClusters01 SOURCES overlappingClusters01.cpp)
add_cola_test(NAME overlappingClusters02 SOURCES overlappingClusters02.cpp)
add_cola_test(NAME overlappingClusters04 SOURCES overlappingClusters04.cpp)
add_cola_test(NAME rectangularClusters01 SOURCES rectangularClusters01.cpp)
add_cola_test(NAME StillOverlap01 SOURCES StillOverlap01.cpp)
add_cola_test(NAME StillOverlap02 SOURCES StillOverlap02.cpp)
add_cola_test(NAME FixedRelativeConstraint01 SOURCES FixedRelativeConstraint01.cpp)
add_cola_test(NAME rectclustershapecontainment SOURCES rectclustershapecontainment.cpp) 
add_cola_test(NAME random_graph SOURCES random_graph.cpp)
add_cola_test(NAME page_bounds SOURCES page_bounds.cpp)
add_cola_test(NAME constrained SOURCES constrained.cpp)
add_cola_test(NAME unsatisfiable SOURCES unsatisfiable.cpp) 
add_cola_test(NAME invalid SOURCES invalid.cpp)
add_cola_test(NAME makefeasible SOURCES makefeasible02.cpp)
add_cola_test(NAME shortest_paths SOURCES shortest_paths.cpp)

