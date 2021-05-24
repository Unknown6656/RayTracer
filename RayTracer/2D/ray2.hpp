#pragma once

#include "vec2.hpp"


namespace ray_tracer_2d
{
    struct ray2
    {
        const vec2 origin;
        const vec2 direction;
        const size_t iteration_depth;
        const float current_refraction_index;
        const bool is_inside;


        ray2() noexcept
            : ray2(vec2(), vec2())
        {
        }

        ray2(const vec2& origin, const vec2& dir) noexcept
            : ray2(origin, dir, 0, 1.f, false)
        {
        }

        ray2(const vec2& origin, const vec2& dir, const size_t depth, const float refraction_index, const bool inside) noexcept
            : origin(origin)
            , direction(dir.normalize())
            , iteration_depth(depth)
            , current_refraction_index(refraction_index)
            , is_inside(inside)
        {
        }

        inline vec2 evaluate(const float at) const noexcept
        {
            return origin + direction * at;
        }

        inline ray2 create_next(const float at, const vec2& next_dir, const float new_refraction_index) const noexcept
        {
            return ray2(evaluate(at), next_dir, iteration_depth + 1, new_refraction_index, !is_inside);
        }

        TO_STRING(ray2, "O=" << origin << ", D=" << direction << ", I=" << iteration_depth);
        CPP_IS_FUCKING_RETARDED(ray2);

        inline vec2 operator()(const float at) const noexcept
        {
            return evaluate(at);
        }
    };

    //struct ray_trace_iteration
    //{
    //    ray2 Ray;
    //    bool Hit;
    //    float Distance;
    //    ARGB ComputedColor;
    //    vec2 SurfaceNormal;
    //    vec2 IntersectionPoint;
    //    std::tuple<float, float> UVCoordinates;
    //    int TriangleIndex;
    //
    //
    //    TO_STRING(ray_trace_iteration, "R=" << Ray << ", D=" << Distance << ", N=" << SurfaceNormal << ", C=" << ComputedColor);
    //};

    //typedef std::vector<ray_trace_iteration> ray_trace_result;
};
