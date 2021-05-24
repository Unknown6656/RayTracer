#pragma once

#include "primitive3.hpp"


namespace ray_tracer_3d
{
    struct scene;

    struct light
    {
        enum class light_mode
        {
            Spot,
            Parallel,
            Global,
        };

        const light_mode mode;
        const vec3 position;
        const vec3 direction;
        const ARGB diffuse_color;
        const ARGB specular_color;
        const float diffuse_intensity;
        const float specular_intensity;
        const float opening_angle;
        const float falloff_exponent;


        light()
            : light(ARGB::WHITE, ARGB::WHITE, vec3(0, 10, 0), vec3(0, -1, 0), 1, 1, DEG2RAD(20), 3, light_mode::Spot)
        {
        }

        light(const ARGB& diffuse, const ARGB& specular, const vec3& pos, const vec3& dir, const float diffuse_power, const float specular_power, const float opening_angle, const float falloff_exponent, const light_mode mode)
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

        TO_STRING(light, "P=" << position
                      << ",D=" << direction
                      << ",O=" << opening_angle
                      << ",F=" << falloff_exponent
                      << ",DIFF_I=" << diffuse_intensity << diffuse_color
                      << ",SPEC=" << specular_intensity << specular_color
                      << ",M=" << (mode == light_mode::Parallel ? "par" : mode == light_mode::Global ? "glob" : "spot"));
        CPP_IS_FUCKING_RETARDED(light);
    };

    struct mesh_reference
    {
        std::vector<int> _indices;
        scene* _scene;


        mesh_reference(scene* scene, int index) noexcept
            : mesh_reference(scene, std::vector<int>{ index })
        {
        }

        mesh_reference(scene* scene, std::vector<int> indices) noexcept
        {
            _scene = scene;
            _indices = indices;
        }

        mesh_reference(scene* scene, const std::vector<mesh_reference>& references)
        {
            _scene = scene;
            _indices = std::vector<int>();

            for (const mesh_reference& reference : references)
                for (int index : reference._indices)
                    if (std::find(_indices.begin(), _indices.end(), index) == _indices.end())
                        _indices.push_back(index);
        }

        int shape_count() const noexcept;

        std::vector<primitive*> mesh() const noexcept;

        float surface_area() const;

        void set_material(const material& mat) noexcept;

        inline static mesh_reference empty(scene* scene) noexcept
        {
            return mesh_reference(scene, std::vector<int>());
        }
    };

    struct scene
    {
        // static_assert(std::is_standard_layout_v<Scene>);

        std::vector<primitive*> mesh;
        std::vector<light> lights;


        scene() noexcept
            : mesh(std::vector<primitive*>())
            , lights(std::vector<light>())
        {
        }

        inline mesh_reference add_triangularized_sphere(const vec3& center, const float& radius, const unsigned int subdivison_level = 2) noexcept
        {
            mesh_reference sphere = add_icosahedron(center, radius);

            for (unsigned int i = 0; i < subdivison_level; ++i)
                sphere = subdivide(sphere);

            for (const int index : sphere._indices)
            {
                const triangle* tri = reinterpret_cast<triangle*>(&mesh[index]);

                mesh[index] = new triangle(
                    tri->A.sub(center).normalize().scale(radius).add(center),
                    tri->B.sub(center).normalize().scale(radius).add(center),
                    tri->C.sub(center).normalize().scale(radius).add(center)
                );
            }

            return sphere;
        }

        inline mesh_reference add_sphere(const vec3& center, const float& radius) noexcept
        {
            return add_shape(new sphere(center, radius));
        }

        inline mesh_reference add_triangle(const vec3& a, const vec3& b, const vec3& c, EULER_OPTARG) noexcept
        {
            const std::vector<float> mat = vec3::create_rotation_matrix(euler_angles);

            return add_shape(new triangle(a.transform(mat), b.transform(mat), c.transform(mat)));
        }

        inline mesh_reference add_plane(const vec3& a, const vec3& b, const vec3& c, const vec3& d, EULER_OPTARG) noexcept
        {
            return mesh_reference(this, std::vector<mesh_reference>
            {
                add_triangle(a, b, c, euler_angles),
                    add_triangle(c, d, a, euler_angles),
            });
        }

        inline mesh_reference add_planeXY(const vec3& pos, const float& size, EULER_OPTARG) noexcept
        {
            return add_planeXY(pos, size, size, euler_angles);
        }

        inline mesh_reference add_planeXY(const vec3& pos, const float& width, const float& height, EULER_OPTARG) noexcept
        {
            const auto mat = vec3::create_rotation_matrix(euler_angles);
            const vec3 x = vec3(width / 2, 0, 0).transform(mat);
            const vec3 y = vec3(0, height / 2, 0).transform(mat);

            return add_plane(pos.sub(x).add(y), pos.add(x).add(y), pos.add(x).sub(y), pos.sub(x).sub(y));
        }

        inline mesh_reference add_cube(const vec3& center, const float& size, EULER_OPTARG) noexcept
        {
            return add_cube(center, size, size, size, euler_angles);
        }

        inline mesh_reference add_cube(const vec3& center, const float& size_x, const float& size_y, const float& size_z, EULER_OPTARG) noexcept
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
            std::vector<mesh_reference> references
            {
                add_plane(a, b, c, d),
                add_plane(a, e, f, b),
                add_plane(b, f, g, c),
                add_plane(c, g, h, d),
                add_plane(d, h, e, a),
                add_plane(e, h, g, f),
            };

            return mesh_reference(this, references);
        }

        inline mesh_reference add_icosahedron(const vec3& center, const float& size, EULER_OPTARG) noexcept
        {
            constexpr float x = .525731112119133606;
            constexpr float z = .850650808352039932;
            const auto mat = vec3::create_rotation_matrix(euler_angles);
            const vec3 v0 = center.add(vec3(-x, 0, z).scale(size).transform(mat));
            const vec3 v1 = center.add(vec3(x, 0, z).scale(size).transform(mat));
            const vec3 v2 = center.add(vec3(-x, 0, -z).scale(size).transform(mat));
            const vec3 v3 = center.add(vec3(x, 0, -z).scale(size).transform(mat));
            const vec3 v4 = center.add(vec3(0, z, x).scale(size).transform(mat));
            const vec3 v5 = center.add(vec3(0, z, -x).scale(size).transform(mat));
            const vec3 v6 = center.add(vec3(0, -z, x).scale(size).transform(mat));
            const vec3 v7 = center.add(vec3(0, -z, -x).scale(size).transform(mat));
            const vec3 v8 = center.add(vec3(z, x, 0).scale(size).transform(mat));
            const vec3 v9 = center.add(vec3(-z, x, 0).scale(size).transform(mat));
            const vec3 v10 = center.add(vec3(z, -x, 0).scale(size).transform(mat));
            const vec3 v11 = center.add(vec3(-z, -x, 0).scale(size).transform(mat));
            std::vector<mesh_reference> references
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

            return mesh_reference(this, references);
        }

        inline mesh_reference add_shape(primitive* const shape) noexcept
        {
            mesh.push_back(shape);

            return mesh_reference(this, mesh.size() - 1);
        }

        inline mesh_reference subdivide(const int triangle_idx) noexcept
        {
            if (triangle_idx > 0 && triangle_idx < this->mesh.size())
                return mesh_reference::empty(this);

            //     A
            //    / \
            //   AB-AC
            //  / \ / \
            // B---BC--C

            const triangle* tri = reinterpret_cast<triangle*>(&mesh[triangle_idx]);
            const vec3 mAB = tri->A.add(tri->B).scale(.5);
            const vec3 mAC = tri->A.add(tri->C).scale(.5);
            const vec3 mBC = tri->B.add(tri->C).scale(.5);
            const std::vector<mesh_reference> references
            {
                add_triangle(tri->A, mAB,    mAC),
                add_triangle(mAB,    tri->B, mBC),
                add_triangle(mAB,    mBC,    mAC),
                add_triangle(mAC,    mBC,    tri->C),
            };

            return mesh_reference(this, references);
        }

        inline mesh_reference subdivide(mesh_reference& object)
        {
            std::vector<mesh_reference> references;

            for (const int index : object._indices)
                references.push_back(subdivide(index));

            return mesh_reference(this, references);
        }

        inline const light& add_light(const light& light) noexcept
        {
            lights.push_back(light);

            return light;
        }

        inline const light& add_spot_light(const vec3& position, const vec3& direction, const ARGB& color, const float intensity = 1, const float opening_angle = DEG2RAD(20), const float falloff_exp = 3) noexcept
        {
            const light light(color, color, position, direction, intensity, intensity, opening_angle, falloff_exp, light::light_mode::Spot);

            return add_light(light);
        }

        inline const light& add_global_light(const ARGB& color, const float intensity = 1) noexcept
        {
            const light light(color, color, vec3::Zero, vec3::UnitX, intensity, intensity, 0, 0, light::light_mode::Spot);

            return add_light(light);
        }

        inline const light& add_parallel_light(const vec3& direction, const ARGB& color, const float intensity = 1) noexcept
        {
            const light light(color, color, vec3::Zero, direction, intensity, intensity, 0, 0, light::light_mode::Parallel);

            return add_light(light);
        }
    };
};
