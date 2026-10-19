#pragma once
#include <array>

class Curve {
public:
    virtual ~Curve() = default;

    virtual std::array<double, 3> point(double t) const = 0;
    virtual std::array<double, 3> derivative(double t) const = 0;

    void printAt(double t) const;
};