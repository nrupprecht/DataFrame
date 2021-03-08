cmake_minimum_required(VERSION 3.17)
project(DataFrame)

set(CMAKE_CXX_STANDARD 14)

add_executable(DataFrame main.cpp Objects/DataFrame.cpp Objects/Column.cpp Objects/Concrete.cpp)