cmake_minimum_required(VERSION 3.2)
project(queue)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")

set(SOURCE_FILES_QUEUE_BLOCKING blocking_queue.c)
set(SOURCE_FILES_LOCK_FREE_STACK lock_free_stack.c)

add_executable(blocking_queue ${SOURCE_FILES_QUEUE_BLOCKING})
add_executable(lock_free_stack ${SOURCE_FILES_LOCK_FREE_STACK})