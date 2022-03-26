cmake_minimum_required(VERSION 3.16)

project(avoid)

option(LIBAVOID_DEBUG "Libavoid debug logs" OFF)
option(PATH_DEBUG "Routing path debug logs" OFF)
option(NUDGE_DEBUG "Debug nudging" OFF)
option(ESTIMATED_COST_DEBUG "Debug estimated cost" OFF)
option(ASTAR_DEBUG "Debug ASTAR" OFF)

add_library(${PROJECT_NAME}
    actioninfo.cpp
    connectionpin.cpp
    connector.cpp
    connend.cpp
    geometry.cpp
    geomtypes.cpp
    graph.cpp
    hyperedge.cpp
    hyperedgeimprover.cpp
    hyperedgetree.cpp
    junction.cpp
    makepath.cpp
    mtst.cpp
    obstacle.cpp
    orthogonal.cpp
    router.cpp
    scanline.cpp
    shape.cpp
    timer.cpp
    vertices.cpp
    viscluster.cpp
    visibility.cpp
    vpsc.cpp
)

if (LIBAVOID_DEBUG)
    add_definitions(-DLIBAVOID_DEBUG)
endif()

if (PATH_DEBUG)
    add_definitions(-DPATHDEBUG)
endif()

if (NUDGE_DEBUG)
    add_definitions(-DNUDGE_DEBUG)
endif()

if (ESTIMATED_COST_DEBUG)
    add_definitions(-DESTIMATED_COST_DEBUG)
endif()

if (ASTAR_DEBUG)
    add_definitions(-DASTAR_DEBUG)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)

if (ENABLE_TESTS)
    add_definitions("-DIMAGE_OUTPUT_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/tests/output/\"")
    set(TEST_CASES
        2junctions
        buildOrthogonalChannelInfo1
        checkpointNudging1
        checkpointNudging2
        checkpointNudging3
        checkpoints01
        checkpoints02
        checkpoints03
        complex
        connectionpin01
        connectionpin02
        connectionpin03
        connendmove
        corneroverlap01
        endlessLoop01
        example
        finalSegmentNudging1
        finalSegmentNudging2
        finalSegmentNudging3
        forwardFlowingConnectors01
        freeFloatingDirection01
        hola01
        hyperedge01
        hyperedge02
        hyperedgeLoop1
        hyperedgeRerouting01
        improveHyperedge01
        improveHyperedge02
        improveHyperedge03
        improveHyperedge04
        improveHyperedge05
        improveHyperedge06
        infinity
        inline
        inlineOverlap01
        inlineOverlap02
        inlineOverlap03
        inlineOverlap04
        inlineOverlap05
        inlineOverlap06
        inlineOverlap07
        inlineOverlap08
        inlineOverlap09
        inlineOverlap10
        inlineOverlap11
        inlineShapes
        junction01
        junction02
        junction03
        junction04
        latesetup
        lineSegWrapperCrash1
        lineSegWrapperCrash2
        lineSegWrapperCrash3
        lineSegWrapperCrash4
        lineSegWrapperCrash5
        lineSegWrapperCrash6
        lineSegWrapperCrash7
        lineSegWrapperCrash8
        multiconnact
        node1
        nudgeCrossing01
        nudgeintobug
        nudgeold
        nudgingSkipsCheckpoint01
        nudgingSkipsCheckpoint02
        orderassertion
        orthordering01
        orthordering02
        overlappingRects
        penaltyRerouting01
        performance01
        reallyslowrouting
        removeJunctions01
        restrictedNudging
        slowrouting
        tjunct
        treeRootCrash01
        treeRootCrash02
        unsatisfiableRangeAssertion
        validPaths01
        validPaths02
        vertlineassertion
    )

    # old tests without test framework
    foreach(TEST_CASE IN LISTS TEST_CASES)
        # currently tests are just simple apps/executables, no test executor is used
        add_executable(test_avoid_${TEST_CASE} tests/${TEST_CASE}.cpp)
        target_link_libraries(test_avoid_${TEST_CASE} ${PROJECT_NAME})
        target_include_directories(test_avoid_${TEST_CASE} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../)
        add_test(NAME test_avoid_${TEST_CASE}
                COMMAND test_avoid_${TEST_CASE})
    endforeach()

    # new tests with test framework
    set(NEW_TEST_CASES
        orthogonal/hierarchical)
    foreach(TEST_CASE IN LISTS NEW_TEST_CASES)
        set(TARGET_NAME "")
        string(REPLACE "/" "_" TARGET_NAME ${TEST_CASE})
        set(FULL_TARGET_NAME ${PROJECT_NAME}_${TARGET_NAME}_test)
        add_executable(
                ${FULL_TARGET_NAME}
                tests/new/${TEST_CASE}.cpp
        )
        target_link_libraries(
                ${FULL_TARGET_NAME}
                gtest_main
                ${PROJECT_NAME}
        )
        gtest_discover_tests(${FULL_TARGET_NAME})
    endforeach()
endif()
