#pragma once

#include "motion/ObjectTransform.h"

namespace motion
{
    // Управление фигурой мышью:
    //   ЛКМ          - вращение вокруг осей Z и X,
    //   Shift + ЛКМ  - вращение вокруг оси Y (крен),
    //   ПКМ          - перемещение в плоскости экрана,
    //   колесо       - приближение/отдаление камеры.
    class MouseManipulator
    {
    public:
        void update(Transform& transform, const Camera3D& camera) const;

        // Приближение/отдаление: двигает камеру к цели по её взгляду.
        void zoomCamera(Camera3D& camera) const;

    private:
        float rotateSpeed_ = 0.4f;   // градусов на пиксель
        float moveSpeed_ = 0.03f;    // единиц мира на пиксель
        float zoomStep_ = 1.0f;      // единиц мира на щелчок колеса
        float minDistance_ = 1.0f;   // ближе к цели камеру не пускаем
    };
}
