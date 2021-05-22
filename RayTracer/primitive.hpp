#pragma once

#include "ray.hpp"


enum class primitive_type
{
    triangle,
    sphere,
};

class primitive
{
protected:
    float _area;
public:
    primitive_type type;
    Material material;


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

    virtual std::tuple<float, float> UV_at(const vec3& vec) const = 0;

    virtual bool intersect(const ray& ray, float* const __restrict distance, bool* const __restrict inside, std::tuple<float, float>* const __restrict uv = nullptr) const = 0;

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

    std::tuple<float, float> UV_at(const vec3& vec) const override
    {
        float u = 0.0;
        float v = 0.0;
        const vec3 PA = A.sub(vec);
        const vec3 PB = B.sub(vec);
        const vec3 PC = C.sub(vec);

        u = PB.cross(PC).length() / _area;
        v = PC.cross(PA).length() / _area;

        return std::make_tuple(u, v);
    }

    bool intersect(const ray& ray, float* const __restrict distance, bool* const __restrict inside, std::tuple<float, float>* const __restrict uv = nullptr) const override
    {
        float u, v;
        bool hit = ray.möller_trumbore_intersect(A, B, C, distance, &u, &v, inside);

        if (uv)
            *uv = std::make_tuple(u, v);

        return hit;
    }

    std::string to_string() const noexcept override
    {
        std::stringstream ss;

        ss << "triangle[A=" << A << ",B=" << B << ",C=" << C << ",N=" << non_normalized_normal << ",Area=" << _area << "]";

        return ss.str();
    }

    CPP_IS_FUCKING_RETARDED(triangle);
};

class sphere
    : public primitive
{
public:
    const vec3 center;
    const float radius;
    const float radius2;


    sphere() noexcept
        : sphere(vec3::Zero, 1.f)
    {
    }

    sphere(const vec3& center, const float& radius) noexcept
        : primitive(4 * 3.14159265358979323846 * radius * radius, primitive_type::sphere)
        , center(center)
        , radius(radius)
        , radius2(radius * radius)
    {
    }

    vec3 normal_at(const vec3& vec) const override
    {
        return vec.sub(center).normalize();
    }

    std::tuple<float, float> UV_at(const vec3& vec) const override
    {
        float u = 0.0;
        float v = 0.0;
        const vec3& N = normal_at(vec);

        u = std::atan2(N.X, N.Z);
        v = std::acos(N.Y);

        return std::make_tuple(u, v);
    }

    bool intersect(const ray& ray, float* const __restrict distance, bool* const __restrict inside, std::tuple<float, float>* const __restrict uv = nullptr) const override
    {
        const vec3 L = ray.origin - center;
        const float a = ray.direction.squared_length();
        const float b = 2 * ray.direction.dot(L);
        const float c = L.squared_length() - radius2;
        float x0, x1;
        bool hit = solve_quadratic(a, b, c, &x0, &x1);

        if (x0 < 0)
            x0 = x1;

        *inside = x0 < 0;
        *distance = x0;
        *uv = UV_at(ray.evaluate(x0));

        return hit;
    }

    std::string to_string() const noexcept override
    {
        std::stringstream ss;

        ss << "sphere[C=" << center << ",R=" << radius << ",Area=" << _area << "]";

        return ss.str();
    }

    CPP_IS_FUCKING_RETARDED(sphere);
};

