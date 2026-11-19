#include "scene/SceneManager.h"
#include "shapes/Circle.h"
#include "shapes/Ellipse.h"
#include "shapes/Helix.h"
#include <algorithm>
#include <iostream>
#include <numeric>

void SceneManager::attachItem(SceneItem item)
{
    recalculatePivot(item);
    items_.push_back(std::move(item));
    selectedIndex_ = static_cast<int>(items_.size()) - 1;
}

void SceneManager::addCircle(double radius)
{
    SceneItem item;
    item.kind = CurveKind::Circle;
    item.curve = std::make_shared<Circle>(radius);
    attachItem(std::move(item));
}

void SceneManager::addEllipse(double rx, double ry)
{
    SceneItem item;
    item.kind = CurveKind::Ellipse;
    item.curve = std::make_shared<Ellipse>(rx, ry);
    attachItem(std::move(item));
}

void SceneManager::addHelix(double r, double step, double turns)
{
    SceneItem item;
    item.kind = CurveKind::Helix;
    item.curve = std::make_shared<Helix>(r, step, turns);
    attachItem(std::move(item));
}

bool SceneManager::removeSelected()
{
    if (!hasSelection()) return false;

    items_.erase(items_.begin() + selectedIndex_);
    if (items_.empty())
    {
        selectedIndex_ = -1;
    }
    else if (selectedIndex_ >= static_cast<int>(items_.size()))
    {
        selectedIndex_ = static_cast<int>(items_.size()) - 1;
    }
    return true;
}

void SceneManager::selectNext()
{
    if (items_.empty()) return;
    selectedIndex_ = (selectedIndex_ + 1) % static_cast<int>(items_.size());
}

void SceneManager::selectPrev()
{
    if (items_.empty()) return;
    selectedIndex_ = (selectedIndex_ - 1 + static_cast<int>(items_.size())) % static_cast<int>(items_.size());
}

void SceneManager::selectIndex(int index)
{
    if (index >= 0 && index < static_cast<int>(items_.size()))
    {
        selectedIndex_ = index;
    }
}

bool SceneManager::hasSelection() const
{
    return selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(items_.size());
}

SceneItem* SceneManager::selectedItem()
{
    if (!hasSelection()) return nullptr;
    return &items_[selectedIndex_];
}

const SceneItem* SceneManager::selectedItem() const
{
    if (!hasSelection()) return nullptr;
    return &items_[selectedIndex_];
}

void SceneManager::updateSelectedParams(double p1, double p2, double p3)
{
    SceneItem* item = selectedItem();
    if (!item || !item->curve) return;

    switch (item->kind)
    {
    case CurveKind::Circle:
        if (auto* c = dynamic_cast<Circle*>(item->curve.get()))
        {
            c->setRadius(p1);
        }
        break;
    case CurveKind::Ellipse:
        if (auto* e = dynamic_cast<Ellipse*>(item->curve.get()))
        {
            e->setRadii(p1, p2);
        }
        break;
    case CurveKind::Helix:
        if (auto* h = dynamic_cast<Helix*>(item->curve.get()))
        {
            h->setParams(p1, p2, p3);
        }
        break;
    }

    recalculatePivot(*item);
}

void SceneManager::recalculatePivot(SceneItem& item) const
{
    if (!item.curve) return;
    double turns = curve_cfg::DEFAULT_TURNS;
    if (const auto* h = dynamic_cast<const Helix*>(item.curve.get()))
    {
        turns = h->turns();
    }
    item.transform.pivot = motion::curveCenter(*item.curve, turns, curve_cfg::CURVE_SEGMENTS);
}

void SceneManager::clampItemToGrid(SceneItem& item, float extent) const
{
    if (!item.curve) return;

    double turns = curve_cfg::DEFAULT_TURNS;
    if (const auto* h = dynamic_cast<const Helix*>(item.curve.get()))
    {
        turns = h->turns();
    }

    float minX = 1e9f, maxX = -1e9f;
    float minY = 1e9f, maxY = -1e9f;
    float minZ = 1e9f, maxZ = -1e9f;

    const double tEnd = turns * curve_cfg::TWO_PI;
    for (int i = 0; i <= curve_cfg::BOUNDS_SAMPLES; ++i)
    {
        const auto p = item.curve->point(tEnd * i / curve_cfg::BOUNDS_SAMPLES);
        minX = std::min(minX, static_cast<float>(p[0]));
        maxX = std::max(maxX, static_cast<float>(p[0]));
        minY = std::min(minY, static_cast<float>(p[1]));
        maxY = std::max(maxY, static_cast<float>(p[1]));
        minZ = std::min(minZ, static_cast<float>(p[2]));
        maxZ = std::max(maxZ, static_cast<float>(p[2]));
    }

    const float minPosX = -extent - minX;
    const float maxPosX = extent - maxX;
    const float minPosY = -extent - minY;
    const float maxPosY = extent - maxY;
    const float minPosZ = -extent - minZ;
    const float maxPosZ = extent - maxZ;

    if (item.transform.position.x < minPosX) item.transform.position.x = minPosX;
    if (item.transform.position.x > maxPosX) item.transform.position.x = maxPosX;
    if (item.transform.position.y < minPosY) item.transform.position.y = minPosY;
    if (item.transform.position.y > maxPosY) item.transform.position.y = maxPosY;
    if (item.transform.position.z < minPosZ) item.transform.position.z = minPosZ;
    if (item.transform.position.z > maxPosZ) item.transform.position.z = maxPosZ;
}

void SceneManager::clampAllToGrid(float extent)
{
    for (auto& item : items_)
    {
        clampItemToGrid(item, extent);
    }
}

void SceneManager::printAllAt(double t) const
{
    std::cout << "--- Curves at t = " << t << " ---\n";
    for (size_t i = 0; i < items_.size(); ++i)
    {
        if (!items_[i].curve) continue;
        std::cout << "[" << i << "] ";
        items_[i].curve->printAt(t);
    }

    const auto circles = sortedCircles();
    std::cout << "Circles sorted by radius (" << circles.size() << "): ";
    for (size_t i = 0; i < circles.size(); ++i)
    {
        if (i > 0) std::cout << ", ";
        std::cout << circles[i]->radius();
    }
    std::cout << " | Sum: " << totalCirclesRadius() << "\n";
}

std::vector<std::shared_ptr<Circle>> SceneManager::sortedCircles() const
{
    std::vector<std::shared_ptr<Circle>> result;
    for (const auto& item : items_)
    {
        if (auto c = std::dynamic_pointer_cast<Circle>(item.curve))
        {
            result.push_back(std::move(c));
        }
    }

    std::sort(result.begin(), result.end(),
        [](const std::shared_ptr<Circle>& a, const std::shared_ptr<Circle>& b)
        {
            return a->radius() < b->radius();
        });

    return result;
}

double SceneManager::totalCirclesRadius() const
{
    const auto circles = sortedCircles();
    return std::accumulate(circles.begin(), circles.end(), 0.0,
        [](double acc, const std::shared_ptr<Circle>& c)
        {
            return acc + c->radius();
        });
}