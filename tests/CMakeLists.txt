cmake_minimum_required(VERSION 3.14)
project(threadkit-test)

add_executable(test_threadpool thread_pool_test.cpp)
target_link_libraries(test_threadpool PRIVATE threadkit_static)
