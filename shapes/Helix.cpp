#include "shapes/Helix.h"
#include <cmath>
#include <stdexcept>

namespace { constexpr double TWO_P = 2.0 *  3.14159265358979323846; }

Helix::Helix(double r, double step, double turns) : r_(r),step_(step) {
    if (r <= 0.0)
        throw std::invalid_argument("Helix radius must be positive");
    if (step == 0.0)
        throw std::invalid_argument("Helix step must be non-zero");
    if (turns <= 0.0)
        throw std::invalid_argument("Helix turns must be positive");
}

std::array<double, 3> Helix::point(double t) const {
    return { r_ * std::cos(t), r_ * std::sin(t),step_ * t / TWO_P };
};

std::array<double, 3> Helix::derivative(double t) const {
    return { -r_ * std::cos(t), r_ * std::sin(t),step_ / TWO_P };
};

