#include "scene.hpp"

using namespace ray_tracer_3d;


int ray_tracer_3d::mesh_reference::shape_count() const noexcept
{
    return _indices.size();
}

std::vector<primitive*> ray_tracer_3d::mesh_reference::mesh() const noexcept
{
    std::vector<primitive*> const mesh = _scene->mesh;
    std::vector<primitive*> shapes;

    for (const int index : _indices)
        if (index > 0 && index < mesh.size())
            shapes.push_back(mesh[index]);

    return shapes;
}

float ray_tracer_3d::mesh_reference::surface_area() const
{
    std::vector<primitive*> const mesh = _scene->mesh;
    float area = 0;

    for (const int index : _indices)
        if (index > 0 && index < mesh.size())
            area += mesh[index]->surface_area();

    return area;
}

void ray_tracer_3d::mesh_reference::set_material(const material& mat) noexcept
{
    std::vector<primitive*> const mesh = _scene->mesh;

    for (const int index : _indices)
        if (index > 0 && index < mesh.size())
            mesh[index]->material = mat;
}

