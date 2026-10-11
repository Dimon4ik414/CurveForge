cmake_minimum_required(VERSION 3.15)
project(Curve LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(FetchContent)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
)
FetchContent_MakeAvailable(raylib)


add_executable(cadex_curves
    src/main.cpp
    curve/Curve.cpp
    curve/CurveFactory.cpp
    curve/Curve.h
    curve/CurveFactory.h
    curve/IRound.h
    shapes/Ellipse.cpp
    shapes/Circle.cpp
    shapes/Helix.cpp
    shapes/Ellipse.h
    shapes/Circle.h
    shapes/Helix.h
)

target_include_directories(cadex_curves PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(cadex_curves PRIVATE raylib)