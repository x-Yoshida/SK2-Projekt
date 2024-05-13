cmake_minimum_required(VERSION 3.22)
project(projekt_sk2)

set(CMAKE_CXX_STANDARD 17)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/countries.csv DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/cities.csv DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/names.csv DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(server.bin src/server/server.cpp src/server/handler.cpp src/server/room.cpp)
