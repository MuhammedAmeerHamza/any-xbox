cmake_minimum_required(VERSION 3.20)
project(AnyXbox VERSION 0.10.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(xboxlab_core
    src/format_probe.cpp
    src/package_probe.cpp
    src/pe_inspector.cpp
    src/guest_memory.cpp
    src/guest_cpu.cpp
    src/guest_timing.cpp
    src/guest_exception.cpp
    src/guest_interrupt.cpp
    src/guest_device.cpp
    src/manifest_probe.cpp
    src/x64_interpreter.cpp
)
target_include_directories(xboxlab_core PUBLIC include)

function(anyxbox_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

anyxbox_enable_warnings(xboxlab_core)

add_executable(xboxlab src/main.cpp)
target_link_libraries(xboxlab PRIVATE xboxlab_core)
anyxbox_enable_warnings(xboxlab)

include(CTest)
if(BUILD_TESTING)
    add_executable(xboxlab_tests tests/test_format_probe.cpp)
    target_link_libraries(xboxlab_tests PRIVATE xboxlab_core)
    anyxbox_enable_warnings(xboxlab_tests)
    add_test(NAME xboxlab_tests COMMAND xboxlab_tests)
    add_test(NAME xboxlab_help COMMAND xboxlab --help)
    add_test(NAME xboxlab_manifest_fixture
        COMMAND xboxlab --inspect-manifest ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/microsoftgame_config_synthetic.xml)
    add_test(NAME xboxlab_guest_demo
        COMMAND ${CMAKE_COMMAND}
            -DPROGRAM=$<TARGET_FILE:xboxlab>
            -DINPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/guest_demo_input.txt
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_guest_demo.cmake)
endif()

option(ANYXBOX_BUILD_FUZZER "Build the libFuzzer parser target (Clang only)" OFF)
if(ANYXBOX_BUILD_FUZZER)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "ANYXBOX_BUILD_FUZZER requires Clang/libFuzzer.")
    endif()
    target_compile_options(xboxlab_core PRIVATE -fsanitize=fuzzer-no-link,address,undefined -fno-omit-frame-pointer)
    # The core library is sanitizer-instrumented, so every consumer must link
    # the sanitizer runtimes even when it is not the libFuzzer executable.
    target_link_options(xboxlab_core INTERFACE -fsanitize=address,undefined)
    add_executable(xboxlab_fuzz tests/fuzz_parsers.cpp)
    target_link_libraries(xboxlab_fuzz PRIVATE xboxlab_core)
    anyxbox_enable_warnings(xboxlab_fuzz)
    target_compile_options(xboxlab_fuzz PRIVATE -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer)
    target_link_options(xboxlab_fuzz PRIVATE -fsanitize=fuzzer,address,undefined)
endif()
