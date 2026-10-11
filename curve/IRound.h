#pragma once

// Дополнительная способность: у кривой есть радиус.
// Наследуют только те фигуры, у которых радиус осмыслен.
class IRound {
public:
    virtual ~IRound() = default;
    virtual double radius() const = 0;
};