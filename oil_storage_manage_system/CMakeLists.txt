cmake_minimum_required(VERSION 3.14)
project(oil_storage_manage_system C)

set(CMAKE_C_STANDARD 99)

add_executable(oil_storage_manage_system main.c storage_tank.h pump.h)