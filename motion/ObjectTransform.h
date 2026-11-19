#pragma once

#include "raylib.h"
#include "rlgl.h"
#include "curve/Curve.h"
#include "curve/CurveConfig.h"

namespace motion
{
    struct Transform
    {
        Vector3 position = { 0.0f, 0.0f, 0.0f };
        float rotX = 0.0f;
        float rotY = 0.0f;
        float rotZ = 0.0f;
        Vector3 pivot = { 0.0f, 0.0f, 0.0f };

        void resetPose()
        {
            position = { 0.0f, 0.0f, 0.0f };
            rotX = 0.0f;
            rotY = 0.0f;
            rotZ = 0.0f;
        }

        void push() const
        {
            rlPushMatrix();
            rlTranslatef(position.x, position.y, position.z);
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

    inline Vector3 curveCenter(const Curve& curve, double turns = curve_cfg::DEFAULT_TURNS, int steps = curve_cfg::CURVE_SEGMENTS)
    {
        const double tEnd = turns * curve_cfg::TWO_PI;
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