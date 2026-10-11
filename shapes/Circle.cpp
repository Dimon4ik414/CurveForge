#include "shapes/Circle.h"
#include <cmath>
#include <stdexcept>

Circle::Circle(double r) : r_(r) {
    if (r <= 0.0)
        throw std::invalid_argument("Circle radius must be positive");
}

std::array<double, 3> Circle::point(double t) const {
    return { r_ * std::cos(t), r_ * std::sin(t), 0.0 };
}

std::array<double, 3> Circle::derivative(double t) const {
    return { -r_ * std::sin(t), r_ * std::cos(t), 0.0 };
}