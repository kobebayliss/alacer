cmake_minimum_required(VERSION 3.16)

project(alacer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
    ixwebsocket
    GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
    GIT_TAG        v11.4.5
)

set(USE_TLS ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(ixwebsocket)

find_package(cpr REQUIRED)
find_package(RapidJSON REQUIRED)
add_executable(alacer
    src/main.cpp
    src/book/OrderBook.cpp
    src/book/OrderBookBuilder.cpp
    src/book/OrderBookDisplay.cpp
    src/feed/WebSocketClient.cpp
    src/feed/EventHandler.cpp
)
target_include_directories(alacer PRIVATE /opt/homebrew/include)
target_compile_options(alacer PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_link_libraries(alacer PRIVATE 
	cpr::cpr
	ixwebsocket
)
