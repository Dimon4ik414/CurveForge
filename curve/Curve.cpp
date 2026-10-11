#include "curve/Curve.h"
#include <iostream>

void Curve::printAt(double t) const {
    auto p = point(t);
    auto d = derivative(t);
    std::cout << "P(" << p[0] << ", " << p[1] << ", " << p[2] << ")  "
        << "D(" << d[0] << ", " << d[1] << ", " << d[2] << ")\n";
}