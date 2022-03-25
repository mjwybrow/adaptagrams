cmake_minimum_required(VERSION 3.16)
project(adaptagrams)

option(ENABLE_TESTS "Build tests" OFF)

if (ENABLE_TESTS)
    # GoogleTest requires at least C++11
    set(CMAKE_CXX_STANDARD 11)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/af29db7ec28d6df1c7f0f745186884091e602e07.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    include(GoogleTest)
endif()

add_subdirectory(libavoid)
add_subdirectory(libcola)
add_subdirectory(libtopology)
add_subdirectory(libvpsc)
add_subdirectory(libdialect)
