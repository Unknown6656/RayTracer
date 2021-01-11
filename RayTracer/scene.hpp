#pragma once

#include "ray.hpp"


struct Scene;
class MeshReference;


struct Triangle
{
    Material Mat;
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

    inline Material get_material() const noexcept
    {
        return Mat;
    }

    inline void set_material(Material mat) noexcept
    {
        Mat = mat;
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

class MeshReference
{
    std::vector<int> _indices;
    Scene* _scene;

public:
    MeshReference(Scene* scene, std::vector<int> indices) noexcept;
    MeshReference(Scene* scene, const std::vector<MeshReference>& references);

    inline int triangle_count() const noexcept;
    inline std::vector<Triangle> mesh() const;
    inline void set_material(Material mat);
};

struct Scene
{
    std::vector<Triangle> Mesh;


    inline int add_triangle(const Triangle& triangle, EULER_OPTARG) noexcept
    {
        return add_triangle(triangle.A, triangle.B, triangle.C, euler_angles);
    }

    inline int add_triangle(const Vec3& a, const Vec3& b, const Vec3& c, EULER_OPTARG) noexcept
    {
        const std::vector<double> mat = Vec3::create_rotation_matrix(euler_angles);
        Triangle triangle(a.transform(mat), b.transform(mat), c.transform(mat));

        Mesh.push_back(triangle);

        return Mesh.size() - 1;
    }

    inline MeshReference add_plane(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, EULER_OPTARG) noexcept
    {
        const int i1 = add_triangle(a, b, c, euler_angles);
        const int i2 = add_triangle(c, d, a, euler_angles);

        return MeshReference(this, std::vector<int> { i1, i2 });
    }

    inline MeshReference add_planeXY(const Vec3& pos, const double& size, EULER_OPTARG) noexcept
    {
        return add_planeXY(pos, size, size, euler_angles);
    }

    inline MeshReference add_planeXY(const Vec3& pos, const double& width, const double& height, EULER_OPTARG) noexcept
    {
        const auto mat = Vec3::create_rotation_matrix(euler_angles);
        const Vec3 x = Vec3(width / 2, 0, 0).transform(mat);
        const Vec3 y = Vec3(0, height / 2, 0).transform(mat);

        return add_plane(pos.sub(x).add(y), pos.add(x).add(y), pos.add(x).sub(y), pos.sub(x).sub(y));
    }

    inline MeshReference add_cube(const Vec3& center, const double& size, EULER_OPTARG) noexcept
    {
        return add_cube(center, size, size, size, euler_angles);
    }

    inline MeshReference add_cube(const Vec3& center, const double& size_x, const double& size_y, const double& size_z, EULER_OPTARG) noexcept
    {
        const auto mat = Vec3::create_rotation_matrix(euler_angles);
        const double x = size_x / 2, y = size_y / 2, z = size_z / 2;
        const Vec3 a = center.add(Vec3(x, y, z).transform(mat));
        const Vec3 b = center.add(Vec3(x, y, -z).transform(mat));
        const Vec3 c = center.add(Vec3(-x, y, -z).transform(mat));
        const Vec3 d = center.add(Vec3(-x, y, z).transform(mat));
        const Vec3 e = center.add(Vec3(x, -y, z).transform(mat));
        const Vec3 f = center.add(Vec3(x, -y, -z).transform(mat));
        const Vec3 g = center.add(Vec3(-x, -y, -z).transform(mat));
        const Vec3 h = center.add(Vec3(-x, -y, z).transform(mat));
        std::vector<MeshReference> references
        {
            add_plane(a, b, c, d),
            add_plane(a, e, f, b),
            add_plane(b, f, g, c),
            add_plane(c, g, h, d),
            add_plane(d, h, e, a),
            add_plane(e, h, g, f),
        };

        return MeshReference(this, references);
    }
};

