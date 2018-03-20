#pragma once
#include <cmath>
#include <cfenv>
#include "common.h"

namespace fx {

template <int N>
class fixed_point
{
public:
    fixed_point() : value_(0)
    {
    }

    fixed_point(double d) : value_(normalize(d))
    {        
    }

    ALWAYS_INLINE static double normalize(double d)
    {
        return round(d * M) / M;
    }

    fixed_point operator+(fixed_point d) const
    {
        return fixed_point(value_ + d.value_);
    }

    fixed_point operator+(double d) const
    {
        return fixed_point(value_ + d);
    }

    fixed_point operator-(fixed_point d) const
    {
        return fixed_point(value_ - d.value_);
    }

    fixed_point operator-(double d) const
    {
        return fixed_point(value_ - d);
    }

    fixed_point operator*(double d) const
    {
        return fixed_point(value_ * d);
    }

    fixed_point operator*(int n) const
    {
        return fixed_point(value_ * n);
    }

    fixed_point operator/(double d) const
    {
        return fixed_point(value_ / d);
    }

    fixed_point operator/(int n) const
    {
        return fixed_point(value_ / n);
    }

    bool operator==(fixed_point d) const
    {
        return value_ == d.value_;
    }

    bool operator==(double d) const
    {
        return value_ == normalize(d);
    }

    bool operator!=(fixed_point d) const
    {
        return !(*this == d);
    }

    bool operator!=(double d) const
    {
        return !(*this == d);
    }

    bool operator<(fixed_point d) const
    {
        return value_ < d.value_;
    }

    bool operator<(double d) const
    {
        return value_ < normalize(d);
    }

    bool operator<=(fixed_point d) const
    {
        return value_ <= d.value_;
    }

    bool operator<=(double d) const
    {
        return value_ <= normalize(d);
    }

    bool operator>(fixed_point d) const
    {
        return value_ > d.value_;
    }

    bool operator>(double d) const
    {
        return value_ > normalize(d);
    }

    bool operator>=(fixed_point d) const
    {
        return value_ >= d.value_;
    }

    bool operator>=(double d) const
    {
        return value_ >= normalize(d);
    }

    operator double()
    {
        return value_;
    }

    operator double() const
    {
        return value_;
    }

private:
    constexpr static int pow10(int n) // 10^n
    {
        return (n > 0) ? 10 * pow10(n - 1) : 1;
    }

private:
    static const int M = pow10(N);
    double value_;
};
} // namespace fx
