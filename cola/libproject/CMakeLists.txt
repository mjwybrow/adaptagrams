set(LIB_NAME project)

set(LIB_SOURCES
    project.cpp
    util.cpp
)

set(LIB_HEADERS
    project_log.h
    project.h
    util.h
    variable.h
)

add_library(${LIB_NAME}
    ${LIB_SOURCES}
)

target_sources(
    ${LIB_NAME}
    PUBLIC
    FILE_SET libproject_headers
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    TYPE HEADERS
    FILES ${LIB_HEADERS}
)

target_include_directories(${LIB_NAME}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/cola
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(
    ${LIB_NAME} 
    PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

target_compile_definitions(${LIB_NAME}
    PUBLIC
    ${COMMON_COMPILE_DEFINITIONS}
)

target_link_options(${LIB_NAME}
    PUBLIC
    ${COMMON_LINK_OPTIONS}
)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

install(
    TARGETS ${LIB_NAME}
    FILE_SET libproject_headers DESTINATION include/libproject
)
