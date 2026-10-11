#pragma once
#include <array>

// Базовый интерфейс любой 3D-кривой.
class Curve {
public:
    virtual ~Curve() = default;

    virtual std::array<double, 3> point(double t) const = 0;
    virtual std::array<double, 3> derivative(double t) const = 0;

    // удобная печать точки и производной при заданном t
    void printAt(double t) const;
};