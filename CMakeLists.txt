cmake_minimum_required(VERSION 3.16)

project(alacer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(cpr REQUIRED)
find_package(RapidJSON REQUIRED)

add_executable(alacer
    src/main.cpp
    src/OrderBook.cpp
    src/OrderBookBuilder.cpp
    src/OrderBookDisplay.cpp
)

target_compile_options(alacer PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_include_directories(alacer SYSTEM PRIVATE /opt/homebrew/include)

target_link_libraries(alacer PRIVATE 
	cpr::cpr
)
