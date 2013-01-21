#include "displacement.h"

Displacement::Displacement(const Point2f& from, const Point2f& to):from(from),to(to)
{

}

const Point2f Displacement::getDisplacement() const {
    return to-from;
}

const Point2f& Displacement::getFrom() const {
    return from;
}

const Point2f& Displacement::getTo() const {
    return to;
}

float Displacement::similarity(const Displacement& a, const Displacement& b)
{

}

