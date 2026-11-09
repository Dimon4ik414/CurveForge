#include "raylib.h"
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

    // Сколько витков рисуем у кривой (по ним же считаем центр фигуры).
    constexpr double CURVE_TURNS = 3.0;
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

    Vector3 end;
    end.x = pos.x + dir.x;
    end.y = pos.y + dir.y;
    end.z = pos.z + dir.z;

    DrawSphere(pos, 0.06f, color);
    DrawLine3D(pos, end, color);
}



int main() {
   
    std::mt19937 gen(std::random_device{}());
    std::vector<std::shared_ptr<Curve>> curves;
    for (int i = 0; i <1; ++i)

        curves.push_back(CurveFactory::createRandom(gen));
        //curves.push_back(std::make_shared<Ellipse>(3.0, 1.5));
        //curves.push_back(std::make_shared<Helix>(3.0,1.5));
        //curves.push_back(std::make_shared<Circle>(3.0,1.5));

    // Трансформация фигуры: двигаем и вращаем саму фигуру,
    // камера при этом остаётся статичной.
    motion::Transform transform;
    transform.pivot = motion::curveCenter(*curves.front(), CURVE_TURNS);

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

    while (!WindowShouldClose()) {
        // Автоповорот камеры отключён: камера статична,
        // а мышью управляем самой фигурой.
        // UpdateCamera(&camera, CAMERA_ORBITAL);
        manipulator.update(transform, camera);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        transform.push();
        for (const auto& c : curves) {
   
                drawCurve(*c, DARKBLUE, CURVE_TURNS);
        }

        for (const auto& c : curves)
            drawTangent(*c, MAROON);
        transform.pop();

        EndMode3D();
        
        DrawText(TextFormat("Total radius: %.3f", sum), 10, 10, 20, DARKGREEN);
        DrawText(TextFormat("Curves: %d",
            static_cast<int>(curves.size()),
            static_cast<int>(rounds.size())),
            10, 35, 20, DARKGRAY);

        // Подсказка по управлению — временная, до появления меню.
        DrawText("LMB: rotate   Shift+LMB: roll   RMB: move",
            10, GetScreenHeight() - 28, 18, GRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}