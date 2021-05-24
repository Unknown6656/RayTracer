#pragma once

#include "vec3.hpp"


namespace ray_tracer_3d
{
    struct ray3
    {
        const vec3 origin;
        const vec3 direction;
        const size_t iteration_depth;


        ray3() noexcept : ray3(vec3(), vec3(), 0) {}

        ray3(const vec3& origin, const vec3& dir) noexcept : ray3(origin, dir, 0) {}

        ray3(const vec3& origin, const vec3& dir, const size_t depth) noexcept
            : origin(origin)
            , direction(dir.normalize())
            , iteration_depth(depth)
        {
        }

        inline vec3 evaluate(const float at) const noexcept
        {
            return origin + direction * at;
        }

        inline ray3 create_next(const float at, const vec3& next_dir) const noexcept
        {
            return ray3(evaluate(at), next_dir, iteration_depth + 1);
        }

        bool möller_trumbore_intersect(
            const vec3& A, const vec3& B, const vec3& C,
            float* const __restrict t,
            float* const __restrict u,
            float* const __restrict v,
            bool* const __restrict hit_backface
        ) const noexcept
        {
            if (!t || !u || !v)
                return false;

            const vec3 edgeAB = B - A;
            const vec3 edgeAC = C - A;
            const vec3 pvec = direction.cross(edgeAC);
            const float det = edgeAB.dot(pvec);

            if (std::abs(det) < EPSILON)
                return false;
            else if (hit_backface)
                *hit_backface = det < 0;

            const float inv_det = 1 / det;
            const vec3 tvec = origin - A;

            *u = tvec.dot(pvec) * inv_det;

            if (*u < 0 || *u > 1)
                return false;

            const vec3 qvec = tvec.cross(edgeAB);

            *v = direction.dot(qvec) * inv_det;

            if (*v < 0 || *u + *v > 1)
                return false;

            *t = edgeAC.dot(qvec) * inv_det;

            return true;
        }

        inline std::string to_string() const noexcept
        {
            std::stringstream ss;
            ss << "[O=" << origin << ", D=" << direction << ", I=" << iteration_depth << ']';

            return ss.str();
        }

        OSTREAM_OPERATOR(ray3);
        CPP_IS_FUCKING_RETARDED(ray3);

        inline vec3 operator()(const float at) const noexcept
        {
            return evaluate(at);
        }
    };

    struct ray_trace_iteration
    {
        ray3 Ray;
        bool Hit;
        float Distance;
        ARGB ComputedColor;
        vec3 SurfaceNormal;
        vec3 IntersectionPoint;
        std::tuple<float, float> UVCoordinates;
        int TriangleIndex;


        inline std::string to_string() const noexcept
        {
            std::stringstream ss;

            ss << "[R=" << Ray << ", D=" << Distance << ", N=" << SurfaceNormal << ", C=" << ComputedColor << "]";

            return ss.str();
        }

        OSTREAM_OPERATOR(ray_trace_iteration);
    };

    typedef std::vector<ray_trace_iteration> ray_trace_result;
};
