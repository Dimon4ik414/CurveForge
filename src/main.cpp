#include "raylib.h"
#include "raymath.h"
#include "curve/CurveConfig.h"
#include "motion/MouseManipulator.h"
#include "scene/SceneManager.h"
#include "shapes/Circle.h"
#include "shapes/Ellipse.h"
#include "shapes/Helix.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace
{
    constexpr float MIN_CAM_DIST = 3.0f;
    constexpr float MAX_CAM_DIST = 85.0f;
    constexpr float DEFAULT_CAM_DIST = 55.0f;
    constexpr float LINE_THICKNESS = 0.08f;

    enum class AxisLock { None, X, Y, Z };
}

static Vector3 toRay(const std::array<double, 3>& p)
{
    return Vector3{ static_cast<float>(p[0]), static_cast<float>(p[1]), static_cast<float>(p[2]) };
}

static void drawThickCurve(const Curve& c, Color color, double turns, float thickness, int steps = curve_cfg::CURVE_SEGMENTS)
{
    const double tEnd = turns * curve_cfg::TWO_PI;
    Vector3 prev = toRay(c.point(0.0));
    DrawSphere(prev, thickness * 0.55f, color);

    for (int i = 1; i <= steps; ++i)
    {
        Vector3 cur = toRay(c.point(tEnd * i / steps));
        DrawCylinderEx(prev, cur, thickness * 0.5f, thickness * 0.5f, 8, color);
        DrawSphere(cur, thickness * 0.5f, color);
        prev = cur;
    }
}

static void drawTangent(const Curve& c, Color color)
{
    const auto p = c.point(curve_cfg::MATH_PI / 4.0);
    const auto d = c.derivative(curve_cfg::MATH_PI / 4.0);
    Vector3 pos = toRay(p);
    Vector3 dir = toRay(d);

    const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    if (len > 1e-6f)
    {
        dir.x /= len;
        dir.y /= len;
        dir.z /= len;
    }

    Vector3 end{ pos.x + dir.x, pos.y + dir.y, pos.z + dir.z };
    DrawSphere(pos, 0.12f, color);
    DrawLine3D(pos, end, color);
}

static void drawGridWithAxes(int halfCells, float cellSize, AxisLock active, int step)
{
    const Color gridColor = Color{ 200, 200, 200, 255 };
    const Color wallColor = Color{ 185, 185, 185, 150 };

    const Color axisX = (active == AxisLock::X) ? RED : Color{ 150, 0, 0, 255 };
    const Color axisY = (active == AxisLock::Y) ? GREEN : Color{ 0, 150, 0, 255 };
    const Color axisZ = (active == AxisLock::Z) ? BLUE : Color{ 0, 0, 150, 255 };

    const float extent = halfCells * cellSize;
    const float corner = -extent;

    for (int i = -halfCells; i <= halfCells; i += step)
    {
        const float p = i * cellSize;
        DrawLine3D(Vector3{ -extent, p, corner }, Vector3{ extent, p, corner }, gridColor);
        DrawLine3D(Vector3{ p, -extent, corner }, Vector3{ p, extent, corner }, gridColor);

        DrawLine3D(Vector3{ -extent, corner, p }, Vector3{ extent, corner, p }, wallColor);
        DrawLine3D(Vector3{ p, corner, corner }, Vector3{ p, corner, extent }, wallColor);

        DrawLine3D(Vector3{ corner, -extent, p }, Vector3{ corner, extent, p }, wallColor);
        DrawLine3D(Vector3{ corner, p, corner }, Vector3{ corner, p, extent }, wallColor);
    }

    DrawLine3D(Vector3{ corner, corner, corner }, Vector3{ extent, corner, corner }, axisX);
    DrawLine3D(Vector3{ corner, corner, corner }, Vector3{ corner, extent, corner }, axisY);
    DrawLine3D(Vector3{ corner, corner, corner }, Vector3{ corner, corner, extent }, axisZ);

    DrawSphere(Vector3{ extent, corner, corner }, 0.14f, axisX);
    DrawSphere(Vector3{ corner, extent, corner }, 0.14f, axisY);
    DrawSphere(Vector3{ corner, corner, extent }, 0.14f, axisZ);
}

struct OrbitCamera
{
    Camera3D cam{};
    float yaw = 45.0f;
    float pitch = 30.0f;
    float dist = DEFAULT_CAM_DIST;
    Vector3 target{ 0.0f, 0.0f, 0.0f };

    void reset()
    {
        yaw = 45.0f;
        pitch = 30.0f;
        dist = DEFAULT_CAM_DIST;
        target = Vector3{ 0.0f, 0.0f, 0.0f };
        recompute();
    }

    void init()
    {
        cam.up = Vector3{ 0.0f, 0.0f, 1.0f };
        cam.fovy = 45.0f;
        cam.projection = CAMERA_PERSPECTIVE;
        reset();
    }

    void recompute()
    {
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
        if (dist < MIN_CAM_DIST) dist = MIN_CAM_DIST;
        if (dist > MAX_CAM_DIST) dist = MAX_CAM_DIST;

        const float yawR = yaw * DEG2RAD;
        const float pitchR = pitch * DEG2RAD;

        Vector3 offset;
        offset.x = dist * std::cos(pitchR) * std::sin(yawR);
        offset.y = dist * std::cos(pitchR) * std::cos(yawR);
        offset.z = dist * std::sin(pitchR);

        cam.position = Vector3Add(target, offset);
        cam.target = target;
    }

    void update()
    {
        const bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
        {
            const Vector2 d = GetMouseDelta();
            if (shift)
            {
                if (d.x != 0.0f || d.y != 0.0f)
                {
                    const Vector3 forward = Vector3Normalize(Vector3Subtract(target, cam.position));
                    const Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, cam.up));
                    const Vector3 up = Vector3Normalize(Vector3CrossProduct(right, forward));
                    const float s = 0.003f * dist;
                    target = Vector3Add(target, Vector3Scale(right, -d.x * s));
                    target = Vector3Add(target, Vector3Scale(up, d.y * s));
                }
            }
            else
            {
                yaw += d.x * 0.4f;
                pitch -= d.y * 0.4f;
            }
        }

        recompute();
    }
};

static AxisLock updateAxisLock()
{
    if (IsKeyDown(KEY_X)) return AxisLock::X;
    if (IsKeyDown(KEY_Y)) return AxisLock::Y;
    if (IsKeyDown(KEY_Z)) return AxisLock::Z;
    return AxisLock::None;
}

static void moveObjectByLockedAxis(motion::Transform& transform, AxisLock axis, float speed)
{
    if (axis == AxisLock::None || !IsMouseButtonDown(MOUSE_BUTTON_LEFT)) return;
    const Vector2 d = GetMouseDelta();
    if (d.x == 0.0f && d.y == 0.0f) return;

    switch (axis)
    {
    case AxisLock::X: transform.position.x += d.x * speed; break;
    case AxisLock::Y: transform.position.y += d.x * speed; break;
    case AxisLock::Z: transform.position.z += -d.y * speed; break;
    default: break;
    }
}

static void adjustPrimaryParam(SceneManager& scene, double delta)
{
    SceneItem* item = scene.selectedItem();
    if (!item || !item->curve) return;

    if (item->kind == CurveKind::Circle)
    {
        if (auto* c = dynamic_cast<Circle*>(item->curve.get()))
        {
            const double nextR = std::max(0.5, c->radius() + delta);
            scene.updateSelectedParams(nextR, 0.0, 0.0);
        }
    }
    else if (item->kind == CurveKind::Ellipse)
    {
        if (auto* e = dynamic_cast<Ellipse*>(item->curve.get()))
        {
            const double nextRx = std::max(0.5, e->radiusX() + delta);
            scene.updateSelectedParams(nextRx, e->radiusY(), 0.0);
        }
    }
    else if (item->kind == CurveKind::Helix)
    {
        if (auto* h = dynamic_cast<Helix*>(item->curve.get()))
        {
            const double nextR = std::max(0.5, h->radius() + delta);
            scene.updateSelectedParams(nextR, h->step(), h->turns());
        }
    }
}

static std::string describeSelected(const SceneItem* item)
{
    if (!item || !item->curve) return "None";

    if (item->kind == CurveKind::Circle)
    {
        if (const auto* c = dynamic_cast<const Circle*>(item->curve.get()))
        {
            return "Circle (r=" + std::to_string(c->radius()).substr(0, 4) + ")";
        }
    }
    else if (item->kind == CurveKind::Ellipse)
    {
        if (const auto* e = dynamic_cast<const Ellipse*>(item->curve.get()))
        {
            return "Ellipse (rx=" + std::to_string(e->radiusX()).substr(0, 4) +
                ", ry=" + std::to_string(e->radiusY()).substr(0, 4) + ")";
        }
    }
    else if (item->kind == CurveKind::Helix)
    {
        if (const auto* h = dynamic_cast<const Helix*>(item->curve.get()))
        {
            return "Helix (r=" + std::to_string(h->radius()).substr(0, 4) +
                ", step=" + std::to_string(h->step()).substr(0, 4) +
                ", turns=" + std::to_string(h->turns()).substr(0, 3) + ")";
        }
    }
    return "Curve";
}

int main()
{
    constexpr int GRID_HALF = 40;
    constexpr float GRID_CELL = 0.5f;
    constexpr int GRID_STEP = 5;
    constexpr float EXTENT = GRID_HALF * GRID_CELL;

    SceneManager scene;
    scene.addCircle(4.0);
    scene.printAllAt(curve_cfg::MATH_PI / 4.0);

    motion::MouseManipulator manipulator;

    InitWindow(1100, 720, "CADEX - 3D curves");
    SetTargetFPS(60);

    OrbitCamera orbit;
    orbit.init();

    while (!WindowShouldClose())
    {
        bool sceneChanged = false;

        if (IsKeyPressed(KEY_ONE)) { scene.addCircle(4.0); sceneChanged = true; }
        if (IsKeyPressed(KEY_TWO)) { scene.addEllipse(5.0, 3.0); sceneChanged = true; }
        if (IsKeyPressed(KEY_THREE)) { scene.addHelix(4.0, 3.0, curve_cfg::DEFAULT_TURNS); sceneChanged = true; }

        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_DELETE)) { sceneChanged = scene.removeSelected(); }
        if (IsKeyPressed(KEY_TAB)) scene.selectNext();

        if (IsKeyPressed(KEY_LEFT_BRACKET)) { adjustPrimaryParam(scene, -0.5); sceneChanged = true; }
        if (IsKeyPressed(KEY_RIGHT_BRACKET)) { adjustPrimaryParam(scene, 0.5); sceneChanged = true; }

        if (IsKeyPressed(KEY_P)) sceneChanged = true;

        if (sceneChanged)
        {
            scene.printAllAt(curve_cfg::MATH_PI / 4.0);
        }

        if (IsKeyPressed(KEY_R))
        {
            orbit.reset();
            if (SceneItem* sel = scene.selectedItem())
            {
                sel->transform.resetPose();
            }
        }

        const AxisLock axis = updateAxisLock();

        if (SceneItem* sel = scene.selectedItem())
        {
            manipulator.update(sel->transform, orbit.cam, axis == AxisLock::None);
            moveObjectByLockedAxis(sel->transform, axis, 0.03f);
        }

        manipulator.zoomCamera(orbit.cam, MIN_CAM_DIST, MAX_CAM_DIST);
        orbit.dist = Vector3Distance(orbit.cam.position, orbit.cam.target);
        orbit.update();

        scene.clampAllToGrid(EXTENT);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(orbit.cam);

        drawGridWithAxes(GRID_HALF, GRID_CELL, axis, GRID_STEP);

        const int activeIdx = scene.selectedIndex();
        for (int i = 0; i < static_cast<int>(scene.items().size()); ++i)
        {
            const auto& item = scene.items()[i];
            if (!item.curve) continue;

            double turns = curve_cfg::DEFAULT_TURNS;
            if (const auto* h = dynamic_cast<const Helix*>(item.curve.get()))
            {
                turns = h->turns();
            }

            const bool isSelected = (i == activeIdx);
            const Color lineCol = isSelected ? DARKBLUE : GRAY;
            const Color tangCol = isSelected ? MAROON : DARKGRAY;

            item.transform.push();
            drawThickCurve(*item.curve, lineCol, turns, LINE_THICKNESS);
            drawTangent(*item.curve, tangCol);
            item.transform.pop();
        }

        EndMode3D();

        DrawRectangle(10, 10, 440, 118, Fade(LIGHTGRAY, 0.65f));
        DrawText(TextFormat("Curves: %d | Active: %d", static_cast<int>(scene.items().size()), activeIdx + 1), 18, 16, 20, DARKGREEN);
        DrawText(TextFormat("Selected: %s", describeSelected(scene.selectedItem()).c_str()), 18, 42, 18, DARKGRAY);
        DrawText("[1]Circle  [2]Ellipse  [3]Helix  [Del]Remove  [Tab]Next", 18, 70, 16, DARKGRAY);
        DrawText("[ / ] Param +/-   [P] Print t=PI/4   [R] Reset   [X/Y/Z+LMB] Axis", 18, 94, 16, DARKGRAY);

        DrawText(TextFormat("Cam dist: %.1f / %.1f", orbit.dist, MAX_CAM_DIST), GetScreenWidth() - 210, 16, 18, GRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}