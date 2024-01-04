cmake_minimum_required(VERSION 3.27)
project(projekt_sk2)

set(CMAKE_CXX_STANDARD 17)

add_executable(server.bin src/server/server.cpp src/server/handler.cpp)