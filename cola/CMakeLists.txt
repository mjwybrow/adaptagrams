
set(COMMON_COMPILE_DEFINITIONS
    # $<$<PLATFORM_ID:Windows>:_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS>
    # $<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS>
    # $<$<PLATFORM_ID:Windows>:LIBAVOID_NO_DLL>
    # $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0x0501>
    # $<$<PLATFORM_ID:Windows>:_USE_MATH_DEFINES>

    # libavoid
    $<$<PLATFORM_ID:Windows>:NDEBUG>
    $<$<PLATFORM_ID:Windows>:NOMINMAX>
    $<$<PLATFORM_ID:Windows>:WIN32>
    $<$<PLATFORM_ID:Windows>:_WINDOWS>
    $<$<PLATFORM_ID:Windows>:_USRDLL>
    $<$<PLATFORM_ID:Windows>:LIBAVOID_EXPORTS>

    $<$<PLATFORM_ID:Unix>:-Wall>
    $<$<PLATFORM_ID:Unix>:-Wpointer-arith>
    $<$<PLATFORM_ID:Unix>:-Wcast-align>
    $<$<PLATFORM_ID:Unix>:-Wsign-compare>
    $<$<PLATFORM_ID:Unix>:-Woverloaded-virtual>
    $<$<PLATFORM_ID:Unix>:-Wswitch>
)

add_subdirectory(libavoid)
add_subdirectory(libcola)
add_subdirectory(libdialect)
add_subdirectory(libproject)
add_subdirectory(libtopology)
add_subdirectory(libvpsc)

