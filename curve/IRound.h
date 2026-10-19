#pragma once

class IRound {
public:
    virtual ~IRound() = default;
    virtual double radius() const = 0;
};