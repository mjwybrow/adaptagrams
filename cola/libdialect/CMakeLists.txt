cmake_minimum_required(VERSION 3.16)

project(dialect)

add_library(${PROJECT_NAME}
    aca.cpp
    bendseqlookup.cpp
    chains.cpp
    constraints.cpp
    edges.cpp
    expansion.cpp
    faces.cpp
    graphs.cpp
    hola.cpp
    io.cpp
    logging.cpp
    nearalign.cpp
    nexes.cpp
    nodeconfig.cpp
    nodes.cpp
    ortho.cpp
    peeling.cpp
    planarise.cpp
    qalookup.cpp
    quadaction.cpp
    routing.cpp
    sides.cpp
    treeplacement.cpp
    trees.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../)

if (ENABLE_TESTS)
    # TODO: other test cases
    set(TEST_CASES routing01 chainconfig01 treeboxes01)

    foreach(TEST_CASE IN LISTS TEST_CASES)
        # currently tests are just simple apps/executables, no test executor is used
        add_executable(test_${PROJECT_NAME}_${TEST_CASE} tests/${TEST_CASE}.cpp)
        target_link_libraries(test_${PROJECT_NAME}_${TEST_CASE} ${PROJECT_NAME} avoid cola vpsc)
        target_include_directories(test_${PROJECT_NAME}_${TEST_CASE} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../)
        add_test(NAME test_${PROJECT_NAME}_${TEST_CASE}
                COMMAND test_${PROJECT_NAME}_${TEST_CASE})
    endforeach()
endif()
