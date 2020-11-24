cmake_minimum_required(VERSION 3.15)
project(compress2)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(ZLIB)
find_package(Boost)

include_directories(include)

add_library(comp src/comp/deflate.cpp src/comp/comp.cpp src/comp/bitpack.cpp src/comp/composition.cpp src/comp/frontend.cpp src/comp/delta.cpp)
target_link_libraries(comp ZLIB::ZLIB)

add_library(gendata src/gendata/gendata.cpp src/gendata/tight_loop.cpp)

add_library(thcomp src/thcomp/thcomp.cpp)
target_link_libraries(thcomp PRIVATE Threads::Threads)

add_executable(encoder src/encoder.cpp)
target_link_libraries(encoder thcomp comp)

add_executable(generator src/generator.cpp)
target_link_libraries(generator gendata)

