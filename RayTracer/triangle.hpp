#pragma once

#include "ray.hpp"


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

struct Shape
{
    Material Material;

    virtual Vec3 normal_at(const Vec3& vec) const = 0;
    virtual bool intersect(const Ray& ray, double* const distance, bool* const inside) const = 0;
};

struct Triangle
    : virtual public Shape
{
    const Vec3 A, B, C;
private:
    /// THIS IS NOT NORMALIZED !
    const Vec3 Normal;
public:


    Triangle() noexcept : Triangle(Vec3(), Vec3(), Vec3()) { }

    Triangle(const Vec3& a, const Vec3& b, const Vec3& c) noexcept
        : Shape()
        , A(a)
        , B(b)
        , C(c)
        , Normal(b.sub(a).cross(c.sub(a)))
    {
    }

    virtual Vec3 normal_at(const Vec3& vec) const noexcept
    {
        return Normal;
    }

    virtual bool intersect(const Ray& ray, double* const __restrict distance, bool* const __restrict inside) const
    {
        // möller trumbore intersection algorithm

        const Vec3 edgeAB = B.sub(A);
        const Vec3 edgeAC = C.sub(A);
        const double a = Normal.dot(ray.Direction);

        std::stringstream ss; ss << A << B << C << " ray=" << ray << " ";

        if (fabs(a) < EPSILON)
        {
            ss << "parallel\n"; std::cout << ss.str();
            return false;
        }

        const double f = 1.0 / a;

        auto s = ray.Origin.sub(A);
        auto u = f * s.dot(Normal);

        if (u < 0.0 || u > 1.0)
        {
            ss << "u:" << u << "\n"; std::cout << ss.str();
            return false;
        }

        auto q = s.cross(edgeAC);
        auto v = f * ray.Direction.dot(q);

        if (v < 0.0 || (u + v) > 1.0)
        {
            ss << "v:" << v << ", u+v:" << u+v << "\n"; std::cout << ss.str();
            return false;
        }

        *distance = f * edgeAC.dot(q);

        if (*distance < EPSILON)
        {
            ss << "behind origin "<< *distance << "\n"; std::cout << ss.str();
            return false;
        }

        *inside = ray.Direction.dot(Normal) > 0;

        ss << "hit\n"; std::cout << ss.str();
        return true;
    }
};


// TODO : more shapes
