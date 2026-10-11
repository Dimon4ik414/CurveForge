#include "curve/CurveFactory.h"
#include <memory>               // ← на всякий случай, чтобы make_shared был точно виден
#include "shapes/Ellipse.h"
#include "shapes/Circle.h"
#include "shapes/Helix.h"

std::shared_ptr<Curve> CurveFactory::createRandom(std::mt19937& gen) {
    std::uniform_int_distribution<int> typeDist(0, 2);
    std::uniform_real_distribution<double> paramDist(0.5, 5.0);

    switch (typeDist(gen)) {
    case 1: return std::make_shared<Circle>(paramDist(gen));
    default: return std::make_shared<Circle>(paramDist(gen));
    }
}