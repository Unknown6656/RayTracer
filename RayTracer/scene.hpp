#pragma once

#include "primitive.hpp"


struct Scene;


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

    inline int shape_count() const noexcept
    {
        return _indices.size();
    }

    inline std::vector<Primitive> mesh() const noexcept
    {
        std::vector<Primitive>* const mesh = reinterpret_cast<std::vector<Primitive>*>(_scene);
        std::vector<Primitive> shapes;

        if (mesh && mesh->size())
            for (const int index : _indices)
                if (index > 0 && index < mesh->size())
                    shapes.push_back(mesh->at(index));

        return shapes;
    }

    inline float surface_area() const
    {
        std::vector<Primitive>* const mesh = reinterpret_cast<std::vector<Primitive>*>(_scene);
        float area = 0;

        if (mesh && mesh->size())
            for (const int index : _indices)
                if (index > 0 && index < mesh->size())
                    area += mesh->at(index).surface_area();

        return area;
    }

    inline void set_material(const Material& mat) noexcept
    {
        std::vector<Primitive>* const mesh = reinterpret_cast<std::vector<Primitive>*>(_scene);

        if (mesh && mesh->size())
            for (const int index : _indices)
                if (index > 0 && index < mesh->size())
                    mesh->at(index).set_material(mat);
    }

    inline static MeshReference Empty(Scene* scene) noexcept
    {
        return MeshReference(scene, std::vector<int>());
    }
};

struct Light
{
    enum class LightMode
    {
        Spot,
        Parallel,
        Global,
    };

    const LightMode mode;
    const vec3 position;
    const vec3 direction;
    const ARGB diffuse_color;
    const ARGB specular_color;
    const float diffuse_intensity;
    const float specular_intensity;
    const float opening_angle;
    const float falloff_exponent;


    Light() : Light(ARGB::WHITE, ARGB::WHITE, vec3(0, 10, 0), vec3(0, -1, 0), 1, 1, DEG2RAD(20), 3, LightMode::Spot) { }

    Light(const ARGB& diffuse, const ARGB& specular, const vec3& pos, const vec3& dir, const float diffuse_power, const float specular_power, const float opening_angle, const float falloff_exponent, const LightMode mode)
        : diffuse_color(diffuse)
        , specular_color(specular)
        , position(pos)
        , direction(dir.normalize())
        , opening_angle(opening_angle)
        , falloff_exponent(falloff_exponent)
        , diffuse_intensity(diffuse_power)
        , specular_intensity(specular_power)
        , mode(mode)
    {
    }

    inline std::string to_string() const noexcept
    {
        std::stringstream ss;
        ss << '[P=' << position << ",D=" << direction << ",O=" << opening_angle << ",F=" << falloff_exponent << ",DIFF=" << diffuse_intensity;
        ss << diffuse_color << ",SPEC=" << specular_intensity << specular_color << ",M=" << (mode == LightMode::Parallel ? "par" : mode == LightMode::Global ? "glob" : "spot") << "]";

        return ss.str();
    }

    OSTREAM_OPERATOR(Light);
    CPP_IS_FUCKING_RETARDED(Light);
};

struct Scene
{
    // static_assert(std::is_standard_layout_v<Scene>);

    // DO NOT CHANGE THE LAYOUT OF THIS STRUCT. IT WILL BREAK OTHERWISE !
    std::vector<Primitive> mesh;
    std::vector<Light> lights;
    ARGB background_color;


    Scene() noexcept
        : mesh(std::vector<Primitive>())
        , lights(std::vector<Light>())
        , background_color(ARGB::BLACK)
    {
    }

    inline MeshReference add_triangle(const vec3& a, const vec3& b, const vec3& c, EULER_OPTARG) noexcept
    {
        const std::vector<float> mat = vec3::create_rotation_matrix(euler_angles);
        const Primitive triangle(a.transform(mat), b.transform(mat), c.transform(mat));

        return add_shape(triangle);
    }

    inline MeshReference add_shape(const Primitive& shape) noexcept
    {
        mesh.push_back(shape);

        return MeshReference(this, mesh.size() - 1);
    }

    inline MeshReference add_plane(const vec3& a, const vec3& b, const vec3& c, const vec3& d, EULER_OPTARG) noexcept
    {
        return MeshReference(this, std::vector<MeshReference>
        {
            add_triangle(a, b, c, euler_angles),
            add_triangle(c, d, a, euler_angles),
        });
    }

    inline MeshReference add_planeXY(const vec3& pos, const float& size, EULER_OPTARG) noexcept
    {
        return add_planeXY(pos, size, size, euler_angles);
    }

    inline MeshReference add_planeXY(const vec3& pos, const float& width, const float& height, EULER_OPTARG) noexcept
    {
        const auto mat = vec3::create_rotation_matrix(euler_angles);
        const vec3 x = vec3(width / 2, 0, 0).transform(mat);
        const vec3 y = vec3(0, height / 2, 0).transform(mat);

        return add_plane(pos.sub(x).add(y), pos.add(x).add(y), pos.add(x).sub(y), pos.sub(x).sub(y));
    }

    inline MeshReference add_cube(const vec3& center, const float& size, EULER_OPTARG) noexcept
    {
        return add_cube(center, size, size, size, euler_angles);
    }

    inline MeshReference add_cube(const vec3& center, const float& size_x, const float& size_y, const float& size_z, EULER_OPTARG) noexcept
    {
        const auto mat = vec3::create_rotation_matrix(euler_angles);
        const float x = size_x / 2, y = size_y / 2, z = size_z / 2;
        const vec3 a = center.add(vec3(x, y, z).transform(mat));
        const vec3 b = center.add(vec3(x, y, -z).transform(mat));
        const vec3 c = center.add(vec3(-x, y, -z).transform(mat));
        const vec3 d = center.add(vec3(-x, y, z).transform(mat));
        const vec3 e = center.add(vec3(x, -y, z).transform(mat));
        const vec3 f = center.add(vec3(x, -y, -z).transform(mat));
        const vec3 g = center.add(vec3(-x, -y, -z).transform(mat));
        const vec3 h = center.add(vec3(-x, -y, z).transform(mat));
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

    inline MeshReference add_icosahedron(const vec3& center, const float& size, EULER_OPTARG) noexcept
    {
        constexpr float x = 0.525731112119133606;
        constexpr float z = 0.850650808352039932;
        const auto mat = vec3::create_rotation_matrix(euler_angles);
        const vec3 v0 = center.add(vec3(-x,  0,  z).scale(size).transform(mat));
        const vec3 v1 = center.add(vec3( x,  0,  z).scale(size).transform(mat));
        const vec3 v2 = center.add(vec3(-x,  0, -z).scale(size).transform(mat));
        const vec3 v3 = center.add(vec3( x,  0, -z).scale(size).transform(mat));
        const vec3 v4 = center.add(vec3( 0,  z,  x).scale(size).transform(mat));
        const vec3 v5 = center.add(vec3( 0,  z, -x).scale(size).transform(mat));
        const vec3 v6 = center.add(vec3( 0, -z,  x).scale(size).transform(mat));
        const vec3 v7 = center.add(vec3( 0, -z, -x).scale(size).transform(mat));
        const vec3 v8 = center.add(vec3( z,  x,  0).scale(size).transform(mat));
        const vec3 v9 = center.add(vec3(-z,  x,  0).scale(size).transform(mat));
        const vec3 v10 = center.add(vec3( z, -x, 0).scale(size).transform(mat));
        const vec3 v11 = center.add(vec3(-z, -x, 0).scale(size).transform(mat));
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
            return MeshReference::Empty(this);

        //     A
        //    / \
        //   AB-AC
        //  / \ / \
        // B---BC--C

        const TriangleData& triangle = mesh[triangle_idx].data.triangle;
        const vec3& A = triangle.A;
        const vec3& B = triangle.B;
        const vec3& C = triangle.C;
        const vec3 mAB = A.add(B).scale(.5);
        const vec3 mAC = A.add(C).scale(.5);
        const vec3 mBC = B.add(C).scale(.5);

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

    inline MeshReference add_triangularized_sphere(const vec3& center, const float& radius, const unsigned int subdivison_level = 2) noexcept
    {
        MeshReference sphere = add_icosahedron(center, radius);

        for (unsigned int i = 0; i < subdivison_level; ++i)
            sphere = subdivide(sphere);

        for (const int index : sphere._indices)
        {
            const TriangleData& triangle = mesh[index].data.triangle;

            mesh[index] = Primitive(
                triangle.A.sub(center).normalize().scale(radius).add(center),
                triangle.B.sub(center).normalize().scale(radius).add(center),
                triangle.C.sub(center).normalize().scale(radius).add(center)
            );
        }

        return sphere;
    }

    inline MeshReference add_sphere(const vec3& center, const float& radius) noexcept
    {
        return add_shape(Primitive(center, radius));
    }

    inline const Light& add_light(const Light& light) noexcept
    {
        lights.push_back(light);

        return light;
    }

    inline const Light& add_spot_light(const vec3& position, const vec3& direction, const ARGB& color, const float intensity = 1, const float opening_angle = DEG2RAD(20), const float falloff_exp = 3)
    {
        const Light light(color, color, position, direction, intensity, intensity, opening_angle, falloff_exp, Light::LightMode::Spot);

        return add_light(light);
    }

    inline const Light& add_global_light(const ARGB& color, const float intensity = 1) noexcept
    {
        const Light light(color, color, vec3::Zero, vec3::UnitX, intensity, intensity, 0, 0, Light::LightMode::Spot);

        return add_light(light);
    }

    inline const Light& add_parallel_light(const vec3& direction, const ARGB& color, const float intensity = 1) noexcept
    {
        const Light light(color, color, vec3::Zero, direction, intensity, intensity, 0, 0, Light::LightMode::Parallel);

        return add_light(light);
    }
};
