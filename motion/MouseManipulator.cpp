#include "motion/MouseManipulator.h"

#include <cmath>

namespace
{
    Vector3 subtract(Vector3 a, Vector3 b)
    {
        return Vector3{ a.x - b.x, a.y - b.y, a.z - b.z };
    }

    Vector3 cross(Vector3 a, Vector3 b)
    {
        return Vector3{
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    Vector3 normalize(Vector3 v)
    {
        const float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);

        if (len < 1e-6f)
        {
            return v;
        }

        return Vector3{ v.x / len, v.y / len, v.z / len };
    }

    // Держит углы в диапазоне (-180, 180], чтобы значение не росло бесконечно.
    float wrapAngle(float angle)
    {
        float result = fmodf(angle, 360.0f);

        if (result > 180.0f)
        {
            result -= 360.0f;
        }

        if (result <= -180.0f)
        {
            result += 360.0f;
        }

        return result;
    }
}

void motion::MouseManipulator::update(Transform& transform, const Camera3D& camera) const
{
    const Vector2 delta = GetMouseDelta();

    if ((delta.x == 0.0f) && (delta.y == 0.0f))
    {
        return;
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
        {
            transform.rotY += delta.x * rotateSpeed_;
        }
        else
        {
            transform.rotZ += delta.x * rotateSpeed_;
            transform.rotX += delta.y * rotateSpeed_;
        }
    }
    else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        const Vector3 forward = normalize(subtract(camera.target, camera.position));
        const Vector3 right = normalize(cross(forward, camera.up));
        const Vector3 up = normalize(cross(right, forward));

        // Сдвиг в плоскости экрана: вправо - по right, вниз - против up.
        const float dx = delta.x * moveSpeed_;
        const float dy = delta.y * moveSpeed_;

        transform.position.x += right.x * dx - up.x * dy;
        transform.position.y += right.y * dx - up.y * dy;
        transform.position.z += right.z * dx - up.z * dy;
    }

    transform.rotX = wrapAngle(transform.rotX);
    transform.rotY = wrapAngle(transform.rotY);
    transform.rotZ = wrapAngle(transform.rotZ);
}

void motion::MouseManipulator::zoomCamera(Camera3D& camera) const
{
    const float wheel = GetMouseWheelMove();

    if (wheel == 0.0f)
    {
        return;
    }

    const Vector3 forward = normalize(subtract(camera.target, camera.position));

    float distance = 0.0f;
    distance += (camera.position.x - camera.target.x) * (camera.position.x - camera.target.x);
    distance += (camera.position.y - camera.target.y) * (camera.position.y - camera.target.y);
    distance += (camera.position.z - camera.target.z) * (camera.position.z - camera.target.z);
    distance = sqrtf(distance);

    // Колесо вверх (wheel > 0) - приближаемся, вниз - отдаляемся.
    distance -= wheel * zoomStep_;

    if (distance < minDistance_)
    {
        distance = minDistance_;
    }

    // Новая позиция: на луче взгляда на нужном расстоянии от цели.
    camera.position.x = camera.target.x - forward.x * distance;
    camera.position.y = camera.target.y - forward.y * distance;
    camera.position.z = camera.target.z - forward.z * distance;
}
