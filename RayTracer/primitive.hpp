#pragma once

#include "ray.hpp"


struct SphereData
{
    const Vec3 center;
    const double radius;
    const double radius2;


    SphereData() noexcept : SphereData(Vec3(), 1) {}

    SphereData(const Vec3& center, const double& rad) noexcept
        : center(center)
        , radius(rad)
        , radius2(rad* rad)
    {
    }

    CPP_IS_FUCKING_RETARDED(SphereData);
};

struct TriangleData
{
    const Vec3 A, B, C;
    const Vec3 non_normalized_normal;


    TriangleData() noexcept : TriangleData(Vec3::Zero, Vec3::UnitX, Vec3::UnitY) {}

    TriangleData(const Vec3& a, const Vec3& b, const Vec3& c) noexcept
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
    const double _area;


public:
    Primitive(const Vec3& A, const Vec3& B, const Vec3& C) noexcept
        : type(PrimitiveType::Triangle)
        , data(GeometryData(TriangleData(A, B, C)))
        , _area(B.sub(A).cross(C.sub(A)).length() / 2)
    {
    }

    Primitive(const Vec3& center, const double& radius) noexcept
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

    inline double surface_area() const noexcept
    {
        return _area;
    }

    inline Vec3 normal_at(const Vec3& vec) const
    {
        switch (type)
        {
            case PrimitiveType::Sphere:
                return vec.sub(data.sphere.center).normalize();
            case PrimitiveType::Triangle:
                return data.triangle.non_normalized_normal; // TODO : interpolate between all points
        }

        return Vec3::Zero;
    }

    inline std::tuple<double, double> UV_at(const Vec3& vec) const
    {
        double u = 0.0;
        double v = 0.0;

        switch (type)
        {
            case PrimitiveType::Sphere:
            {
                const Vec3& N = normal_at(vec);

                u = std::atan2(N.X, N.Z);
                v = std::acos(N.Y);

                break;
            }
            case PrimitiveType::Triangle:
            {
                const Vec3 PA = data.triangle.A.sub(vec);
                const Vec3 PB = data.triangle.B.sub(vec);
                const Vec3 PC = data.triangle.C.sub(vec);

                u = PB.cross(PC).length() / _area;
                v = PC.cross(PA).length() / _area;

                break;
            }
        }

        return std::make_tuple(u, v);
    }

    inline bool intersect(const Ray& ray, double* const __restrict distance, bool* const __restrict inside, std::tuple<double, double>* const uv = nullptr) const
    {
        bool hit = false;

        switch (type)
        {
            case PrimitiveType::Sphere:
            {
                const Vec3 L = ray.Origin - data.sphere.center;
                const double a = ray.Direction.norm();
                const double b = 2 * ray.Direction.dot(L);
                const double c = L.norm() - data.sphere.radius2;
                double x0, x1;

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
                double u, v;

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

