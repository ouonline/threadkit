cmake_minimum_required(VERSION 3.14)
project(threadkit)

# ----- options ----- #

option(THREADKIT_BUILD_TESTS "build tests" ON)
option(THREADKIT_INSTALL "install headers and libs" ON)
option(THREADKIT_HOLD_DEPS "do not update existing deps" OFF)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 14)
endif()

# ----- targets ----- #

file(GLOB_RECURSE __THREADKIT_SRC__ ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(threadkit_static STATIC ${__THREADKIT_SRC__})
unset(__THREADKIT_SRC__)

target_include_directories(threadkit_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(MSVC)
    target_compile_options(threadkit_static PRIVATE /W4)
else()
    target_compile_options(threadkit_static PRIVATE -Wall -Wextra -Werror)
endif()

# ----- dependencies ----- #

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(threadkit_static PUBLIC pthread)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(threadkit_static PUBLIC synchronization)
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
