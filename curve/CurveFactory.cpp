#include "curve/CurveFactory.h"
#include <memory>             
#include "shapes/Ellipse.h"
#include "shapes/Circle.h"
#include "shapes/Helix.h"

std::shared_ptr<Curve> CurveFactory::createRandom(std::mt19937& gen) {
    std::uniform_int_distribution<int> typeDist(0, 2);
    std::uniform_real_distribution<double> paramDist(0.5, 5.0);

    switch (typeDist(gen)) {
    case 0: return std::make_shared<Circle>(paramDist(gen));
    case 1: return std::make_shared<Ellipse>(paramDist(gen), paramDist(gen));
    case 2: return std::make_shared<Helix>(paramDist(gen), paramDist(gen), 3.0);
    }
}