cmake_minimum_required(VERSION 3.20)
project(AnyXbox VERSION 0.8.0 LANGUAGES CXX)

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
)
target_include_directories(xboxlab_core PUBLIC include)

if(MSVC)
    target_compile_options(xboxlab_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(xboxlab_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(xboxlab src/main.cpp)
target_link_libraries(xboxlab PRIVATE xboxlab_core)

include(CTest)
if(BUILD_TESTING)
    add_executable(xboxlab_tests tests/test_format_probe.cpp)
    target_link_libraries(xboxlab_tests PRIVATE xboxlab_core)
    add_test(NAME xboxlab_tests COMMAND xboxlab_tests)
    add_test(NAME xboxlab_help COMMAND xboxlab --help)
endif()
