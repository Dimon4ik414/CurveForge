#pragma once
#include "curve/Curve.h"
#include "curve/IRound.h"

class Circle : public Curve, public IRound {
public:
    explicit Circle(double r);

    std::array<double, 3> point(double t) const override;
    std::array<double, 3> derivative(double t) const override;
    double radius() const override { return r_; }

private:
    double r_;
};