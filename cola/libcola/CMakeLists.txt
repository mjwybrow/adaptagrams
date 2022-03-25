cmake_minimum_required(VERSION 3.16)

project(cola)

set(CMAKE_CXX_STANDARD 17)

option(ENABLE_TESTS "Build tests" OFF)
option(ENABLE_CAIRO "Cairo support for SVG generation" OFF)

add_library(${PROJECT_NAME}
    box.cpp
    cc_clustercontainmentconstraints.cpp
    cc_nonoverlapconstraints.cpp
    cluster.cpp
    cola.cpp
    colafd.cpp
    compound_constraints.cpp
    conjugate_gradient.cpp
    connected_components.cpp
    convex_hull.cpp
    gradient_projection.cpp
    output_svg.cpp
    pseudorandom.cpp
    shapepair.cpp
    straightener.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../)
target_link_libraries(${PROJECT_NAME} cairomm-1.0 sigc-3.0 freetype)

if (ENABLE_TESTS)
    enable_testing()

#    TODO: other test cases
    set(TEST_CASES
#        boundary
        connected_components
        makefeasible
        page_bounds
        rectangularClusters01
        rectclustershapecontainment
        overlappingClusters01
        unconstrained
    )


    foreach(TEST_CASE IN LISTS TEST_CASES)
        # currently tests are just simple apps/executables, no test executor is used
        add_executable(test_${PROJECT_NAME}_${TEST_CASE} tests/${TEST_CASE}.cpp)
        target_include_directories(test_${PROJECT_NAME}_${TEST_CASE} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../)
        target_link_libraries(test_${PROJECT_NAME}_${TEST_CASE} ${PROJECT_NAME} avoid vpsc topology)
        if (ENABLE_CAIRO)
            add_definitions(-DCAIRO_HAS_SVG_SURFACE -DCAIRO_HAS_PDF_SURFACE -DHAVE_CAIROMM)
            target_link_libraries(test_${PROJECT_NAME}_${TEST_CASE} cairomm-1.0 sigc-3.0 freetype)
        endif()
        add_test(NAME test_${PROJECT_NAME}_${TEST_CASE}
                COMMAND test_${PROJECT_NAME}_${TEST_CASE})
    endforeach()
endif()