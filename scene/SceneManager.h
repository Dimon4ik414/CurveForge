#pragma once

#include "curve/Curve.h"
#include "motion/ObjectTransform.h"
#include <memory>
#include <vector>

class Circle;

enum class CurveKind
{
    Circle,
    Ellipse,
    Helix
};

struct SceneItem
{
    CurveKind kind = CurveKind::Circle;
    std::shared_ptr<Curve> curve;
    motion::Transform transform;
};

class SceneManager
{
public:
    void addCircle(double radius);
    void addEllipse(double rx, double ry);
    void addHelix(double r, double step, double turns);

    bool removeSelected();
    void selectNext();
    void selectPrev();
    void selectIndex(int index);

    bool hasSelection() const;
    int selectedIndex() const { return selectedIndex_; }
    SceneItem* selectedItem();
    const SceneItem* selectedItem() const;

    const std::vector<SceneItem>& items() const { return items_; }

    void updateSelectedParams(double p1, double p2, double p3);
    void recalculatePivot(SceneItem& item) const;
    void clampItemToGrid(SceneItem& item, float extent) const;
    void clampAllToGrid(float extent);

    void printAllAt(double t) const;
    std::vector<std::shared_ptr<Circle>> sortedCircles() const;
    double totalCirclesRadius() const;

private:
    void attachItem(SceneItem item);

    std::vector<SceneItem> items_;
    int selectedIndex_ = -1;
};