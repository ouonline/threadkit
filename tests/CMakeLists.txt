cmake_minimum_required(VERSION 3.14)
project(threadkit-test)

add_executable(test_threadpool test_threadpool.cpp)
target_link_libraries(test_threadpool PRIVATE threadkit_static)

add_executable(test_mpsc_queue test_mpsc_queue.cpp)
target_link_libraries(test_mpsc_queue PRIVATE threadkit_static)

add_executable(test_event_count test_event_count.cpp)
target_link_libraries(test_event_count PRIVATE threadkit_static)

add_executable(test_scheduler test_scheduler.cpp)
target_link_libraries(test_scheduler PRIVATE threadkit_static)

add_executable(test_mutex test_mutex.cpp)
target_link_libraries(test_mutex PRIVATE threadkit_static)
