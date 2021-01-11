#pragma once

#include <cmath>
#include <sstream>
#include <string>
#include <ostream>

#define OSTREAM_OPERATOR(x) friend inline std::ostream& operator<<(std::ostream& os, const x& value) { os << value.to_string(); return os; }


struct ARGB
{
    double A, R, G, B;


    ARGB() noexcept : ARGB(0) {}

    ARGB(double gray) noexcept : ARGB(1, gray) {}

    ARGB(double a, double gray) noexcept : ARGB(gray, gray, gray) {}

    ARGB(double r, double g, double b) noexcept : ARGB(1, r, g, b) {}

    ARGB(double a, double r, double g, double b) noexcept
        : A(a), R(r), G(g), B(b)
    {
    }

    std::string to_string() const noexcept
    {
        std::stringstream ss;
        ss << '[' << A << "," << R << "," << G << "," << B << ']';

        return ss.str();
    }

    OSTREAM_OPERATOR(ARGB);

    inline ARGB operator-() const noexcept
    {
        return ARGB(-A, -R, -G, -B);
    }

    inline ARGB operator+(const ARGB& other) const noexcept
    {
        return ARGB(A + other.A, R + other.R, G + other.G, B + other.B);
    }

    inline ARGB operator-(const ARGB& other) const noexcept
    {
        return ARGB(A - other.A, R - other.R, G - other.G, B - other.B);
    }

    inline ARGB operator*(const double factor) const noexcept
    {
        return ARGB(A * factor, R * factor, G * factor, B * factor);
    }

    inline ARGB operator/(const double factor) const
    {
        return ARGB(A / factor, R / factor, G / factor, B / factor);
    }
};

struct Material
{
    double Opacity;
    ARGB DiffuseColor;
    ARGB SpecularColor;
    ARGB EmissiveColor;
    double Specularity;
    double SpecularIndex;
    double Reflectiveness;
    double Refractiveness;
    ARGB RefractiveIndex;
};
