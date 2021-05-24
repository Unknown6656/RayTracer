#pragma once

#include "vec3.hpp"
#include "../2D/vec2.hpp"

using namespace ray_tracer_2d;


namespace ray_tracer_3d
{
    struct ray3
    {
        const vec3 origin;
        const vec3 direction;
        const size_t iteration_depth;
        const float current_refraction_index;
        const bool is_inside;


        ray3() noexcept
            : ray3(vec3(), vec3())
        {
        }

        ray3(const vec3& origin, const vec3& dir) noexcept
            : ray3(origin, dir, 0, 1.f, false)
        {
        }

        ray3(const vec3& origin, const vec3& dir, const size_t depth, const float refraction_index, const bool inside) noexcept
            : origin(origin)
            , direction(dir.normalize())
            , iteration_depth(depth)
            , current_refraction_index(refraction_index)
            , is_inside(inside)
        {
        }

        inline vec3 evaluate(const float at) const noexcept
        {
            return origin + direction * at;
        }

        inline ray3 create_next(const float at, const vec3& next_dir, const float new_refraction_index) const noexcept
        {
            return ray3(evaluate(at), next_dir, iteration_depth + 1, new_refraction_index, !is_inside);
        }

        TO_STRING(ray3, "O=" << origin << ",D=" << direction << ",It=" << iteration_depth << ",Rho=" << current_refraction_index << ",In=" << is_inside);
        CPP_IS_FUCKING_RETARDED(ray3);

        inline vec3 operator()(const float at) const noexcept
        {
            return evaluate(at);
        }
    };
};
