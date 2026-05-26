cmake_minimum_required(VERSION 3.15)
project(server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Drogon REQUIRED)

add_executable(server 
    src/main.cpp
    src/AuthController.cpp
    src/BusinessController.cpp
)

target_include_directories(server PRIVATE include)

target_link_libraries(server
    Drogon::Drogon
)