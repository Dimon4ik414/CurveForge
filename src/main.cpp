#include "raylib.h"
#include "raymath.h"
#include "curve/Curve.h"
#include "curve/IRound.h"
#include "curve/CurveFactory.h"
#include "motion/ObjectTransform.h"
#include "motion/MouseManipulator.h"
#include "shapes/Ellipse.h"
#include "shapes/Circle.h"
#include "shapes/Helix.h"

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
    return Vector3{ (float)p[0], (float)p[1], (float)p[2] };
}

static void drawCurve(const Curve& c, Color color, double turns = 1.0, int steps = 256) {
    double t_end = turns * 2.0 * P;
    Vector3 prev = toRay(c.point(0.0));
    for (int i = 1; i <= steps; ++i) {
        Vector3 cur = toRay(c.point(t_end * i / steps));
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

// ---------- Сетка "уголок" + оси ----------
static void drawGridWithAxes(int halfCells, float cellSize, AxisLock active, int step) {
    const Color gridColor = Color{ 200, 200, 200, 255 };
    const Color wallColor = Color{ 185, 185, 185, 150 };

    const Color axisX = (active == AxisLock::X) ? RED : Color{ 150,   0,   0, 255 };
    const Color axisY = (active == AxisLock::Y) ? GREEN : Color{ 0, 150,   0, 255 };
    const Color axisZ = (active == AxisLock::Z) ? BLUE : Color{ 0,   0, 150, 255 };

    const float extent = halfCells * cellSize;
    const float corner = -extent;

    // Пол (Z = -extent)
    for (int i = -halfCells; i <= halfCells; i += step) {
        float p = i * cellSize;
        DrawLine3D(Vector3{ -extent, p, corner }, Vector3{ extent, p, corner }, gridColor);
        DrawLine3D(Vector3{ p, -extent, corner }, Vector3{ p,  extent, corner }, gridColor);
    }

    // Задняя стена (Y = -extent)
    for (int i = -halfCells; i <= halfCells; i += step) {
        float p = i * cellSize;
        DrawLine3D(Vector3{ -extent, corner, p }, Vector3{ extent, corner, p }, wallColor);
        DrawLine3D(Vector3{ p, corner, corner }, Vector3{ p, corner, extent }, wallColor);
    }

    // Левая стена (X = -extent)
    for (int i = -halfCells; i <= halfCells; i += step) {
        float p = i * cellSize;
        DrawLine3D(Vector3{ corner, -extent, p }, Vector3{ corner,  extent, p }, wallColor);
        DrawLine3D(Vector3{ corner, p, corner }, Vector3{ corner, p, extent }, wallColor);
    }

    // Оси
    DrawLine3D(Vector3{ corner, corner, corner }, Vector3{ extent, corner, corner }, axisX);
    DrawLine3D(Vector3{ corner, corner, corner }, Vector3{ corner,  extent, corner }, axisY);
    DrawLine3D(Vector3{ corner, corner, corner }, Vector3{ corner, corner,  extent }, axisZ);

    DrawSphere(Vector3{ extent, corner, corner }, 0.1f, axisX);
    DrawSphere(Vector3{ corner,  extent, corner }, 0.1f, axisY);
    DrawSphere(Vector3{ corner, corner,  extent }, 0.1f, axisZ);
}

// ---------- Камера ----------
struct OrbitCamera {
    Camera3D cam{};
    float yaw = 45.0f;
    float pitch = 30.0f;
    float dist = 60.0f;
    Vector3 target{ 0.0f, 0.0f, 0.0f };

    void init() {
        cam.up = Vector3{ 0.0f, 0.0f, 1.0f };
        cam.fovy = 45.0f;
        cam.projection = CAMERA_PERSPECTIVE;
        recompute();
    }

    void recompute() {
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        float yawR = yaw * DEG2RAD;
        float pitchR = pitch * DEG2RAD;

        Vector3 offset;
        offset.x = dist * std::cos(pitchR) * std::sin(yawR);
        offset.y = dist * std::cos(pitchR) * std::cos(yawR);
        offset.z = dist * std::sin(pitchR);

        cam.position = Vector3Add(target, offset);
        cam.target = target;
    }

    void update() {
        const bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && shift) {
            Vector2 d = GetMouseDelta();
            yaw += d.x * 0.4f;
            pitch -= d.y * 0.4f;
        }
        else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 d = GetMouseDelta();
            if (d.x != 0 || d.y != 0) {
                Vector3 forward = Vector3Normalize(Vector3Subtract(target, cam.position));
                Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, cam.up));
                Vector3 up = Vector3Normalize(Vector3CrossProduct(right, forward));
                float s = 0.003f * dist;
                target = Vector3Add(target, Vector3Scale(right, -d.x * s));
                target = Vector3Add(target, Vector3Scale(up, d.y * s));
            }
        }

        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            dist -= wheel * 5.0f;
            if (dist < 3.0f) dist = 3.0f;
        }

        recompute();
    }
};

static AxisLock updateAxisLock() {
    if (IsKeyDown(KEY_X)) return AxisLock::X;
    if (IsKeyDown(KEY_Y)) return AxisLock::Y;
    if (IsKeyDown(KEY_Z)) return AxisLock::Z;
    return AxisLock::None;
}

static void moveObjectByLockedAxis(motion::Transform& transform, AxisLock axis, float speed) {
    if (axis == AxisLock::None || !IsMouseButtonDown(MOUSE_BUTTON_LEFT)) return;
    Vector2 d = GetMouseDelta();
    if (d.x == 0.0f && d.y == 0.0f) return;

    switch (axis) {
    case AxisLock::X: transform.position.x += d.x * speed; break;
    case AxisLock::Y: transform.position.y += d.x * speed; break;
    case AxisLock::Z: transform.position.z += -d.y * speed; break;
    default: break;
    }
}

// ---------- ИСПРАВЛЕННАЯ БЛОКИРОВКА ----------
static void clampToGrid(motion::Transform& transform, float extent, const Curve& curve) {
    float minX = 1e9f, maxX = -1e9f;
    float minY = 1e9f, maxY = -1e9f;
    float minZ = 1e9f, maxZ = -1e9f;

    // Считаем локальные габариты фигуры
    for (int i = 0; i <= 128; ++i) {
        auto p = curve.point(2.0 * P * i / 128.0);
        minX = std::min(minX, (float)p[0]); maxX = std::max(maxX, (float)p[0]);
        minY = std::min(minY, (float)p[1]); maxY = std::max(maxY, (float)p[1]);
        minZ = std::min(minZ, (float)p[2]); maxZ = std::max(maxZ, (float)p[2]);
    }

    // ВАЖНО: ограничиваем не центр, а реальные края фигуры
    // Например, если minZ = 0 и maxZ = 0 (плоская фигура), то floor и ceiling для позиции Z будут -extent и +extent
    float minPosX = -extent - minX;
    float maxPosX = extent - maxX;
    float minPosY = -extent - minY;
    float maxPosY = extent - maxY;
    float minPosZ = -extent - minZ;
    float maxPosZ = extent - maxZ;

    // Применяем ограничения к позиции
    if (transform.position.x < minPosX) transform.position.x = minPosX;
    if (transform.position.x > maxPosX) transform.position.x = maxPosX;
    if (transform.position.y < minPosY) transform.position.y = minPosY;
    if (transform.position.y > maxPosY) transform.position.y = maxPosY;

    // Для Z: пол в самом низу (-extent), потолок на +extent
    if (transform.position.z < minPosZ) transform.position.z = minPosZ;
    if (transform.position.z > maxPosZ) transform.position.z = maxPosZ;
}

int main() {
    const int   GRID_HALF = 40;
    const float GRID_CELL = 0.5f;
    const int   GRID_STEP = 5;
    const float EXTENT = GRID_HALF * GRID_CELL;   // 20.0

    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<double> rdist(EXTENT * 0.1, EXTENT * 0.3);

    std::vector<std::shared_ptr<Curve>> curves;
    std::uniform_int_distribution<int> typeDist(0, 2);
    switch (typeDist(gen)) {
    case 0: curves.push_back(std::make_shared<Circle>(rdist(gen))); break;
    case 1: curves.push_back(std::make_shared<Ellipse>(rdist(gen), rdist(gen))); break;
    default: curves.push_back(std::make_shared<Helix>(rdist(gen), rdist(gen) * 2.0)); break;
    }

    motion::Transform transform;
    transform.pivot = motion::curveCenter(*curves.front(), CURVE_TURNS);
    transform.position = Vector3{ 0.0f, 0.0f, 0.0f }; // Старт в центре, clamp поставит на пол

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

    OrbitCamera orbit;
    orbit.target = Vector3{ 0.0f, 0.0f, 0.0f };
    orbit.init();

    AxisLock axis = AxisLock::None;

    while (!WindowShouldClose()) {
        axis = updateAxisLock();

        if (axis == AxisLock::None && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            manipulator.update(transform, orbit.cam);
        }

        orbit.update();
        moveObjectByLockedAxis(transform, axis, 0.03f);

        // Ограничиваем строго по краям фигуры (а не по её центру)
        clampToGrid(transform, EXTENT, *curves.front());

        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(orbit.cam);

        drawGridWithAxes(GRID_HALF, GRID_CELL, axis, GRID_STEP);

        transform.push();
        for (const auto& c : curves) drawCurve(*c, DARKBLUE, CURVE_TURNS);
        for (const auto& c : curves) drawTangent(*c, MAROON);
        transform.pop();

        EndMode3D();

        DrawText(TextFormat("Total radius: %.3f", sum), 10, 10, 20, DARKGREEN);
        DrawText(TextFormat("Curves: %d", (int)curves.size(), (int)rounds.size()), 10, 35, 20, DARKGRAY);
        DrawText("LMB: rotate   X/Y/Z + LMB: move   RMB: pan   Shift+RMB: orbit   Wheel: zoom", 10, GetScreenHeight() - 28, 16, GRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}