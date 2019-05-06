cmake_minimum_required(VERSION 3.14)
project(oil_storage_manage_system C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS -pthread)

add_executable(oil_storage_manage_system main.c storage_tank.h pump.h pump.c storage_tank.c oil_storage.h oil_storage_def.h oil_storage.c oil_storage_interface.h oil_storage_interface.c)