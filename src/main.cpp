#include "raylib.h"
#include "raymath.h"
#include "curve/Curve.h"
#include "curve/IRound.h"
#include "curve/CurveFactory.h"
#include "motion/ObjectTransform.h"
#include "motion/MouseManipulator.h"
//#include "shapes/Ellipse.h"
//#include "shapes/Circle.h"
//#include "shapes/Helix.h"

#include <array>
#include <vector>
#include <memory>
#include <random>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <cmath>


namespace
{
    constexpr double P = 3.14159265358979323846;
    constexpr double CURVE_TURNS = 3.0;
    enum class AxisLock { None, X, Y, Z };
}


static Vector3 toRay(const std::array<double, 3>& p) {
    Vector3 v;
    v.x = static_cast<float>(p[0]);
    v.y = static_cast<float>(p[1]);
    v.z = static_cast<float>(p[2]);
    return v;
}

static void drawCurve(const Curve& c, Color color, double turns = 1.0, int steps = 256) {
    double t_end = turns * 2.0 * P;
    Vector3 prev = toRay(c.point(0.0));
    for (int i = 1; i <= steps; ++i) {
        double t = t_end * i / steps;
        Vector3 cur = toRay(c.point(t));
        DrawLine3D(prev, cur, color);
        prev = cur;
    }
}

static void drawTangent(const Curve& c, Color color) {
    auto p = c.point(P / 4.0);
    auto d = c.derivative(P / 4.0);
    Vector3 pos = toRay(p);
    Vector3 dir = toRay(d);
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    if (len > 1e-6f) { dir.x /= len; dir.y /= len; dir.z /= len; }
    Vector3 end{ pos.x + dir.x, pos.y + dir.y, pos.z + dir.z };
    DrawSphere(pos, 0.06f, color);
    DrawLine3D(pos, end, color);
}

// ---------- сетка: пол снизу + стены XZ/YZ + оси в углу, Z по ребру ----------
static void drawGridWithAxes(int halfCells, float cellSize, AxisLock active, int step = 1) {
    const Color gridColor = Color{ 200, 200, 200, 255 };
    const Color wallXZ = Color{ 190, 190, 190, 180 };
    const Color wallYZ = Color{ 185, 185, 185, 160 };

    const Color axisX = (active == AxisLock::X) ? RED : Color{ 150,   0,   0, 255 };
    const Color axisY = (active == AxisLock::Y) ? GREEN : Color{ 0, 150,   0, 255 };
    const Color axisZ = (active == AxisLock::Z) ? BLUE : Color{ 0,   0, 150, 255 };

    const float extent = halfCells * cellSize;
    const float corner = -extent;     // угол, где сходятся стены
    const float floorZ = -extent;     // пол снизу

    // ---------- пол XY (снизу) ----------
    for (int i = -halfCells; i <= halfCells; i += step) {
        float y = i * cellSize;
        if (i == 0) continue;
        DrawLine3D(Vector3{ -extent, y, floorZ }, Vector3{ extent, y, floorZ }, gridColor);
    }
    for (int i = -halfCells; i <= halfCells; i += step) {
        float x = i * cellSize;
        if (i == 0) continue;
        DrawLine3D(Vector3{ x, -extent, floorZ }, Vector3{ x,  extent, floorZ }, gridColor);
    }

    // ---------- стена XZ (y = corner) ----------
    for (int i = -halfCells; i <= halfCells; i += step) {
        float z = i * cellSize;
        if (i == 0) continue;
        DrawLine3D(Vector3{ -extent, corner, z }, Vector3{ extent, corner, z }, wallXZ);
    }
    for (int i = -halfCells; i <= halfCells; i += step) {
        float x = i * cellSize;
        if (i == 0) continue;
        DrawLine3D(Vector3{ x, corner, -extent }, Vector3{ x, corner,  extent }, wallXZ);
    }

    // ---------- стена YZ (x = corner) ----------
    for (int i = -halfCells; i <= halfCells; i += step) {
        float z = i * cellSize;
        if (i == 0) continue;
        DrawLine3D(Vector3{ corner, -extent, z }, Vector3{ corner,  extent, z }, wallYZ);
    }
    for (int i = -halfCells; i <= halfCells; i += step) {
        float y = i * cellSize;
        if (i == 0) continue;
        DrawLine3D(Vector3{ corner, y, -extent }, Vector3{ corner, y,  extent }, wallYZ);
    }

    // ---------- оси ----------
    // X — по низу задней стены
    DrawLine3D(Vector3{ corner, corner, floorZ },
        Vector3{ extent, corner, floorZ }, axisX);
    // Y — по низу левой стены
    DrawLine3D(Vector3{ corner, corner, floorZ },
        Vector3{ corner,  extent, floorZ }, axisY);
    // Z — по ребру, от пола вверх
    DrawLine3D(Vector3{ corner, corner, floorZ },
        Vector3{ corner, corner,  extent }, axisZ);

    // маркеры на концах осей
    DrawSphere(Vector3{ extent, corner, floorZ }, 0.05f, axisX);
    DrawSphere(Vector3{ corner,  extent, floorZ }, 0.05f, axisY);
    DrawSphere(Vector3{ corner, corner,  extent }, 0.05f, axisZ);
}

// ---------- камера: зажатое колесо ----------
static void moveCameraByMiddleMouse(Camera3D& camera) {
    if (!IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) return;
    Vector2 d = GetMouseDelta();
    if (d.x == 0.0f && d.y == 0.0f) return;

    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
    Vector3 up = Vector3Normalize(Vector3CrossProduct(right, forward));

    float dist = Vector3Distance(camera.position, camera.target);
    float s = 0.003f * dist;

    Vector3 shift = Vector3Add(Vector3Scale(right, -d.x * s), Vector3Scale(up, d.y * s));
    camera.position = Vector3Add(camera.position, shift);
    camera.target = Vector3Add(camera.target, shift);
}

static AxisLock updateAxisLock() {
    if (IsKeyDown(KEY_X)) return AxisLock::X;
    if (IsKeyDown(KEY_Y)) return AxisLock::Y;
    if (IsKeyDown(KEY_Z)) return AxisLock::Z;
    return AxisLock::None;
}

static void moveObjectByLockedAxis(motion::Transform& transform, AxisLock axis,
    float speed, float limit) {
    if (axis == AxisLock::None) return;
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) return;

    Vector2 d = GetMouseDelta();
    if (d.x == 0.0f && d.y == 0.0f) return;

    switch (axis) {
    case AxisLock::X:
        transform.position.x += d.x * speed;
        if (transform.position.x > limit) transform.position.x = limit;
        if (transform.position.x < -limit) transform.position.x = -limit;
        break;
    case AxisLock::Y:
        transform.position.y += d.x * speed;
        if (transform.position.y > limit) transform.position.y = limit;
        if (transform.position.y < -limit) transform.position.y = -limit;
        break;
    case AxisLock::Z:
        transform.position.z += -d.y * speed;
        // запрещаем опускать фигуру вниз (пол = 0)
        if (transform.position.z < 0.0f) transform.position.z = 0.0f;
        if (transform.position.z > limit) transform.position.z = limit;
        break;
    default: break;
    }
}


int main() {
    std::mt19937 gen(std::random_device{}());
    std::vector<std::shared_ptr<Curve>> curves;
    for (int i = 0; i < 1; ++i)
        curves.push_back(CurveFactory::createRandom(gen));
    //curves.push_back(std::make_shared<Ellipse>(3.0, 1.5));
    //curves.push_back(std::make_shared<Helix>(3.0,1.5));
    //curves.push_back(std::make_shared<Circle>(3.0,1.5));

    motion::Transform transform;
    transform.pivot = motion::curveCenter(*curves.front(), CURVE_TURNS);
    // стартовая позиция: фигура на полу
    transform.position.z = 0.0f;

    motion::MouseManipulator manipulator;

    std::cout << "Curves at t = PI/4\n";
    for (const auto& c : curves) c->printAt(P / 4.0);

    std::vector<IRound*> rounds;
    for (auto& c : curves)
        if (auto* r = dynamic_cast<IRound*>(c.get()))
            rounds.push_back(r);

    std::sort(rounds.begin(), rounds.end(),
        [](IRound* a, IRound* b) { return a->radius() < b->radius(); });

    double sum = std::accumulate(rounds.begin(), rounds.end(), 0.0,
        [](double acc, IRound* r) { return acc + r->radius(); });
    std::cout << "Total radius of round curves: " << sum << "\n";

    InitWindow(1000, 700, "CADEX - 3D curves");
    SetTargetFPS(60);

    Camera3D camera = { 0 };
    camera.position = Vector3{ 12.0f, -12.0f, 10.0f };
    camera.target = Vector3{ 0.0f, 0.0f, 2.0f };
    camera.up = Vector3{ 0.0f, 0.0f, 1.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    AxisLock axis = AxisLock::None;

    const int   GRID_HALF = 40;
    const float GRID_CELL = 0.5f;
    const int   GRID_STEP = 5;
    const float MOVE_LIMIT = GRID_HALF * GRID_CELL;   // = 20.0f

    while (!WindowShouldClose()) {
        axis = updateAxisLock();

        if (axis == AxisLock::None) {
            manipulator.update(transform, camera);
        }
        manipulator.zoomCamera(camera);

        moveCameraByMiddleMouse(camera);
        moveObjectByLockedAxis(transform, axis, 0.03f, MOVE_LIMIT);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);

        drawGridWithAxes(GRID_HALF, GRID_CELL, axis, GRID_STEP);

        transform.push();
        for (const auto& c : curves)
            drawCurve(*c, DARKBLUE, CURVE_TURNS);
        for (const auto& c : curves)
            drawTangent(*c, MAROON);
        transform.pop();

        EndMode3D();

        DrawText(TextFormat("Total radius: %.3f", sum), 10, 10, 20, DARKGREEN);
        DrawText(TextFormat("Curves: %d",
            static_cast<int>(curves.size()),
            static_cast<int>(rounds.size())), 10, 35, 20, DARKGRAY);

        DrawText("X/Y/Z + LMB: move by axis   RMB: rotate   Shift+RMB: roll   Wheel: zoom",
            10, GetScreenHeight() - 50, 16, GRAY);
        DrawText("Middle mouse: move camera",
            10, GetScreenHeight() - 28, 16, GRAY);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}