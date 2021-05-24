#pragma once

#include "ray2.hpp"
#include "../material.hpp"


namespace ray_tracer_2d
{
    class primitive2
    {
    protected:
        float _area;
    public:
        enum class primitive_type
        {
            line,
            circle,
        } type;
        material material;


        primitive2(float area, primitive_type type) noexcept
            : _area(area)
            , type(type)
        {
        }

        inline float surface_area() const noexcept
        {
            return _area;
        }

        virtual vec2 normal_at(const vec2& vec) const = 0;

        virtual bool intersect(const ray2& ray, float* const __restrict distance) const = 0;

        virtual std::string to_string() const noexcept = 0;

        OSTREAM_OPERATOR(primitive2);
    };

    // Line from A to B. "outside" is the right-hand side when viewing from A to B. The normal vector points to the "outside".
    class line2
        : public primitive2
    {
        const vec2 normal;
    public:
        const vec2 A, B;


        line2() noexcept
            : line2(vec2::Zero, vec2::UnitX)
        {
        }

        line2(const vec2& a, const vec2& b) noexcept
            : primitive2(0, primitive_type::line)
            , A(a)
            , B(b)
            , normal(B.sub(A).rotate(-M_PI_2).normalize())
        {
        }

        vec2 normal_at(const vec2&) const override
        {
            return normal;
        }

        bool intersect(const ray2& ray, float* const __restrict distance) const override
        {
            const vec2 v1 = ray.origin.sub(A);
            const vec2 v2 = B.sub(A);
            const vec2 v3 = (-ray.direction.Y, ray.direction.X);
            const float dot = v2.dot(v3);

            if (abs(dot) < EPSILON)
                return false;

            const float t1 = (v2.X * v1.Y - v2.Y * v1.X) / dot;
            const float t2 = v1.dot(v3) / dot;

            if (t1 >= 0.f && t2 >= 0.f && t2 <= 1.f)
            {
                *distance = t1;

                return true;
            }

            return false;
        }

        std::string to_string() const noexcept override
        {
            std::stringstream ss;

            ss << NAMEOF(triangle) << "[A=" << A << ",B=" << B << ",C=" << C << ",Area=" << _area << "]";

            return ss.str();
        }

        CPP_IS_FUCKING_RETARDED(line2);
    };

    class circle2
        : public primitive2
    {
        const float radius2;
    public:
        const vec2 center;
        const float radius;


        circle2() noexcept
            : sphere(vec2::Zero, 1.f)
        {
        }

        circle2(const vec2& center, const float& radius) noexcept
            : primitive2(3.14159265358979323846 * radius * radius, primitive_type::circle)
            , center(center)
            , radius(radius)
            , radius2(radius* radius)
        {
        }

        vec2 normal_at(const vec2& vec) const override
        {
            return vec.sub(center).normalize();
        }

        bool intersect(const ray2& ray, float* const __restrict distance) const override
        {
            

            // D * ray.direction.Y +/- SIGN(ray.direction.Y) * ray.direction.X * sqrt(radius2 - D * D)
            // - D * ray.direction.X +/- abs(ray.direction.Y) * sqrt(radius2 - D * D)





            // x1,x2 = ray.direction * (center - ray.origin) +/- sqrt( (ray.direction * (center - ray.origin))^2 + radius2 - (center - ray.origin).squared_length)

        }

        std::string to_string() const noexcept override
        {
            std::stringstream ss;

            ss << NAMEOF(circle) << "[C=" << center << ",R=" << radius << ",Area=" << _area << "]";

            return ss.str();
        }

        CPP_IS_FUCKING_RETARDED(circle2);
    };
};
