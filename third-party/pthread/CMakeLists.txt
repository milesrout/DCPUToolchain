cmake_minimum_required(VERSION 2.8.5)

if(MSVC)
    set(pthread_LIBRARY ${toolchain_SOURCE_DIR}/third-party/pthread/lib/pthreadVC2.lib PARENT_SCOPE)
    set(pthread_SOURCE_DIR ${toolchain_SOURCE_DIR}/third-party/pthread/include PARENT_SCOPE)
else()
    find_package(Threads REQUIRED)
    set(pthread_LIBRARY ${Threads_LIBRARY} PARENT_SCOPE)
    set(pthread_SOURCE_DIR ${Threads_INCLUDE_DIR} PARENT_SCOPE)
endif()
