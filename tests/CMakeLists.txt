cmake_minimum_required(VERSION 3.14)
project(threadkit-test)

add_executable(test_threadpool test_thread_pool.cpp)
target_link_libraries(test_threadpool PRIVATE threadkit_static)
