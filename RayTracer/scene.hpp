#pragma once

#include "ray.hpp"


struct Scene;
struct MeshReference;


struct Triangle
{
    Material Mat;
    const Vec3 A, B, C;
    const double surface_area;
private:
    /// THIS IS NOT NORMALIZED !
    const Vec3 _normal;
public:


    Triangle() noexcept : Triangle(Vec3(), Vec3(), Vec3()) { }

    Triangle(const Vec3& a, const Vec3& b, const Vec3& c) noexcept
        : A(a)
        , B(b)
        , C(c)
        , _normal(b.sub(a).cross(c.sub(a)))
        , surface_area(b.sub(a).cross(c.sub(a)).length() / 2)
    {
    }

    inline Material get_material() const noexcept
    {
        return Mat;
    }

    inline void set_material(const Material& mat) noexcept
    {
        Mat = mat;
    }

    inline Vec3 normal_at(const Vec3& vec) const noexcept
    {
        return _normal;
    }

    inline bool intersect(const Ray& ray, double* const __restrict distance, bool* const __restrict inside, std::tuple<double, double>* const uv = nullptr) const noexcept
    {
        double u, v;
        const bool hit = ray.MöllerTrumboreIntersect(A, B, C, distance, &u, &v, inside);

        if (uv)
            *uv = std::make_tuple(u, v);

        return hit;
    }

    // why? because c++ is fucking retarded, that's why!
    inline Triangle& operator =(const Triangle& value)
    {
        return this == &value ? *this : *new(this)Triangle(value);
    }
};

struct MeshReference
{
    std::vector<int> _indices;
    Scene* _scene;


    MeshReference(Scene* scene, int index)
        : MeshReference(scene, std::vector<int>{ index })
    {
    }

    MeshReference(Scene* scene, std::vector<int> indices) noexcept
    {
        _scene = scene;
        _indices = indices;
    }

    MeshReference(Scene* scene, const std::vector<MeshReference>& references)
    {
        _scene = scene;
        _indices = std::vector<int>();

        for (const MeshReference& reference : references)
            for (int index : reference._indices)
                if (std::find(_indices.begin(), _indices.end(), index) == _indices.end())
                    _indices.push_back(index);
    }

    inline int triangle_count() const noexcept
    {
        return _indices.size();
    }

    inline std::vector<Triangle> mesh() const noexcept
    {
        std::vector<Triangle>* const mesh = reinterpret_cast<std::vector<Triangle>*>(_scene);
        std::vector<Triangle> triangles;

        if (mesh && mesh->size())
            for (const int index : _indices)
                if (index > 0 && index < mesh->size())
                    triangles.push_back(mesh->at(index));

        return triangles;
    }

    inline double surface_area() const
    {
        std::vector<Triangle>* const mesh = reinterpret_cast<std::vector<Triangle>*>(_scene);
        double area = 0;

        if (mesh && mesh->size())
            for (const int index : _indices)
                if (index > 0 && index < mesh->size())
                    area += mesh->at(index).surface_area;

        return area;
    }

    inline void set_material(const Material& mat) noexcept
    {
        std::vector<Triangle>* const mesh = reinterpret_cast<std::vector<Triangle>*>(_scene);

        if (mesh && mesh->size())
            for (const int index : _indices)
                if (index > 0 && index < mesh->size())
                    mesh->at(index).set_material(mat);
    }
};

struct Scene
{
    // static_assert(std::is_standard_layout_v<Scene>);
    std::vector<Triangle> mesh;


    inline MeshReference add_triangle(const Triangle& triangle, EULER_OPTARG) noexcept
    {
        return add_triangle(triangle.A, triangle.B, triangle.C, euler_angles);
    }

    inline MeshReference add_triangle(const Vec3& a, const Vec3& b, const Vec3& c, EULER_OPTARG) noexcept
    {
        const std::vector<double> mat = Vec3::create_rotation_matrix(euler_angles);
        Triangle triangle(a.transform(mat), b.transform(mat), c.transform(mat));

        mesh.push_back(triangle);

        return MeshReference(this, mesh.size() - 1);
    }

    inline MeshReference add_plane(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, EULER_OPTARG) noexcept
    {
        return MeshReference(this, std::vector<MeshReference>
        {
            add_triangle(a, b, c, euler_angles),
            add_triangle(c, d, a, euler_angles),
        });
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

    inline MeshReference add_icosahedron(const Vec3& center, const double& size, EULER_OPTARG) noexcept
    {
        constexpr double x = 0.525731112119133606;
        constexpr double z = 0.850650808352039932;
        const auto mat = Vec3::create_rotation_matrix(euler_angles);
        const Vec3 v0 = center.add(Vec3(-x,  0,  z).scale(size).transform(mat));
        const Vec3 v1 = center.add(Vec3( x,  0,  z).scale(size).transform(mat));
        const Vec3 v2 = center.add(Vec3(-x,  0, -z).scale(size).transform(mat));
        const Vec3 v3 = center.add(Vec3( x,  0, -z).scale(size).transform(mat));
        const Vec3 v4 = center.add(Vec3( 0,  z,  x).scale(size).transform(mat));
        const Vec3 v5 = center.add(Vec3( 0,  z, -x).scale(size).transform(mat));
        const Vec3 v6 = center.add(Vec3( 0, -z,  x).scale(size).transform(mat));
        const Vec3 v7 = center.add(Vec3( 0, -z, -x).scale(size).transform(mat));
        const Vec3 v8 = center.add(Vec3( z,  x,  0).scale(size).transform(mat));
        const Vec3 v9 = center.add(Vec3(-z,  x,  0).scale(size).transform(mat));
        const Vec3 v10 = center.add(Vec3( z, -x, 0).scale(size).transform(mat));
        const Vec3 v11 = center.add(Vec3(-z, -x, 0).scale(size).transform(mat));
        std::vector<MeshReference> references
        {
            add_triangle(v0, v4, v1),
            add_triangle(v0, v9, v4),
            add_triangle(v9, v5, v4),
            add_triangle(v4, v5, v8),
            add_triangle(v4, v8, v1),
            add_triangle(v8, v10, v1),
            add_triangle(v8, v3, v10),
            add_triangle(v5, v3, v8),
            add_triangle(v5, v2, v3),
            add_triangle(v2, v7, v3),
            add_triangle(v7, v10, v3),
            add_triangle(v7, v6, v10),
            add_triangle(v7, v11, v6),
            add_triangle(v11, v0, v6),
            add_triangle(v0, v1, v6),
            add_triangle(v6, v1, v10),
            add_triangle(v9, v0, v11),
            add_triangle(v9, v11, v2),
            add_triangle(v9, v2, v5),
            add_triangle(v7, v2, v11)
        };

        return MeshReference(this, references);
    }
    
    inline MeshReference subdivide(const int triangle_idx)
    {
        if (triangle_idx > 0 && triangle_idx < this->mesh.size())
            ; // TODO : return empty mesh

        //     A
        //    / \
        //   AB-AC
        //  / \ / \
        // B---BC--C

        Triangle* const triangle = &mesh[triangle_idx];
        const Vec3& A = triangle->A;
        const Vec3& B = triangle->B;
        const Vec3& C = triangle->C;
        const Vec3 mAB = A.add(B).scale(.5);
        const Vec3 mAC = A.add(C).scale(.5);
        const Vec3 mBC = B.add(C).scale(.5);

        std::vector<MeshReference> references
        {
            add_triangle(A, mAB, mAC),
            add_triangle(mAB, B, mBC),
            add_triangle(mAB, mBC, mAC),
            add_triangle(mAC, mBC, C),
        };

        return MeshReference(this, references);
    }

    inline MeshReference subdivide(MeshReference& object)
    {
        std::vector<MeshReference> references;

        for (const int index : object._indices)
            references.push_back(subdivide(index));

        return MeshReference(this, references);
    }

    inline MeshReference add_sphere(const Vec3& center, const double& radius, const unsigned int subdivison_level = 2) noexcept
    {
        MeshReference sphere = add_icosahedron(center, radius);

        for (unsigned int i = 0; i < subdivison_level; ++i)
            sphere = subdivide(sphere);

        for (const int index : sphere._indices)
            mesh[index] = Triangle(
                mesh[index].A.sub(center).normalize().scale(radius).add(center),
                mesh[index].B.sub(center).normalize().scale(radius).add(center),
                mesh[index].C.sub(center).normalize().scale(radius).add(center)
            );

        return sphere;
    }
};
