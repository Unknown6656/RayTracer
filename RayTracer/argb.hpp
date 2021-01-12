#pragma once

#include "common.hpp"


struct ARGB
{
    static const ARGB BLACK, WHITE, TRANSPARENT, RED, GREEN, BLUE;

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
    ARGB DiffuseColor;
    ARGB SpecularColor;
    ARGB EmissiveColor;
    double EmissiveIntensity;
    double Specularity;
    double SpecularIndex;
    double Reflectiveness;
    double Refractiveness;
    ARGB RefractiveIndex;


    inline double opacity() const noexcept
    {
        return DiffuseColor.A;
    }

    static inline const Material& diffuse(const ARGB& color) noexcept
    {
        Material mat;
        mat.DiffuseColor = color;
        mat.SpecularColor = ARGB::TRANSPARENT;
        mat.EmissiveColor = ARGB::TRANSPARENT;

        return mat;
    }

    static inline const Material& reflective(const ARGB& base, const double reflectiveness) noexcept
    {
        Material mat;
        mat.DiffuseColor = base;
        mat.Reflectiveness = reflectiveness;

        return mat;
    }

    static inline const Material& emissive(const ARGB& base, const double intensity) noexcept
    {
        Material mat;
        mat.DiffuseColor = base;
        mat.EmissiveColor = base;
        mat.EmissiveIntensity = intensity;

        return mat;
    }
};
