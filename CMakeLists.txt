cmake_minimum_required(VERSION 3.17)
project(DataFrame)

set(CMAKE_CXX_STANDARD 14)

add_executable(DataFrame main.cpp Objects/source/DataFrame.cpp Objects/source/Column.cpp)