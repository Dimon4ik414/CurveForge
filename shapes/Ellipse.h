#pragma once
#include "curve/Curve.h"
#include "curve/IRound.h"


class Ellipse : public Curve,public IRound {
public:
   explicit Ellipse(double rx, double ry);

    std::array<double, 3> point(double t) const override;
    std::array<double, 3> derivative(double t) const override;
    double radius() const override { return rx_,ry_ ; }

private:
    double rx_;
    double ry_;
};