#pragma once

#include "ray.hpp"


struct SphereData
{
    const vec3 center;
    const float radius;
    const float radius2;


    SphereData() noexcept : SphereData(vec3(), 1) {}

    SphereData(const vec3& center, const float& rad) noexcept
        : center(center)
        , radius(rad)
        , radius2(rad * rad)
    {
    }

    CPP_IS_FUCKING_RETARDED(SphereData);
};

struct TriangleData
{
    const vec3 A, B, C;
    const vec3 non_normalized_normal;


    TriangleData() noexcept : TriangleData(vec3::Zero, vec3::UnitX, vec3::UnitY) {}

    TriangleData(const vec3& a, const vec3& b, const vec3& c) noexcept
        : A(a)
        , B(b)
        , C(c)
        , non_normalized_normal(b.sub(a).cross(c.sub(a)))
    {
    }

    CPP_IS_FUCKING_RETARDED(TriangleData);
};

union GeometryData
{
    const TriangleData triangle;
    const SphereData sphere;


    GeometryData(const SphereData& sph) noexcept : sphere(sph) {}
    GeometryData(const TriangleData& tri) noexcept : triangle(tri) {}

    CPP_IS_FUCKING_RETARDED(GeometryData);
};

struct Primitive
{
    enum class PrimitiveType
    {
        Triangle,
        Sphere,
        // TODO : more?
    };

    const PrimitiveType type;
    const GeometryData data;
    Material material;
private:
    const float _area;


public:
    Primitive(const vec3& A, const vec3& B, const vec3& C) noexcept
        : type(PrimitiveType::Triangle)
        , data(GeometryData(TriangleData(A, B, C)))
        , _area(B.sub(A).cross(C.sub(A)).length() / 2)
    {
    }

    Primitive(const vec3& center, const float& radius) noexcept
        : type(PrimitiveType::Sphere)
        , data(SphereData(center, radius))
        , _area(4 * 3.14159265358979323846 * radius * radius)
    {
    }

    inline Material get_material() const noexcept
    {
        return material;
    }

    inline void set_material(const Material& mat) noexcept
    {
        material = mat;
    }

    inline float surface_area() const noexcept
    {
        return _area;
    }

    inline vec3 normal_at(const vec3& vec) const
    {
        switch (type)
        {
            case PrimitiveType::Sphere:
                return vec.sub(data.sphere.center).normalize();
            case PrimitiveType::Triangle:
                return data.triangle.non_normalized_normal; // TODO : interpolate between all points
        }

        return vec3::Zero;
    }

    inline std::tuple<float, float> UV_at(const vec3& vec) const
    {
        float u = 0.0;
        float v = 0.0;

        switch (type)
        {
            case PrimitiveType::Sphere:
            {
                const vec3& N = normal_at(vec);

                u = std::atan2(N.X, N.Z);
                v = std::acos(N.Y);

                break;
            }
            case PrimitiveType::Triangle:
            {
                const vec3 PA = data.triangle.A.sub(vec);
                const vec3 PB = data.triangle.B.sub(vec);
                const vec3 PC = data.triangle.C.sub(vec);

                u = PB.cross(PC).length() / _area;
                v = PC.cross(PA).length() / _area;

                break;
            }
        }

        return std::make_tuple(u, v);
    }

    inline bool intersect(const ray& ray, float* const __restrict distance, bool* const __restrict inside, std::tuple<float, float>* const uv = nullptr) const
    {
        bool hit = false;

        switch (type)
        {
            case PrimitiveType::Sphere:
            {
                const vec3 L = ray.origin - data.sphere.center;
                const float a = ray.direction.squared_length();
                const float b = 2 * ray.direction.dot(L);
                const float c = L.squared_length() - data.sphere.radius2;
                float x0, x1;

                hit = solve_quadratic(a, b, c, &x0, &x1);

                if (x0 < 0)
                    x0 = x1;

                *inside = x0 < 0;
                *distance = x0;
                *uv = UV_at(ray.evaluate(x0));

                break;
            }
            case PrimitiveType::Triangle:
            {
                float u, v;

                hit = ray.MöllerTrumboreIntersect(data.triangle.A, data.triangle.B, data.triangle.C, distance, &u, &v, inside);

                if (uv)
                    *uv = std::make_tuple(u, v);

                break;
            }
        }

        return hit;
    }

    inline std::string to_string() const noexcept
    {
        std::stringstream ss;

        switch (type)
        {
            case PrimitiveType::Sphere:
                ss << "sphere[C=" << data.sphere.center << ",R=" << data.sphere.radius;

                break;
            case PrimitiveType::Triangle:
                ss << "triangle[A=" << data.triangle.A << ",B=" << data.triangle.B << ",C=" << data.triangle.C << ",N=" << data.triangle.non_normalized_normal;

                break;
        }

        ss << ",Area=" << _area << "]";

        return ss.str();
    }

    OSTREAM_OPERATOR(Primitive);
    CPP_IS_FUCKING_RETARDED(Primitive);
};

