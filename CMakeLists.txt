cmake_minimum_required(VERSION 3.16)
project(ChessEngine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The engine itself: rules + brain, reusable anywhere
add_library(chesscore src/engine.cpp)
target_include_directories(chesscore PUBLIC src)

# The terminal game, which uses the engine
add_executable(engine src/main.cpp)
target_link_libraries(engine PRIVATE chesscore)