cmake_minimum_required(VERSION 3.14)
project(threadkit)

option(THREADKIT_BUILD_TESTS "build tests" ON)

file(GLOB THREADKIT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(threadkit_static STATIC ${THREADKIT_SRC})
target_include_directories(threadkit_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(threadkit_static PUBLIC cxx_std_11)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(threadkit_static PUBLIC pthread)
endif()

if(THREADKIT_BUILD_TESTS)
    add_subdirectory(tests)
endif()
