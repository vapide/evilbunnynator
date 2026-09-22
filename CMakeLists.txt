# Created using ChatGPT, on 09/13/2026.
# I have yet to learn CMake.

cmake_minimum_required(VERSION 3.15)

project(engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# build time with cmake --build <dir> --config Release
# Without --config is debug
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(EB_LTO "Whole-program optimization (/GL + /LTCG, MSVC only)" ON)

set(EB_ARCH "avx2" CACHE STRING
    "Target ISA tier: x86-64 | sse42 | avx2 | avx512 | native")
set_property(CACHE EB_ARCH PROPERTY STRINGS x86-64 sse42 avx2 avx512 native)

if(MSVC)

    if(EB_ARCH STREQUAL "x86-64")
        set(EB_ARCH_FLAGS "")
        set(EB_ARCH_REQUIRES "x86-64 baseline (SSE2)")
    elseif(EB_ARCH STREQUAL "sse42")
        set(EB_ARCH_FLAGS /arch:SSE4.2)
        set(EB_ARCH_REQUIRES "SSE4.1 + SSE4.2 + POPCNT")
    elseif(EB_ARCH STREQUAL "avx2")
        set(EB_ARCH_FLAGS /arch:AVX2)
        set(EB_ARCH_REQUIRES "AVX + AVX2 + FMA + POPCNT")
    elseif(EB_ARCH STREQUAL "avx512")
        set(EB_ARCH_FLAGS /arch:AVX512)
        set(EB_ARCH_REQUIRES "AVX-512 F + CD + BW + DQ + VL")
    elseif(EB_ARCH STREQUAL "native")
        set(EB_ARCH_FLAGS /arch:AVX2)   # MSVC has no true "native"
        set(EB_ARCH_REQUIRES "AVX + AVX2 + FMA + POPCNT")
    else()
        message(FATAL_ERROR "Unknown EB_ARCH '${EB_ARCH}'")
    endif()

    add_compile_options(
        /W4         # High warning level
        /EHsc       # Standard C++ exceptions
        /MP         # Multi-process compilation
        ${EB_ARCH_FLAGS}
    )

else()

    if(EB_ARCH STREQUAL "x86-64")
        set(EB_ARCH_FLAGS -march=x86-64)
        set(EB_ARCH_REQUIRES "x86-64 baseline (SSE2)")
    elseif(EB_ARCH STREQUAL "sse42")
        set(EB_ARCH_FLAGS -march=x86-64-v2)
        set(EB_ARCH_REQUIRES "SSE4.1 + SSE4.2 + POPCNT")
    elseif(EB_ARCH STREQUAL "avx2")
        set(EB_ARCH_FLAGS -march=x86-64-v3)
        set(EB_ARCH_REQUIRES "AVX + AVX2 + BMI/BMI2 + FMA + POPCNT")
    elseif(EB_ARCH STREQUAL "avx512")
        set(EB_ARCH_FLAGS -march=x86-64-v4)
        set(EB_ARCH_REQUIRES "AVX-512 F + CD + BW + DQ + VL")
    elseif(EB_ARCH STREQUAL "native")
        set(EB_ARCH_FLAGS -march=native)
        set(EB_ARCH_REQUIRES "whatever built it - NOT for release")
    else()
        message(FATAL_ERROR "Unknown EB_ARCH '${EB_ARCH}'")
    endif()

    # The ISA tier applies to every configuration, as /arch does on MSVC.
    add_compile_options(
        -Wall
        -Wextra
        ${EB_ARCH_FLAGS}
    )

    add_compile_options(
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Debug>:-O0>

        $<$<CONFIG:Release>:-O3>
    )

endif()

message(STATUS "ISA tier: ${EB_ARCH} (requires ${EB_ARCH_REQUIRES})")

set(ENGINE_SOURCES

    src/core/bitboard.cpp
    src/core/move.cpp
    src/core/position.cpp

    src/engine/engine.cpp

    src/movegen/attacks.cpp
    src/movegen/kingsafety.cpp
    src/movegen/magics.cpp
    src/movegen/movegen.cpp
    src/movegen/perft.cpp
    src/movegen/raytables.cpp

    src/evaluation/material.cpp

    src/search/negamax.cpp
    src/search/search.cpp
    src/search/eval_shaping.cpp
    src/search/quiescence.cpp


    #src/nnue/nnue.cpp


    src/uci/uci.cpp

    src/utils/paths.cpp
    src/utils/san.cpp
    src/utils/zobrist.cpp
)

###############################################################################
# Engine
###############################################################################

add_executable(engine
    src/main.cpp
    ${ENGINE_SOURCES}
)

target_include_directories(engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(MSVC)
    target_link_options(engine PRIVATE /STACK:8388608)
elseif(MINGW)
    target_link_options(engine PRIVATE -Wl,--stack,8388608)
elseif(APPLE)
    target_link_options(engine PRIVATE -Wl,-stack_size,0x800000)
endif()

###############################################################################
# Release Optimizations
###############################################################################

if(MSVC)

    # Release codegen, as measured on the full engine's `bench` suite:
    #   /O2 /Oi /Ot   speed-favouring optimization, intrinsics
    #   /GL + /LTCG   whole-program optimization (EB_LTO, default ON)
    #   /Gy /GF       COMDAT functions + string pooling, so the linker's
    #                 /OPT:REF can actually drop what is unused
    #   /Ob3          more aggressive inlining
    #
    # Tested and rejected there: PGO (slower), /Gw (noise), /GS- (no gain,
    # removes stack cookies), /GR- (noise). /fp:fast deliberately untested.
    target_compile_options(engine PRIVATE

        $<$<CONFIG:Release>:
            /O2
            /Oi
            /Ot
            /Ob3
            /Gy
            /GF
            $<$<BOOL:${EB_LTO}>:/GL>
        >
    )

    target_link_options(engine PRIVATE

        $<$<CONFIG:Release>:
            $<$<BOOL:${EB_LTO}>:/LTCG>
            /OPT:REF    # drop unreferenced functions/data (pairs with /Gy /GF)
            /OPT:ICF    # fold identical COMDATs
        >
    )

endif()
