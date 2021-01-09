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
    const Vec3 Normal;


    Triangle() noexcept : Triangle(Vec3(), Vec3(), Vec3()) { }

    Triangle(const Vec3& a, const Vec3& b, const Vec3& c) noexcept
        : Shape()
        , A(a)
        , B(b)
        , C(c)
        , Normal(b.sub(a).cross(c.sub(a)).normalize())
    {
    }

    virtual Vec3 normal_at(const Vec3& vec) const noexcept
    {
        return Normal;
    }

    virtual bool intersect(const Ray& ray, double* const __restrict distance, bool* const __restrict inside) const
    {
        return false;
        std::cout << ray << std::endl;


        const Vec3 v0v1 = B.sub(A);
        const Vec3 v0v2 = C.sub(A);
        //const Vec3 N = v0v1.crossProduct(v0v2);

        const double cos_angle = Normal.dot(ray.Direction);

        if (fabs(cos_angle) < EPSILON)
        {
            printf("parallel\n");
            return false;
        }

        const double d = Normal.dot(A);
        
        *distance = (Normal.dot(ray.Origin) + d) / cos_angle;

        if (*distance < 0)
        {
            printf("behind origin\n");
            return false;
        }

        const Vec3 P = ray.evaluate(*distance);
        Vec3 C = v0v1.cross(P.sub(A));

        if (Normal.dot(C) < 0)
        {
            printf("outside A-B\n");
            return false;
        }

        C = v0v2.cross(P.sub(B));

        if (Normal.dot(C) < 0)
        {
            printf("outside A-C\n");
            return false;
        }

        C = A.sub(B).cross(P.sub(C));

        if (Normal.dot(C) < 0)
        {
            printf("outside B-C\n");
            return false;
        }


        *inside = ray.Direction.dot(Normal) > 0;

        printf("hit\n");
        return true;
    }
};


// TODO : more shapes
