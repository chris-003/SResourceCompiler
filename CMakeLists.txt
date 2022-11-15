cmake_minimum_required(VERSION 3.24)

project(SResourceCompiler LANGUAGES CXX)
aux_source_directory(./src srcs)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(SResourceCompiler ${srcs})

target_link_libraries(SResourceCompiler PUBLIC ${Boost_LIBRARIES} boost_filesystem)
