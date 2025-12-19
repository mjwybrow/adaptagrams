set(LIB_NAME avoid)

set(LIB_SOURCES
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

set(LIB_HEADERS
    actioninfo.h
    assertions.h
    connectionpin.h
    connector.h
    connend.h
    debug.h
    debughandler.h
    dllexport.h
    geometry.h
    geomtypes.h
    graph.h
    hyperedge.h
    hyperedgeimprover.h
    hyperedgetree.h
    junction.h
    libavoid.h
    makepath.h
    mtst.h
    obstacle.h
    orthogonal.h
    router.h
    scanline.h
    shape.h
    timer.h
    vertices.h
    viscluster.h
    visibility.h
    vpsc.h
)


add_library(${LIB_NAME}
    ${LIB_SOURCES}
)

target_sources(
    ${LIB_NAME}
    PUBLIC
    FILE_SET libavoid_headers
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
    FILE_SET libavoid_headers DESTINATION include/libavoid
)
