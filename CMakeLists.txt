cmake_minimum_required(VERSION 3.14)
project(threadkit)

# ----- options ----- #

option(THREADKIT_BUILD_TESTS "build tests" ON)
option(THREADKIT_INSTALL "install headers and libs" ON)

# ----- targets ----- #

file(GLOB_RECURSE __THREADKIT_SRC__ ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(threadkit_static STATIC ${__THREADKIT_SRC__})
unset(__THREADKIT_SRC__)

target_include_directories(threadkit_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(threadkit_static PUBLIC cxx_std_11)

if(MSVC)
    target_compile_options(threadkit_static PRIVATE /W4)
else()
    target_compile_options(threadkit_static PRIVATE -Wall -Wextra -Werror)
endif()

# ----- dependencies ----- #

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(threadkit_static PUBLIC pthread)
endif()

# ----- installations ----- #

if(THREADKIT_INSTALL)
    install(DIRECTORY include DESTINATION .)
    install(TARGETS threadkit_static DESTINATION lib)
endif()

# ----- tests ----- #

if(THREADKIT_BUILD_TESTS)
    add_subdirectory(tests)
endif()
