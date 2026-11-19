#include "shapes/Helix.h"
#include <cmath>
#include <stdexcept>

Helix::Helix(double r, double step, double turns) {
    setParams(r, step, turns);
}

void Helix::setParams(double r, double step, double turns) {
    if (r <= 0.0)
        throw std::invalid_argument("Helix radius must be positive");
    if (step <= 0.0)
        throw std::invalid_argument("Helix step must be positive");
    if (turns <= 0.0)
        throw std::invalid_argument("Helix turns must be positive");

    r_ = r;
    step_ = step;
    turns_ = turns;
}

std::array<double, 3> Helix::point(double t) const {
    return { r_ * std::cos(t), r_ * std::sin(t), step_ * t / curve_cfg::TWO_PI };
}

std::array<double, 3> Helix::derivative(double t) const {
    return { -r_ * std::sin(t), r_ * std::cos(t), step_ / curve_cfg::TWO_PI };
}