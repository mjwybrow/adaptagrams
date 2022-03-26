cmake_minimum_required(VERSION 3.16)

project(vpsc)

add_library(${PROJECT_NAME}
    block.cpp
    blocks.cpp
    cbuffer.cpp
    constraint.cpp
    rectangle.cpp
    solve_VPSC.cpp
    variable.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../)

if (ENABLE_TESTS)
    # TODO: other test cases
    add_executable(test_${PROJECT_NAME}_block tests/block.cpp)
    target_include_directories(test_${PROJECT_NAME}_block PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../)
    target_link_libraries(test_${PROJECT_NAME}_block ${PROJECT_NAME})
    target_compile_definitions(test_${PROJECT_NAME}_block PRIVATE -DIMAGE_OUTPUT_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/tests/\" -DTEST_DATA_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/tests/\")
    add_test(NAME test_${PROJECT_NAME}_block
            COMMAND test_${PROJECT_NAME}_block)
endif()
