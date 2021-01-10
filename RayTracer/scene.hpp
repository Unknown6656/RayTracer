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

struct Triangle final
{
    const Vec3 A, B, C;
private:
    /// THIS IS NOT NORMALIZED !
    const Vec3 Normal;
public:


    Triangle() noexcept : Triangle(Vec3(), Vec3(), Vec3()) { }

    Triangle(const Vec3& a, const Vec3& b, const Vec3& c) noexcept
        : A(a)
        , B(b)
        , C(c)
        , Normal(b.sub(a).cross(c.sub(a)))
    {
    }

    inline Vec3 normal_at(const Vec3& vec) const noexcept
    {
        return Normal;
    }

    inline bool intersect(const Ray& ray, double* const __restrict distance, bool* const __restrict inside) const noexcept
    {
        double u, v;
        const bool hit = ray.MöllerTrumboreIntersect(A, B, C, distance, &u, &v, inside);

        // TODO: do smth with u and v (?)

        return hit;
    }
};

struct MeshReference
{
    // todo : material assignment etc.
};

struct Scene
{
    std::vector<Triangle> Mesh;


    Scene() : Mesh() { }

    void add_triangle(const Triangle& triangle, OPTIONAL_EULER) noexcept
    {
        add_triangle(triangle.A, triangle.B, triangle.C, euler_angles);
    }

    void add_triangle(const Vec3& a, const Vec3& b, const Vec3& c, OPTIONAL_EULER) noexcept
    {
        const std::vector<double> mat = Vec3::create_rotation_matrix(euler_angles);

        Mesh.push_back(Triangle(a.transform(mat), b.transform(mat), c.transform(mat)));
    }

    void add_plane(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, OPTIONAL_EULER) noexcept
    {
        add_triangle(a, b, c, euler_angles);
        add_triangle(c, d, a, euler_angles);
    }

    void add_cube(const Vec3& center, const double& size_x, const double& size_y, const double& size_z, OPTIONAL_EULER) noexcept
    {
        const auto mat = Vec3::create_rotation_matrix(euler_angles);
        const Vec3 a = center.add(Vec3(size_x, size_y, size_z)).transform(mat);
        const Vec3 b = center.add(Vec3(size_x, size_y, -size_z)).transform(mat);
        const Vec3 c = center.add(Vec3(-size_x, size_y, -size_z)).transform(mat);
        const Vec3 d = center.add(Vec3(-size_x, size_y, size_z)).transform(mat);
        const Vec3 e = center.add(Vec3(size_x, -size_y, size_z)).transform(mat);
        const Vec3 f = center.add(Vec3(size_x, -size_y, -size_z)).transform(mat);
        const Vec3 g = center.add(Vec3(-size_x, -size_y, -size_z)).transform(mat);
        const Vec3 h = center.add(Vec3(-size_x, -size_y, size_z)).transform(mat);

        std::cout << a << b << c << d << e << f << g << h << std::endl;

        add_plane(a, b, c, d);
        add_plane(a, e, f, b);
        add_plane(b, f, g, c);
        add_plane(c, g, h, d);
        add_plane(d, h, e, a);
        add_plane(e, h, g, f);
    }

    void add_cube(const Vec3& center, const double& size, OPTIONAL_EULER) noexcept
    {
        add_cube(center, size, size, size, euler_angles);
    }
};
