cmake_minimum_required(VERSION 3.28)

project(adaptagrams)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} CACHE PATH "" FORCE)

option(BUILD_SWIG_PYTHON "Enable building swig python wrapper")
option(BUILD_TESTS "Enable building tests")
option(ENABLE_CCACHE "Use ccache if available" ON)

# use ccache if available
if(ENABLE_CCACHE)
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        message(STATUS "Found ccache in ${CCACHE_PROGRAM}")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    endif()
endif()

message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")

# optional cairo package
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(package_name cairomm-1.16)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(package_name cairomm-svg-1.16)
else()
    set(HAVE_CAIROMM 0 CACHE BOOL "" FORCE)
endif()

if(DEFINED package_name)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(CAIROMM ${package_name})
        if(CAIROMM_FOUND)
            set(HAVE_CAIROMM 1 CACHE BOOL "have cairomm on system" FORCE)
            message(STATUS "CAIROMM_INCLUDE_DIRS: ${CAIROMM_INCLUDE_DIRS}")
            message(STATUS "CAIROMM_LIBRARY_DIRS: ${CAIROMM_LIBRARY_DIRS}")
            message(STATUS "CAIROMM_LIBRARIES: ${CAIROMM_LIBRARIES}")
            message(STATUS "CAIROMM_CFLAGS: ${CAIROMM_CFLAGS}")
            message(STATUS "CAIROMM_LDFLAGS: ${CAIROMM_LDFLAGS}")
        else()
            message(STATUS "${package_name} NOT FOUND")
            set(HAVE_CAIROMM 0 CACHE BOOL "" FORCE)
        endif()
    else()
        message(STATUS "PkgConfig NOT FOUND")
        set(HAVE_CAIROMM 0 CACHE BOOL "" FORCE)
    endif()
endif()

include(CheckIncludeFile)

CHECK_INCLUDE_FILE(dlfcn.h HAVE_DLFCN_H)
CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILE(stdio.h HAVE_STDIO_H)
CHECK_INCLUDE_FILE(stdlib.h HAVE_STDLIB_H)
CHECK_INCLUDE_FILE(strings.h HAVE_STRINGS_H)
CHECK_INCLUDE_FILE(string.h HAVE_STRING_H)
CHECK_INCLUDE_FILE(sys/stat.h HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE(sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)

configure_file(
    ${CMAKE_SOURCE_DIR}/cola/libcola/config.h.cmake.in 
    ${CMAKE_SOURCE_DIR}/cola/libcola/config.h
)

if(BUILD_TESTS)
    enable_testing()
endif()


add_subdirectory(cola)


if(BUILD_SWIG_PYTHON)
    find_package(SWIG COMPONENTS python)
    find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
    if(SWIG_FOUND AND Python_FOUND)
        message(STATUS "building adaptagrams swig python extension")
        set(generated_file ${CMAKE_BINARY_DIR}/adaptagrams_wrap.cxx)
        execute_process(
            OUTPUT_FILE ${CMAKE_BINARY_DIR}/adaptagrams_wrap.cxx
            COMMAND ${SWIG_EXECUTABLE} -DNDEBUG -c++ -python
                    -outdir ${CMAKE_BINARY_DIR}
                    -o ${generated_file} adaptagrams.i
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/cola
        )
        if(NOT EXISTS ${generated_file})
            message(FATAL_ERROR "did not generate: ${generated_file}")
        endif()

        python_add_library(
            _adaptagrams
            MODULE WITH_SOABI
            ${generated_file}
        )

        set_target_properties(
            _adaptagrams
            PROPERTIES
            CXX_STANDARD 11 
            CXX_STANDARD_REQUIRED ON 
            CXX_EXTENSIONS OFF
        )

        target_compile_definitions(
            _adaptagrams
            PRIVATE
            USE_ASSERT_EXCEPTIONS
            SWIG_PYTHON_SILENT_MEMLEAK
        )

        target_include_directories(
            _adaptagrams
            PUBLIC
            ${CMAKE_SOURCE_DIR}/cola
        )

        target_link_libraries(
            _adaptagrams
            PRIVATE
            avoid
            cola
            dialect
            topology
            vpsc
        )

        install(TARGETS _adaptagrams DESTINATION lib/adaptagrams)
        install(FILES ${CMAKE_SOURCE_DIR}/cola/__init__.py DESTINATION lib/adaptagrams)
        install(FILES ${CMAKE_BINARY_DIR}/adaptagrams.py DESTINATION lib/adaptagrams)
    else()
        message(FATAL_ERROR "NotFoundError: SWIG: ${SWIG_FOUND} Python: ${Python_FOUND}")
    endif()
endif()
