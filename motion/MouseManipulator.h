#pragma once

#include "motion/ObjectTransform.h"

namespace motion
{
    // Управление фигурой мышью:
    //   ЛКМ          - вращение вокруг осей Z и X,
    //   Shift + ЛКМ  - вращение вокруг оси Y (крен),
    //   ПКМ          - перемещение в плоскости экрана.
    class MouseManipulator
    {
    public:
        void update(Transform& transform, const Camera3D& camera) const;

    private:
        float rotateSpeed_ = 0.4f;   // градусов на пиксель
        float moveSpeed_ = 0.03f;    // единиц мира на пиксель
    };
}
