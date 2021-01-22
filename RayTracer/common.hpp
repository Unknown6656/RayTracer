#pragma once

#include <cmath>
#include <math.h>
#include <sstream>
#include <string>
#include <ostream>
#include <algorithm>
#include <vector>
#include <tuple>


#define EPSILON 1e-6
#define INFINITY 1e7

#define OSTREAM_OPERATOR(x) friend inline std::ostream& operator<<(std::ostream& os, const x& value) { os << value.to_string(); return os; }
#define CPP_IS_FUCKING_RETARDED(x) inline x& operator =(const x& value) noexcept { return this == &value ? *this : *new(this)x(value); }


constexpr bool solve_quadratic(const double& a, const double& b, const double& c, double* __restrict const x0, double* __restrict const x1)
{
    const double discr = b * b - 4 * a * c;

    if (discr < 0)
        return false;
    else if (discr == 0)
        *x0 = *x1 = -.5 * b / a;
    else
    {
        const double q = (b > 0) ? -.5 * (b + std::sqrt(discr)) : -.5 * (b - std::sqrt(discr));

        *x0 = q / a;
        *x1 = c / q;
    }

    if (x0 > x1)
    {
        double tmp = *x0;

        *x0 = *x1;
        *x1 = tmp;
    }

    return true;
}
