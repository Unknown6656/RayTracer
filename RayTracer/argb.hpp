#pragma once

#include "common.hpp"


struct ARGB
{
    static const ARGB BLACK, WHITE, TRANSPARENT, RED, GREEN, BLUE;

    float A, R, G, B;


    ARGB() noexcept : ARGB(0) {}

    ARGB(float gray) noexcept : ARGB(1, gray) {}

    ARGB(float a, float gray) noexcept : ARGB(gray, gray, gray) {}

    ARGB(float r, float g, float b) noexcept : ARGB(1, r, g, b) {}

    ARGB(float a, float r, float g, float b) noexcept
        : A(a), R(r), G(g), B(b)
    {
    }

    inline std::string to_string() const noexcept
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

    inline ARGB operator*(const float factor) const noexcept
    {
        return ARGB(A * factor, R * factor, G * factor, B * factor);
    }

    inline ARGB operator*(const ARGB& other) const noexcept
    {
        return ARGB(A * other.A, R * other.R, G * other.G, B * other.B);
    }

    inline ARGB operator/(const float factor) const
    {
        return ARGB(A / factor, R / factor, G / factor, B / factor);
    }
};

struct Material
{
    ARGB DiffuseColor;
    ARGB SpecularColor;
    ARGB EmissiveColor;
    float EmissiveIntensity;
    float Specularity;
    float SpecularIndex;
    float Reflectiveness;
    float Refractiveness;
    ARGB RefractiveIndex;


    inline float opacity() const noexcept
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

    static inline const Material& reflective(const ARGB& base, const float reflectiveness) noexcept
    {
        Material mat;
        mat.DiffuseColor = base;
        mat.Reflectiveness = reflectiveness;

        return mat;
    }

    static inline const Material& emissive(const ARGB& base, const float intensity) noexcept
    {
        Material mat;
        mat.DiffuseColor = base;
        mat.EmissiveColor = base;
        mat.EmissiveIntensity = intensity;

        return mat;
    }
};
