#pragma once
#include "curve/Curve.h"
#include "curve/IRound.h"


class Helix : public Curve, public IRound {
public:
    Helix(double r, double step,double turns = 1.0);

    std::array<double, 3> point(double t) const override;
    std::array<double, 3> derivative(double t) const override;
    double radius() const override { return r_; }
    double turns() const { return turns_; }

private:
    double r_;
    double step_;
    double turns_;
};