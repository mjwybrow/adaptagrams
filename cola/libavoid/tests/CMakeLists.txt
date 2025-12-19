
# libavoid tests

function(add_avoid_test)

    set(options)

    set(oneValueArgs
        NAME
    )

    set(multiValueArgs
        SOURCES
    )

    cmake_parse_arguments(AVOID_TEST "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN} )

    add_executable(${AVOID_TEST_NAME}
        ${AVOID_TEST_SOURCES}
    )

    target_include_directories(${AVOID_TEST_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}/cola
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_link_libraries(${AVOID_TEST_NAME}
        PUBLIC
        avoid
    )

    add_test(NAME ${AVOID_TEST_NAME}
             COMMAND ${AVOID_TEST_NAME}
    )

endfunction()


file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/output)


add_avoid_test(NAME penaltyRerouting01 SOURCES penaltyRerouting01.cpp )
add_avoid_test(NAME treeRootCrash01 SOURCES treeRootCrash01.cpp )
add_avoid_test(NAME treeRootCrash02 SOURCES treeRootCrash02.cpp )
add_avoid_test(NAME hyperedgeRerouting01 SOURCES hyperedgeRerouting01.cpp )
add_avoid_test(NAME forwardFlowingConnectors01 SOURCES forwardFlowingConnectors01.cpp )
add_avoid_test(NAME removeJunctions01 SOURCES removeJunctions01.cpp )
add_avoid_test(NAME endlessLoop01 SOURCES endlessLoop01.cpp )
add_avoid_test(NAME nudgingSkipsCheckpoint01 SOURCES nudgingSkipsCheckpoint01.cpp )
add_avoid_test(NAME nudgingSkipsCheckpoint02 SOURCES nudgingSkipsCheckpoint02.cpp )
add_avoid_test(NAME nudgeCrossing01 SOURCES nudgeCrossing01.cpp )
add_avoid_test(NAME checkpointNudging1 SOURCES checkpointNudging1.cpp )
add_avoid_test(NAME checkpointNudging2 SOURCES checkpointNudging2.cpp )
add_avoid_test(NAME checkpointNudging3 SOURCES checkpointNudging3.cpp )
add_avoid_test(NAME finalSegmentNudging1 SOURCES finalSegmentNudging1.cpp )
add_avoid_test(NAME finalSegmentNudging2 SOURCES finalSegmentNudging2.cpp )
add_avoid_test(NAME finalSegmentNudging3 SOURCES finalSegmentNudging3.cpp )
add_avoid_test(NAME buildOrthogonalChannelInfo1 SOURCES buildOrthogonalChannelInfo1.cpp )
add_avoid_test(NAME hyperedgeLoop1 SOURCES hyperedgeLoop1.cpp )
add_avoid_test(NAME improveHyperedge01 SOURCES improveHyperedge01.cpp )
add_avoid_test(NAME improveHyperedge02 SOURCES improveHyperedge02.cpp )
add_avoid_test(NAME improveHyperedge03 SOURCES improveHyperedge03.cpp )
add_avoid_test(NAME improveHyperedge04 SOURCES improveHyperedge04.cpp )
add_avoid_test(NAME improveHyperedge05 SOURCES improveHyperedge05.cpp )
add_avoid_test(NAME improveHyperedge06 SOURCES improveHyperedge06.cpp )
add_avoid_test(NAME performance01 SOURCES performance01.cpp )
add_avoid_test(NAME restrictedNudging SOURCES restrictedNudging.cpp )
add_avoid_test(NAME freeFloatingDirection01 SOURCES freeFloatingDirection01.cpp )
add_avoid_test(NAME checkpoints01 SOURCES checkpoints01.cpp )
add_avoid_test(NAME checkpoints02 SOURCES checkpoints02.cpp )
add_avoid_test(NAME checkpoints03 SOURCES checkpoints03.cpp )
add_avoid_test(NAME inlineShapes SOURCES inlineShapes.cpp )
add_avoid_test(NAME 2junctions SOURCES 2junctions.cpp )
add_avoid_test(NAME overlappingRects SOURCES overlappingRects.cpp )
add_avoid_test(NAME lineSegWrapperCrash1 SOURCES lineSegWrapperCrash1.cpp )
add_avoid_test(NAME lineSegWrapperCrash2 SOURCES lineSegWrapperCrash2.cpp )
add_avoid_test(NAME lineSegWrapperCrash3 SOURCES lineSegWrapperCrash3.cpp )
add_avoid_test(NAME lineSegWrapperCrash4 SOURCES lineSegWrapperCrash4.cpp )
add_avoid_test(NAME lineSegWrapperCrash5 SOURCES lineSegWrapperCrash5.cpp )
add_avoid_test(NAME lineSegWrapperCrash6 SOURCES lineSegWrapperCrash6.cpp )
add_avoid_test(NAME lineSegWrapperCrash7 SOURCES lineSegWrapperCrash7.cpp )
add_avoid_test(NAME lineSegWrapperCrash8 SOURCES lineSegWrapperCrash8.cpp )
add_avoid_test(NAME example SOURCES example.cpp )
add_avoid_test(NAME validPaths01 SOURCES validPaths01.cpp )
add_avoid_test(NAME validPaths02 SOURCES validPaths02.cpp )
add_avoid_test(NAME multiconnact SOURCES multiconnact.cpp )
add_avoid_test(NAME complex SOURCES complex.cpp )
add_avoid_test(NAME inline SOURCES inline.cpp )
add_avoid_test(NAME infinity SOURCES infinity.cpp )
add_avoid_test(NAME latesetup SOURCES latesetup.cpp )
add_avoid_test(NAME nudgeold SOURCES nudgeold.cpp )
add_avoid_test(NAME node1 SOURCES node1.cpp )
add_avoid_test(NAME vertlineassertion SOURCES vertlineassertion.cpp )
add_avoid_test(NAME inlineoverlap01 SOURCES inlineoverlap01.cpp )
add_avoid_test(NAME inlineoverlap02 SOURCES inlineoverlap02.cpp )
add_avoid_test(NAME inlineoverlap03 SOURCES inlineoverlap03.cpp )
add_avoid_test(NAME inlineoverlap04 SOURCES inlineoverlap04.cpp )
add_avoid_test(NAME inlineoverlap05 SOURCES inlineoverlap05.cpp )
add_avoid_test(NAME inlineoverlap06 SOURCES inlineoverlap06.cpp )
add_avoid_test(NAME inlineoverlap07 SOURCES inlineoverlap07.cpp )
add_avoid_test(NAME inlineoverlap08 SOURCES inlineoverlap08.cpp )
add_avoid_test(NAME inlineOverlap09 SOURCES inlineOverlap09.cpp )
add_avoid_test(NAME inlineOverlap10 SOURCES inlineOverlap10.cpp )
add_avoid_test(NAME inlineOverlap11 SOURCES inlineOverlap11.cpp )
add_avoid_test(NAME orthordering01 SOURCES orthordering01.cpp )
add_avoid_test(NAME orthordering02 SOURCES orthordering02.cpp )
add_avoid_test(NAME tjunct SOURCES tjunct.cpp )
add_avoid_test(NAME hyperedge01 SOURCES hyperedge01.cpp )
add_avoid_test(NAME hyperedge02 SOURCES hyperedge02.cpp )
add_avoid_test(NAME nudgeintobug SOURCES nudgeintobug.cpp )
add_avoid_test(NAME slowrouting SOURCES slowrouting.cpp )
add_avoid_test(NAME orderassertion SOURCES orderassertion.cpp )
add_avoid_test(NAME connendmove SOURCES connendmove.cpp )
add_avoid_test(NAME connectionpin01 SOURCES connectionpin01.cpp )
add_avoid_test(NAME connectionpin02 SOURCES connectionpin02.cpp )
add_avoid_test(NAME connectionpin03 SOURCES connectionpin03.cpp )
add_avoid_test(NAME junction01 SOURCES junction01.cpp )
add_avoid_test(NAME junction02 SOURCES junction02.cpp )
add_avoid_test(NAME junction03 SOURCES junction03.cpp )
add_avoid_test(NAME junction04 SOURCES junction04.cpp )


