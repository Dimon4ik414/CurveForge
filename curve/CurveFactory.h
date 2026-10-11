#pragma once
#include <memory>       // ← обязательно, из-за std::shared_ptr / make_shared
#include <random>
#include "curve/Curve.h"

// Единственное место, где перечислены конкретные типы фигур.
class CurveFactory {
public:
    static std::shared_ptr<Curve> createRandom(std::mt19937& gen);
};