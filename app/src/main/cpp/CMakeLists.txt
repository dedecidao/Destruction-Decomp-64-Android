# Destruction Derby 64 for Android: builds libdd64.so (the game and the port's
# host runtime) and libSDL3.so, for the APK.
#
# The game and the PC host runtime come from external/dd64 (the port-pc branch
# of dedecidao/Destruction-Decomp-64), unchanged. What differs on Android lives
# in app/src/main/cpp/android:
#   android_main.c  entry point (SDL_main), log, crash handler with ARM registers
#   ram_arm.c       the N64 RAM image at 0x80000000, with ARM jump stubs
#   paths.c         the game's folder: the app's external files dir
#   touch.c         the on-screen controls, merged into player 1's pad
#   stubs.c         raphnet adapters and the sampling profiler: not on Android
#
# The symbol tables come from symbols/dd64.syms (names and N64 addresses only,
# exported once from build/dd64.elf with tools/export_symbols.py), so the APK
# builds without the ROM.
cmake_minimum_required(VERSION 3.22)
project(dd64android C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 20)

get_filename_component(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../.. ABSOLUTE)
set(DD64 ${REPO_ROOT}/external/dd64)
set(SDL_DIR ${REPO_ROOT}/external/SDL)
set(ANDROID_SRC ${CMAKE_CURRENT_SOURCE_DIR}/android)
set(SYMS_FILE ${REPO_ROOT}/symbols/dd64.syms CACHE FILEPATH "Symbols exported from build/dd64.elf")

# arm64-v8a is the target (docs/PLANO.md). Until the game's C is converted to
# 64-bit (phase 2), arm64 builds compile everything as a check but do not link
# the game; armeabi-v7a builds the full game, as a reference to compare against.
if(ANDROID_ABI STREQUAL "armeabi-v7a")
    set(DD64_64BIT OFF)
elseif(ANDROID_ABI STREQUAL "arm64-v8a")
    set(DD64_64BIT ON)
else()
    message(FATAL_ERROR "Unsupported ABI ${ANDROID_ABI}")
endif()
option(DD64_ARM64_LINK "Link the game on arm64 (once the 64-bit conversion is done)" OFF)
option(DD64_PROBE "Build only the address-space probe (android/probe.c) as libdd64.so" OFF)

# Preloaded by wrap.sh in 32-bit debug builds: reserves 0x80000000 before ART starts
# (android/reserve/dd64reserve.c, app/src/debug/resources/lib/armeabi-v7a/wrap.sh)
add_library(dd64reserve SHARED ${CMAKE_CURRENT_SOURCE_DIR}/android/reserve/dd64reserve.c)

if(DD64_PROBE)
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    add_subdirectory(${SDL_DIR} SDL EXCLUDE_FROM_ALL)
    add_library(dd64 SHARED ${CMAKE_CURRENT_SOURCE_DIR}/android/probe.c ${CMAKE_CURRENT_SOURCE_DIR}/android/paths.c)
    target_link_libraries(dd64 PRIVATE SDL3::SDL3-shared log)
    return()
endif()
if(NOT EXISTS ${DD64}/port/src/main.c)
    message(FATAL_ERROR "external/dd64 is empty: git submodule update --init --recursive")
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(GEN_DIR ${CMAKE_BINARY_DIR}/gen)
file(MAKE_DIRECTORY ${GEN_DIR})

# ---- SDL3, from the submodule ----
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
add_subdirectory(${SDL_DIR} SDL EXCLUDE_FROM_ALL)

# ---- Symbol and function tables, from symbols/dd64.syms ----
set(GEN_SYMS ${GEN_DIR}/dd64_syms.ld)
set(GEN_FIX ${GEN_DIR}/dd64_fix.c)
if(EXISTS ${SYMS_FILE} AND (NOT DD64_64BIT OR DD64_ARM64_LINK))
    add_custom_command(
        OUTPUT ${GEN_SYMS} ${GEN_FIX}
        COMMAND ${Python3_EXECUTABLE} ${REPO_ROOT}/tools/gen_from_syms.py data
                --syms ${SYMS_FILE} --dd64 ${DD64} --out ${GEN_DIR}
        DEPENDS ${REPO_ROOT}/tools/gen_from_syms.py ${DD64}/tools/port/gen_data.py ${SYMS_FILE}
        COMMENT "Generating the symbol and function tables")
    set(HAVE_SYMS ON)
else()
    # Without the symbols the code still compiles (CI checks it), but cannot link
    message(WARNING "Compile check only (no ${SYMS_FILE}, or arm64 before the 64-bit conversion)")
    set(HAVE_SYMS OFF)
endif()

# ---- Game code ----
include(${DD64}/port/sdk_excluded.cmake)
file(GLOB_RECURSE GAME_SRC CONFIGURE_DEPENDS ${DD64}/src/*.c)
foreach(f ${PORT_SDK_EXCLUDED})
    list(REMOVE_ITEM GAME_SRC ${DD64}/${f})
endforeach()
file(GLOB PORT_GAME_SRC CONFIGURE_DEPENDS ${DD64}/port/game/*.c)

set(COMMON_FLAGS
    -g -O2 -gdwarf-4
    -fno-strict-aliasing -fwrapv -fno-omit-frame-pointer
    # Plain char is signed on the N64 (KMC GCC, MIPS) and on the x86 PC build, but
    # unsigned on ARM by default
    -fsigned-char
    # No fused multiply-add: the N64 FPU rounds each step, and so must we
    -ffp-contract=off
    # Hidden symbols: the N64 addresses in dd64_syms.ld resolve at link time
    -fvisibility=hidden)

# m2c output is old C. Clang turns some GCC warnings into errors; keep them warnings.
set(GAME_LENIENT
    -w -fcommon -fgnu89-inline
    -Wno-error=implicit-function-declaration -Wno-error=implicit-int -Wno-error=int-conversion
    -Wno-error=incompatible-function-pointer-types -Wno-error=incompatible-pointer-types
    -Wno-error=return-type -Wno-error=int-to-pointer-cast)

# Every game memory access calls a hook in port/src/shadow.c (byte order). The
# GCC --param options of the PC build, spelt for clang.
set(GAME_SHADOW
    -fsanitize=kernel-address
    "SHELL:-mllvm -asan-stack=0"
    "SHELL:-mllvm -asan-globals=0"
    "SHELL:-mllvm -asan-opt-globals=0"
    "SHELL:-mllvm -asan-instrumentation-with-call-threshold=0")

add_library(game OBJECT ${GAME_SRC} ${PORT_GAME_SRC} $<$<BOOL:${HAVE_SYMS}>:${GEN_FIX}>)
target_include_directories(game PRIVATE ${ANDROID_SRC}/include ${DD64}/include ${DD64} ${DD64}/port/include)
target_compile_definitions(game PRIVATE TARGET_PC TARGET_ANDROID NON_MATCHING
    errno=dd64_errno _ctype=dd64_ctype
    acos=dd64_acos asin=dd64_asin atan=dd64_atan atan2=dd64_atan2 log=dd64_log log10=dd64_log10
    pow=dd64_pow exp=dd64_exp frexp=dd64_frexp modf=dd64_modf sinf=dd64_sinf
    cosf=dd64_cosf rand=dd64_rand atoi=dd64_atoi ldiv=dd64_ldiv lldiv=dd64_lldiv)
# ARM EABI already aligns doubles and u64s to 8 bytes in structs, as the N64 does
# (the PC build needs -malign-double for that), and passes the hidden struct
# return pointer in r0, as MIPS does in a0.
target_compile_options(game PRIVATE ${COMMON_FLAGS} ${GAME_LENIENT} ${GAME_SHADOW}
    # ARM state, not Thumb, on 32-bit: simpler crash logs while the port is young
    $<$<NOT:$<BOOL:${DD64_64BIT}>>:-marm>)

# inflate_codes (src/auto/51980.c) at -O0: built with clang -O2 for ARM it loops
# forever on the first race's zip entry (the title screen never comes; found by
# bisecting the inflate files on a Galaxy S23 Ultra). GCC -O2 on x86 is fine. It
# only runs while loading, so the speed does not matter; the clang cause is still open.
set_source_files_properties(${DD64}/src/auto/51980.c PROPERTIES COMPILE_OPTIONS -O0)

# ---- Static data conversion table (DWARF of the game objects) ----
set(GEN_WIDTHS ${GEN_DIR}/dd64_widths.c)
# clang emits no DWARF for extern variables (only for BPF), and the game declares
# its globals extern, so gen_widths.py finds almost nothing in the ARM objects
# (20 values instead of ~4800): the static data then stays big-endian. The table
# from the PC build (GCC) is used instead: i686 with -malign-double and ARM EABI
# lay the game's types out alike (4-byte pointers, 8-byte aligned doubles).
set(WIDTHS_FILE ${REPO_ROOT}/symbols/dd64_widths.c CACHE FILEPATH "Static data widths from the PC build")
if(HAVE_SYMS AND EXISTS ${WIDTHS_FILE} AND NOT DD64_64BIT)
    set(GEN_WIDTHS ${WIDTHS_FILE})
elseif(HAVE_SYMS)
    file(GENERATE OUTPUT ${GEN_DIR}/game_objects.txt CONTENT "$<JOIN:$<TARGET_OBJECTS:game>,\n>\n")
    # GNU objdump that reads 32-bit ARM ELF (gen_widths.py parses its --dwarf=info);
    # CI installs binutils-arm-linux-gnueabihf
    find_program(DD64_OBJDUMP NAMES arm-linux-gnueabihf-objdump arm-linux-gnueabi-objdump objdump)
    add_custom_command(
        OUTPUT ${GEN_WIDTHS}
        COMMAND ${Python3_EXECUTABLE} ${REPO_ROOT}/tools/gen_from_syms.py widths
                --syms ${SYMS_FILE} --dd64 ${DD64} --objdump ${DD64_OBJDUMP} --out ${GEN_WIDTHS}
                @${GEN_DIR}/game_objects.txt
        DEPENDS ${REPO_ROOT}/tools/gen_from_syms.py ${DD64}/tools/port/gen_widths.py ${SYMS_FILE}
                $<TARGET_OBJECTS:game>
        COMMENT "Generating the static data conversion table")
endif()

# ---- Port host runtime ----
file(GLOB PORT_SRC CONFIGURE_DEPENDS ${DD64}/port/src/*.c ${DD64}/port/src/*.cpp
                                     ${DD64}/port/fast3d/*.cpp ${DD64}/port/fast3d/glad/*.c)
# Replaced on Android (app/src/main/cpp/android)
foreach(f main.c ram.c profile.c raphnet.c)
    list(REMOVE_ITEM PORT_SRC ${DD64}/port/src/${f})
endforeach()
file(GLOB ANDROID_PORT_SRC CONFIGURE_DEPENDS ${ANDROID_SRC}/*.c)
list(REMOVE_ITEM ANDROID_PORT_SRC ${ANDROID_SRC}/nosyms.c ${ANDROID_SRC}/probe.c)
if(DD64_64BIT)
    # The 32-bit RAM mapping; the arm64 one comes with phase 2
    list(REMOVE_ITEM ANDROID_PORT_SRC ${ANDROID_SRC}/ram_arm.c)
endif()

set(SHADER_DIR ${DD64}/port/fast3d/shaders)
set(GEN_SHADER_SRC ${GEN_DIR}/shader_src.h)
set(SHADER_GLSL ${SHADER_DIR}/combiner.vert.glsl ${SHADER_DIR}/combiner.frag.glsl)
add_custom_command(
    OUTPUT ${GEN_SHADER_SRC}
    COMMAND ${CMAKE_COMMAND} -DOUT=${GEN_SHADER_SRC} "-DIN=${SHADER_GLSL}" -P ${DD64}/cmake/embed_text.cmake
    DEPENDS ${SHADER_GLSL} ${DD64}/cmake/embed_text.cmake
    COMMENT "Embedding the combiner shader's GLSL" VERBATIM)

add_library(port OBJECT ${PORT_SRC} ${ANDROID_PORT_SRC} ${GEN_SHADER_SRC})
target_include_directories(port PRIVATE ${ANDROID_SRC}/include ${DD64}/include ${DD64}/port/include ${DD64}/port/fast3d
                                        ${GEN_DIR} ${ANDROID_SRC})
target_compile_definitions(port PRIVATE TARGET_PC TARGET_ANDROID)
target_compile_options(port PRIVATE ${COMMON_FLAGS} -Wall -Wno-unused-parameter -Wno-unused-function
    -Wno-unused-variable -Wno-sign-compare -Wno-missing-field-initializers -Wno-format-truncation)
target_link_libraries(port PRIVATE SDL3::SDL3-shared)
# Port files whose host calls get an Android version (see each file in android/)
set_source_files_properties(${DD64}/port/src/input.c PROPERTIES COMPILE_DEFINITIONS
    "Port_InputGetPad=Port_InputGetPadHost;Port_InputConnected=Port_InputConnectedHost")
# MSAA off: black frames on the Adreno 740 with a multisample target (android/gpu_caps.c)
set_source_files_properties(${DD64}/port/fast3d/gfx_sdlgpu.cpp PROPERTIES COMPILE_DEFINITIONS
    "SDL_GPUTextureSupportsSampleCount=DD64A_GPUTextureSupportsSampleCount")
set_source_files_properties(${DD64}/port/src/config.c ${DD64}/port/src/rom.c PROPERTIES COMPILE_DEFINITIONS
    "SDL_GetBasePath=DD64A_GetBasePath")

if(HAVE_SYMS)
    add_library(dd64 SHARED ${GEN_WIDTHS} $<TARGET_OBJECTS:game> $<TARGET_OBJECTS:port>)
    target_include_directories(dd64 PRIVATE ${DD64}/include ${DD64}/port/include)
    target_compile_definitions(dd64 PRIVATE TARGET_PC TARGET_ANDROID)
    target_link_libraries(dd64 PRIVATE SDL3::SDL3-shared android log m dl)
    target_link_options(dd64 PRIVATE
        # Block copies keep the big-endian marks with the bytes (port/src/shadow.c)
        -Wl,--wrap=memcpy,--wrap=memmove,--wrap=memset
        # Every N64 data symbol at its N64 address
        -Wl,${GEN_SYMS}
        -Wl,--version-script=${ANDROID_SRC}/exports.map
        -Wl,--no-undefined)
    set_target_properties(dd64 PROPERTIES LINK_DEPENDS "${GEN_SYMS};${ANDROID_SRC}/exports.map")
else()
    # Compile check only
    add_library(dd64 SHARED ${ANDROID_SRC}/nosyms.c)
    add_dependencies(dd64 game port)
    target_link_libraries(dd64 PRIVATE SDL3::SDL3-shared log)
endif()
