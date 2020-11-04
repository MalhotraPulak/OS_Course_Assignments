cmake_minimum_required(VERSION 3.17)
project(Assignment6)

set(CMAKE_CXX_STANDARD 14)

add_executable(Assignment6
        codes/client.c
        codes/server.c
        codes/header.c codes/header.h)

