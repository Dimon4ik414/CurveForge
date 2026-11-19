#pragma once
#include "curve/Curve.h"

class Ellipse : public Curve {
public:
    explicit Ellipse(double rx, double ry);

    std::array<double, 3> point(double t) const override;
    std::array<double, 3> derivative(double t) const override;

    double radiusX() const { return rx_; }
    double radiusY() const { return ry_; }
    void setRadii(double rx, double ry);

private:
    double rx_;
    double ry_;
};