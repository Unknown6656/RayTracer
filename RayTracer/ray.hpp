#pragma once

#include "vec3.hpp"

#define EULER_ARG const Vec3& euler_angles
#define EULER_OPTARG EULER_ARG = Vec3(0, 0, 0)
#define ROT_90 1.5707963267949
#define ROT_180 3.14159265358979
#define ROT_270 4.71238898038469


struct Ray
{
    const Vec3 Origin;
    const Vec3 Direction;
    const size_t IterationDepth;


    Ray() noexcept : Ray(Vec3(), Vec3(), 0) { }

    Ray(const Vec3& origin, const Vec3& dir) noexcept : Ray(origin, dir, 0) { }

    Ray(const Vec3& origin, const Vec3& dir, const size_t depth) noexcept
        : Origin(origin)
        , Direction(dir.normalize())
        , IterationDepth(depth)
    {
    }

    inline Vec3 evaluate(const double at) const noexcept
    {
        return Origin + Direction * at;
    }

    inline Ray create_next(const double at, const Vec3& next_dir) const noexcept
    {
        return Ray(evaluate(at), next_dir, IterationDepth + 1);
    }

    bool MöllerTrumboreIntersect(
        const Vec3& A, const Vec3& B, const Vec3& C,
        double* const __restrict t, double* const __restrict u, double* const __restrict v,
        bool* const __restrict hit_backface
    ) const noexcept
    {
        if (!t || !u || !v)
            return false;

        const Vec3 edgeAB = B - A;
        const Vec3 edgeAC = C - A;
        const Vec3 pvec = Direction.cross(edgeAC);
        const double det = edgeAB.dot(pvec);

        if (fabs(det) < EPSILON)
            return false;
        else if (hit_backface)
            *hit_backface = det < 0;

        const double inv_det = 1 / det;
        const Vec3 tvec = Origin - A;

        *u = tvec.dot(pvec) * inv_det;

        if (*u < 0 || *u > 1)
            return false;

        const Vec3 qvec = tvec.cross(edgeAB);

        *v = Direction.dot(qvec) * inv_det;

        if (*v < 0 || *u + *v > 1)
            return false;

        *t = edgeAC.dot(qvec) * inv_det;

        return true;
    }

    std::string to_string() const noexcept
    {
        std::stringstream ss;
        ss << "[O=" << Origin << ", D=" << Direction << ", I=" << IterationDepth << ']';

        return ss.str();
    }

    // why? because c++ is fucking retarded, that's why!
    inline Ray& operator=(const Ray& value)
    {
        return this == &value ? *this : *new(this)Ray(value);
    }

    OSTREAM_OPERATOR(Ray);
};

struct RayTraceIteration
{
    Ray Ray;
    double Distance;
    ARGB ComputedColor;
    Vec3 SurfaceNormal;


    std::string to_string() const
    {
        std::stringstream ss;
        ss << "[R=" << Ray << ", D=" << Distance << ", N=" << SurfaceNormal << ", C=" << ComputedColor << "]";

        return ss.str();
    }

    OSTREAM_OPERATOR(RayTraceIteration);
};

typedef std::vector<RayTraceIteration> RayTraceResult;
