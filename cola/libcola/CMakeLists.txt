set(LIB_NAME cola)

set(LIB_SOURCES
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

set(LIB_HEADERS
    box.h
    cc_clustercontainmentconstraints.h
    cc_nonoverlapconstraints.h
    cluster.h
    cola_log.h
    cola.h
    commondefs.h
    compound_constraints.h
    conjugate_gradient.h
    connected_components.h
    convex_hull.h
    exceptions.h
    gradient_projection.h
    output_svg.h
    pseudorandom.h
    shapepair.h
    shortest_paths.h
    sparse_matrix.h
    straightener.h
    unused.h
)

add_library(${LIB_NAME}
    ${LIB_SOURCES}
)

target_sources(
    ${LIB_NAME}
    PUBLIC
    FILE_SET libcola_headers
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    TYPE HEADERS
    FILES ${LIB_HEADERS}
)

target_include_directories(${LIB_NAME}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/cola
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<${HAVE_CAIROMM}:${CAIROMM_INCLUDE_DIRS}>
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

add_dependencies(${LIB_NAME}
    vpsc
)

target_link_directories(${LIB_NAME}
    PUBLIC
    $<${HAVE_CAIROMM}:${CAIROMM_LIBRARY_DIRS}>
)

target_link_libraries(${LIB_NAME}
    PUBLIC
    $<${HAVE_CAIROMM}:${CAIROMM_LIBRARIES}>
)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

install(
    TARGETS ${LIB_NAME}
    FILE_SET libcola_headers DESTINATION include/libcola
)
