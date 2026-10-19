#pragma once
#include <memory>       
#include <random>
#include "curve/Curve.h"


class CurveFactory {
public:
    static std::shared_ptr<Curve> createRandom(std::mt19937& gen);
};