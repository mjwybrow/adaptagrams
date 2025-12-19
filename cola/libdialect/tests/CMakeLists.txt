
# libdialect tests

function(add_dialect_test)

    set(options)

    set(oneValueArgs
        NAME
    )

    set(multiValueArgs
      SOURCES
    )

    cmake_parse_arguments(
        DIALECT_TEST 
        "${options}" "${oneValueArgs}"
        "${multiValueArgs}" ${ARGN}
    )

    add_executable(${DIALECT_TEST_NAME}
        ${DIALECT_TEST_SOURCES}
    )

    target_include_directories(${DIALECT_TEST_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}/cola
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_link_libraries(${DIALECT_TEST_NAME}
        PUBLIC
        dialect
        vpsc
        cola
        avoid   
    )

    add_test(NAME ${DIALECT_TEST_NAME}
         COMMAND ${DIALECT_TEST_NAME}
    )

endfunction()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/output/svg)

if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/graphs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/graphs 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

add_dialect_test(NAME aca SOURCES aca.cpp)
add_dialect_test(NAME assignments SOURCES assignments.cpp)
add_dialect_test(NAME bbox SOURCES bbox.cpp)
add_dialect_test(NAME bendcosts SOURCES bendcosts.cpp)
add_dialect_test(NAME chainconfig01 SOURCES chainconfig01.cpp)
add_dialect_test(NAME chainconfig02 SOURCES chainconfig02.cpp)
add_dialect_test(NAME chainconfig03 SOURCES chainconfig03.cpp)
add_dialect_test(NAME chainsandcycles SOURCES chainsandcycles.cpp)
add_dialect_test(NAME cmplayout01 SOURCES cmplayout01.cpp)
add_dialect_test(NAME collateralexpand01 SOURCES collateralexpand01.cpp)
add_dialect_test(NAME collateralexpand02 SOURCES collateralexpand02.cpp)
add_dialect_test(NAME conncomps SOURCES conncomps.cpp)
add_dialect_test(NAME containedsegment01 SOURCES containedsegment01.cpp)
add_dialect_test(NAME destress SOURCES destress.cpp)
add_dialect_test(NAME destress02 SOURCES destress02.cpp)
add_dialect_test(NAME destress_aca SOURCES destress_aca.cpp)
add_dialect_test(NAME expand01 SOURCES expand01.cpp)
add_dialect_test(NAME expand02 SOURCES expand02.cpp)
add_dialect_test(NAME expand03 SOURCES expand03.cpp)
add_dialect_test(NAME expand04 SOURCES expand04.cpp)
add_dialect_test(NAME expand05 SOURCES expand05.cpp)
add_dialect_test(NAME expand06 SOURCES expand06.cpp)
add_dialect_test(NAME expand07 SOURCES expand07.cpp)
add_dialect_test(NAME expand08 SOURCES expand08.cpp)
add_dialect_test(NAME expand09 SOURCES expand09.cpp)
add_dialect_test(NAME extrabdrygap SOURCES extrabdrygap.cpp)
add_dialect_test(NAME faceset01 SOURCES faceset01.cpp)
add_dialect_test(NAME faceset02 SOURCES faceset02.cpp)
add_dialect_test(NAME hola10 SOURCES hola10.cpp)
add_dialect_test(NAME hola11 SOURCES hola11.cpp)
add_dialect_test(NAME hola12 SOURCES hola12.cpp)
add_dialect_test(NAME hola_arpa SOURCES hola_arpa.cpp)
add_dialect_test(NAME hola_garr SOURCES hola_garr.cpp)
add_dialect_test(NAME hola_slovakia SOURCES hola_slovakia.cpp)
add_dialect_test(NAME holalonenode SOURCES holalonenode.cpp)
add_dialect_test(NAME inserttrees01 SOURCES inserttrees01.cpp)
add_dialect_test(NAME leaflessroute01 SOURCES leaflessroute01.cpp)
add_dialect_test(NAME leaflessroute02 SOURCES leaflessroute02.cpp)
add_dialect_test(NAME lookupqas SOURCES lookupqas.cpp)
add_dialect_test(NAME nbroctal SOURCES nbroctal.cpp)
add_dialect_test(NAME nearalign01 SOURCES nearalign01.cpp)
add_dialect_test(NAME nearalign02 SOURCES nearalign02.cpp)
add_dialect_test(NAME nearby SOURCES nearby.cpp)
add_dialect_test(NAME negativesepco SOURCES negativesepco.cpp)
add_dialect_test(NAME negativezero SOURCES negativezero.cpp)
add_dialect_test(NAME nodeconfig01 SOURCES nodeconfig01.cpp)
add_dialect_test(NAME nudgeopt SOURCES nudgeopt.cpp)
add_dialect_test(NAME partition01 SOURCES partition01.cpp)
add_dialect_test(NAME peel SOURCES peel.cpp)
add_dialect_test(NAME planarise01 SOURCES planarise01.cpp)
add_dialect_test(NAME planarise02 SOURCES planarise02.cpp)
add_dialect_test(NAME projseq01 SOURCES projseq01.cpp)
add_dialect_test(NAME readconstraints SOURCES readconstraints.cpp)
add_dialect_test(NAME rotate01 SOURCES rotate01.cpp)
add_dialect_test(NAME rotate02 SOURCES rotate02.cpp)
add_dialect_test(NAME rotate03 SOURCES rotate03.cpp)
add_dialect_test(NAME rotate04 SOURCES rotate04.cpp)
add_dialect_test(NAME routing01 SOURCES routing01.cpp)
add_dialect_test(NAME sep_matrix_iter SOURCES sep_matrix_iter.cpp)
add_dialect_test(NAME solidify SOURCES solidify.cpp)
add_dialect_test(NAME symmtree SOURCES symmtree.cpp)
add_dialect_test(NAME tglf01 SOURCES tglf01.cpp)
add_dialect_test(NAME treeboxes01 SOURCES treeboxes01.cpp)
add_dialect_test(NAME treeplacement01 SOURCES treeplacement01.cpp)
add_dialect_test(NAME treeplacement02 SOURCES treeplacement02.cpp)
add_dialect_test(NAME treeplacement03 SOURCES treeplacement03.cpp)
add_dialect_test(NAME trees SOURCES trees.cpp)
add_dialect_test(NAME trees2 SOURCES trees2.cpp)
add_dialect_test(NAME vpsc01 SOURCES vpsc01.cpp)
add_dialect_test(NAME holasbgn01 SOURCES holasbgn01.cpp)
add_dialect_test(NAME holasbgn02 SOURCES holasbgn02.cpp)
add_dialect_test(NAME holametro01 SOURCES holametro01.cpp)
add_dialect_test(NAME holasbgn03 SOURCES holasbgn03.cpp)
add_dialect_test(NAME holasbgn04 SOURCES holasbgn04.cpp)
add_dialect_test(NAME holaRand SOURCES holaRand.cpp)

