cmake_minimum_required(VERSION 3.30)
# set(CMAKE_VERBOSE_MAKEFILE ON)

# The version number here is the canonical one for the project.
# Run rs-bump-version to increment the patch number
# and propagate the new version to version.hpp.

project(Containers
    VERSION 0.1.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS=1)
    add_compile_options(/EHsc /Gy /MP /O2 /sdl /utf-8 /W4 /WX)
else()
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    add_compile_options(-finput-charset=UTF-8 -march=native -O2
        -Wall -Wextra -Wpedantic -Werror -Wconversion -Wshadow)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(-Wsuggest-override -Wsuggest-destructor-override)
    endif()
endif()

if(WIN32)
    add_compile_definitions(NOMINMAX=1 UNICODE=1 _UNICODE=1 WINVER=0xa00 _WIN32_WINNT=0xa00)
else()
    add_compile_definitions(_REENTRANT=1 _XOPEN_SOURCE=700)
    if(APPLE)
        add_compile_definitions(_DARWIN_C_SOURCE=1)
    endif()
endif()

find_package(Threads REQUIRED)
include_directories(.)
set(unittest test-containers)

add_executable(${unittest}
    test/bounded-array-construction-test.cpp
    test/bounded-array-insertion-test.cpp
    test/bounded-array-misc-test.cpp
    test/bounded-array-tracking-test.cpp
    test/compact-array-construction-test.cpp
    test/compact-array-insertion-test.cpp
    test/compact-array-misc-test.cpp
    test/compact-array-tracking-test.cpp
    test/multi-array-test.cpp
    test/ring-buffer-basic-test.cpp
    test/ring-buffer-static-test.cpp
    test/spatial-index-test.cpp
    test/version-test.cpp
    test/unit-test.cpp
)

target_link_libraries(${unittest}
    PRIVATE Threads::Threads
)

install(DIRECTORY ${library} DESTINATION include PATTERN "*.cpp" EXCLUDE)
