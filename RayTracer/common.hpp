#pragma once

#define _USE_MATH_DEFINES

#include <cmath>
#include <math.h>
#include <sstream>
#include <string>
#include <ostream>
#include <algorithm>
#include <tuple>
#include <stdexcept>
#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <ctime>
#include <ppl.h>


#define EPSILON 1e-6
#define INFINITY 1e7

#define RAD2DEG(x) ((x) * 57.2957795131f)
#define DEG2RAD(x) ((x) * .01745329251f)
#define ROT_45 DEG2RAD(45.f)
#define ROT_90 DEG2RAD(90.f)
#define ROT_135 DEG2RAD(135.f)
#define ROT_180 DEG2RAD(180.f)
#define ROT_270 DEG2RAD(270.f)

#define SIGN(x) ((x) > 0 ? 1 : (x) < 0 ? -1 : 0)

#define OSTREAM_OPERATOR(x) friend inline std::ostream& operator<<(std::ostream& os, const x& value) { os << value.to_string(); return os; }
#define CPP_IS_FUCKING_RETARDED(x) inline x& operator =(const x& value) noexcept { return this == &value ? *this : *new(this)x(value); }

#define NAMEOF(x) #x

#define ABSTRACT(ret, name, ...) \
    virtual ret name(__VA_ARGS__) const = 0 \
    { \
        std::cerr << "Using the abstract function '" #ret " " #name "(" #__VA_ARGS__ ")'." << std::endl; \
        return ret(); \
    }


constexpr bool solve_quadratic(const float& a, const float& b, const float& c, float* __restrict const x0, float* __restrict const x1)
{
    const float discr = b * b - 4 * a * c;

    if (discr < 0)
        return false;
    else if (discr == 0)
        *x0 = *x1 = -.5 * b / a;
    else
    {
        const float q = (b > 0) ? -.5 * (b + std::sqrt(discr)) : -.5 * (b - std::sqrt(discr));

        *x0 = q / a;
        *x1 = c / q;
    }

    if (x0 > x1)
    {
        float tmp = *x0;

        *x0 = *x1;
        *x1 = tmp;
    }

    return true;
}

template<typename A, typename B>
inline std::unique_ptr<B> static_unique_ptr_cast(std::unique_ptr<A>&& p)
{
    B* const d = static_cast<B*>(p.release());

    return std::unique_ptr<B>(d);
}

template<typename A, typename B>
inline std::unique_ptr<B> dynamic_unique_ptr_cast(std::unique_ptr<A>&& p)
{
    if (B* result = dynamic_cast<B*>(p.get()))
    {
        p.release();

        return std::unique_ptr<B>(result);
    }
    else
        return std::unique_ptr<B>(nullptr);
}

