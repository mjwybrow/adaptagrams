
# libproject tests

function(add_project_test)

    set(options)

    set(oneValueArgs
        NAME
    )

    set(multiValueArgs
      SOURCES
    )

    cmake_parse_arguments(
        PROJECT_TEST 
        "${options}" "${oneValueArgs}"
        "${multiValueArgs}" ${ARGN}
    )

    add_executable(${PROJECT_TEST_NAME}
        ${PROJECT_TEST_SOURCES}
    )

    target_include_directories(${PROJECT_TEST_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}/cola
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_link_libraries(${PROJECT_TEST_NAME}
        PUBLIC
        project
    )

    add_test(NAME ${PROJECT_TEST_NAME}
         COMMAND ${PROJECT_TEST_NAME}
    )

endfunction()


set(testutil_SOURCES testutil.cpp quadprogpp/QuadProg++.cc)

add_project_test(NAME simple SOURCES simple.cpp)
add_project_test(NAME static SOURCES static.cpp ${testutil_SOURCES})
add_project_test(NAME random SOURCES random.cpp ${testutil_SOURCES})
add_project_test(NAME treeqptest SOURCES treeqptest.cpp treeqp.cpp ${testutil_SOURCES})
add_project_test(NAME quadprogpptest 
    SOURCES
    quadprogpp/main.cc
    quadprogpp/QuadProg++.h
    quadprogpp/QuadProg++.cc
)

