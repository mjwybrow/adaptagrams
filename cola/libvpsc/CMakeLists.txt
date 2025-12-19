set(LIB_NAME vpsc)

set(LIB_SOURCES
	block.cpp
	blocks.cpp
	cbuffer.cpp
	constraint.cpp
	rectangle.cpp
	solve_VPSC.cpp
	variable.cpp
)

set(LIB_HEADERS
	assertions.h
	block.h
	blocks.h
	cbuffer.h
	constraint.h
	exceptions.h
	linesegment.h
	pairing_heap.h
	rectangle.h
	solve_VPSC.h
	variable.h
)

add_library(${LIB_NAME}
	${LIB_SOURCES}
)

target_sources(
	${LIB_NAME}
	PUBLIC
	FILE_SET libvpsc_headers
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
    FILE_SET libvpsc_headers DESTINATION include/libvpsc
)
