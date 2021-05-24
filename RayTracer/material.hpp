#pragma once

#include "argb.hpp"


struct material
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

    static inline const material& diffuse(const ARGB& color) noexcept
    {
        material mat;
        mat.DiffuseColor = color;
        mat.SpecularColor = ARGB::TRANSPARENT;
        mat.EmissiveColor = ARGB::TRANSPARENT;

        return mat;
    }

    static inline const material& reflective(const ARGB& base, const float reflectiveness) noexcept
    {
        material mat;
        mat.DiffuseColor = base;
        mat.Reflectiveness = reflectiveness;

        return mat;
    }

    static inline const material& emissive(const ARGB& base, const float intensity) noexcept
    {
        material mat;
        mat.DiffuseColor = base;
        mat.EmissiveColor = base;
        mat.EmissiveIntensity = intensity;

        return mat;
    }
};
