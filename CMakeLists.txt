cmake_minimum_required(VERSION 3.15)
project(Curve LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(RAYLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/raylib)

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


target_include_directories(cadex_curves PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${RAYLIB_DIR}/include
)

target_link_libraries(cadex_curves PRIVATE
    ${RAYLIB_DIR}/lib/raylib.lib
    winmm
)


add_custom_command(TARGET cadex_curves POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${RAYLIB_DIR}/lib/raylib.dll
        $<TARGET_FILE_DIR:cadex_curves>
)
