#pragma once
#include "curve/Curve.h"
#include "curve/CurveConfig.h"

class Helix : public Curve {
public:
    Helix(double r, double step, double turns = curve_cfg::DEFAULT_TURNS);

    std::array<double, 3> point(double t) const override;
    std::array<double, 3> derivative(double t) const override;

    double radius() const { return r_; }
    double step() const { return step_; }
    double turns() const { return turns_; }

    void setParams(double r, double step, double turns);

private:
    double r_;
    double step_;
    double turns_;
};