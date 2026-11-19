#pragma once

#include "motion/ObjectTransform.h"

namespace motion
{
    class MouseManipulator
    {
    public:
        void update(Transform& transform, const Camera3D& camera, bool allowPlaneMove) const;
        void zoomCamera(Camera3D& camera, float minDist, float maxDist) const;

    private:
        float rotateSpeed_ = 0.4f;
        float moveSpeed_ = 0.03f;
        float zoomStep_ = 2.0f;
    };
}