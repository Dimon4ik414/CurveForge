#include "shapes/Ellipse.h"
#include <cmath>
#include <stdexcept>

Ellipse::Ellipse(double rx, double ry) : rx_(rx), ry_(ry) {
    if (rx <= 0.0 || ry <= 0.0)
        throw std::invalid_argument("Ellipse radii must be positive");
}

std::array<double, 3> Ellipse::point(double t) const {
    return { rx_ * std::cos(t), ry_ * std::sin(t), 0.0 };
}

std::array<double, 3> Ellipse::derivative(double t) const {
    return { -rx_ * std::sin(t), ry_ * std::cos(t), 0.0 };
}