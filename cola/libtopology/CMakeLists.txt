cmake_minimum_required(VERSION 3.16)

project(topology)

option(ENABLE_TESTS "Build tests" OFF)

add_library(${PROJECT_NAME}
    cola_topology_addon.cpp
    compute_forces.cpp
    orthogonal_topology.cpp
    resize.cpp
    topology_constraints.cpp
    topology_constraints_constructor.cpp
    topology_graph.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../)

if (ENABLE_TESTS)
    enable_testing()

#    TODO: other test cases
    add_executable(test_${PROJECT_NAME}_orthogonalOpt tests/orthogonalOpt.cpp)
    target_link_libraries(test_${PROJECT_NAME}_orthogonalOpt ${PROJECT_NAME} avoid cola topology vpsc)
    target_include_directories(test_${PROJECT_NAME}_orthogonalOpt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../)
    add_test(NAME test_${PROJECT_NAME}_orthogonalOpt
            COMMAND test_${PROJECT_NAME}_orthogonalOpt)
endif()