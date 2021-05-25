#pragma once

#include "ray3.hpp"
#include "../material.hpp"


namespace ray_tracer_3d
{
    struct hit_test
    {
        enum class hit_type
        {
            no_hit,
            hit,
            tangential_hit,
        } type = hit_type::no_hit;
        float distance = INFINITY;
        vec2 uv;


        TO_STRING(hit_test, (type == hit_type::hit ? "hit" : type == hit_type::tangential_hit ? "tangential-hit" : "no-hit") << ",D=" << distance << ",UV=" << uv);
    };

    class primitive
    {
    protected:
        float _area;
    public:
        enum class primitive_type
        {
            triangle,
            sphere,
        } type;
        material material;


        primitive(float area, primitive_type type) noexcept
            : _area(area)
            , type(type)
        {
        }

        inline float surface_area() const noexcept
        {
            return _area;
        }

        virtual vec3 normal_at(const vec3& vec) const = 0;

        virtual vec2 UV_at(const vec3& vec) const = 0;

        virtual void intersect(const ray3& ray, hit_test* const result) const = 0;

        virtual std::string to_string() const noexcept = 0;

        OSTREAM_OPERATOR(primitive);
    };

    class triangle
        : public primitive
    {
    public:
        const vec3 A, B, C;
        const vec3 non_normalized_normal;


        triangle() noexcept
            : triangle(vec3::Zero, vec3::UnitX, vec3::UnitY)
        {
        }

        triangle(const vec3& a, const vec3& b, const vec3& c) noexcept
            : primitive(B.sub(A).cross(C.sub(A)).length() / 2, primitive_type::triangle)
            , A(a)
            , B(b)
            , C(c)
            , non_normalized_normal(b.sub(a).cross(c.sub(a)))
        {
        }

        vec3 normal_at(const vec3& vec) const override
        {
            return non_normalized_normal; // TODO : interpolate between all points
        }

        vec2 UV_at(const vec3& vec) const override
        {
            const vec3 PA = A.sub(vec);
            const vec3 PB = B.sub(vec);
            const vec3 PC = C.sub(vec);

            return vec2(
                PB.cross(PC).length() / _area,
                PC.cross(PA).length() / _area
            );
        }

        bool möller_trumbore_intersect(
            const ray3& ray,
            float* const __restrict t,
            float* const __restrict u,
            float* const __restrict v,
            bool* const __restrict hit_backface
        ) const noexcept
        {
            if (!t || !u || !v)
                return false;

            const vec3 edgeAB = B.sub(A);
            const vec3 edgeAC = C.sub(A);
            const vec3 pvec = ray.direction.cross(edgeAC);
            const float det = edgeAB.dot(pvec);

            if (std::abs(det) < EPSILON)
                return false;
            else if (hit_backface)
                *hit_backface = det < 0;

            const float inv_det = 1 / det;
            const vec3 tvec = ray.origin.sub(A);

            *u = tvec.dot(pvec) * inv_det;

            if (*u < 0 || *u > 1)
                return false;

            const vec3 qvec = tvec.cross(edgeAB);

            *v = ray.direction.dot(qvec) * inv_det;

            if (*v < 0 || *u + *v > 1)
                return false;

            *t = edgeAC.dot(qvec) * inv_det;

            return true;
        }

        void intersect(const ray3& ray, hit_test* const result) const override
        {
            *result = hit_test();

            float u, v;
            bool inside = false;
            bool hit = möller_trumbore_intersect(ray, &result->distance, &u, &v, &inside);

            result->uv = vec2(u, v);
            result->type = hit ? inside ? hit_test::hit_type::hit
                                        : hit_test::hit_type::tangential_hit
                               : hit_test::hit_type::no_hit;
        }

        TO_STRING(triangle, "A=" << A << ",B=" << B << ",C=" << C << ",N=" << non_normalized_normal << ",Area=" << _area)
        CPP_IS_FUCKING_RETARDED(triangle);
    };

    class sphere
        : public primitive
    {
        const float radius2;
    public:
        const vec3 center;
        const float radius;


        sphere() noexcept
            : sphere(vec3::Zero, 1.f)
        {
        }

        sphere(const vec3& center, const float& radius) noexcept
            : primitive(4 * 3.14159265358979323846 * radius * radius, primitive_type::sphere)
            , center(center)
            , radius(radius)
            , radius2(radius* radius)
        {
        }

        vec3 normal_at(const vec3& vec) const override
        {
            return vec.sub(center).normalize();
        }

        vec2 UV_at(const vec3& vec) const override
        {
            const vec3& N = normal_at(vec);

            return vec2(
                std::atan2(N.X, N.Z),
                std::acos(N.Y)
            );
        }

        void intersect(const ray3& ray, hit_test* const result) const override
        {
            const vec3 oc = ray.origin.sub(center);
            const float a = ray.direction.squared_length();
            const float b = 2 * oc.dot(ray.direction);
            const float c = oc.squared_length() - radius2;
            const float discr = b * b - 4 * a * c;

            if (discr < -EPSILON)
                result->type = hit_test::hit_type::no_hit;
            else
            {
                const float fac = .5f / a;
                const float dist = std::max(-b - sqrt(discr), -b + sqrt(discr)) * fac;

                if (dist > 0)
                {
                    const vec3 point = ray(dist);

                    result->uv = UV_at(point);
                    result->distance = dist;
                    result->type = hit_test::hit_type::hit;
                }
                else
                    result->type = hit_test::hit_type::tangential_hit;
            }
        }

        TO_STRING(sphere, "C=" << center << ",R=" << radius << ",Area=" << _area);
        CPP_IS_FUCKING_RETARDED(sphere);
    };
};
