#pragma once

#include "raylib.h"
#include "rlgl.h"       // rlPushMatrix(), rlTranslatef(), rlRotatef()
#include "curve/Curve.h"

namespace motion
{
    // Положение и ориентация фигуры в сцене.
    // Углы хранятся в градусах, как их принимает rlRotatef().
    struct Transform
    {
        Vector3 position = { 0.0f, 0.0f, 0.0f };
        float rotX = 0.0f;
        float rotY = 0.0f;
        float rotZ = 0.0f;

        // Точка, вокруг которой вращается фигура (центр фигуры).
        Vector3 pivot = { 0.0f, 0.0f, 0.0f };

        // Накладывает трансформацию на текущую матрицу.
        // Вызывать внутри BeginMode3D(), после отрисовки обязателен pop().
        void push() const
        {
            rlPushMatrix();

            rlTranslatef(position.x, position.y, position.z);

            // Переносим начало координат в пивот, крутим вокруг него,
            // возвращаемся обратно: так фигура вращается вокруг себя.
            rlTranslatef(pivot.x, pivot.y, pivot.z);

            rlRotatef(rotZ, 0.0f, 0.0f, 1.0f);
            rlRotatef(rotX, 1.0f, 0.0f, 0.0f);
            rlRotatef(rotY, 0.0f, 1.0f, 0.0f);

            rlTranslatef(-pivot.x, -pivot.y, -pivot.z);
        }

        void pop() const
        {
            rlPopMatrix();
        }
    };

    // Центр фигуры: среднее арифметическое по точкам кривой.
    // Нужен как пивот, чтобы фигура вращалась вокруг себя,
    // а не вокруг начала координат.
    inline Vector3 curveCenter(const Curve& curve, double turns, int steps = 256)
    {
        constexpr double P = 3.14159265358979323846;

        const double tEnd = turns * 2.0 * P;
        double sx = 0.0;
        double sy = 0.0;
        double sz = 0.0;

        for (int i = 0; i <= steps; ++i)
        {
            const std::array<double, 3> p = curve.point(tEnd * i / steps);
            sx += p[0];
            sy += p[1];
            sz += p[2];
        }

        const double n = static_cast<double>(steps + 1);

        return Vector3{
            static_cast<float>(sx / n),
            static_cast<float>(sy / n),
            static_cast<float>(sz / n)
        };
    }
}
