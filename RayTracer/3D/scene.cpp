#include "scene.hpp"


MeshReference::MeshReference(Scene* scene, std::vector<int> indices) noexcept
{
    _scene = scene;
    _indices = indices;
}

MeshReference::MeshReference(Scene* scene, const std::vector<MeshReference>& references)
{
    _scene = scene;
    _indices = std::vector<int>();

    for (const MeshReference& reference : references)
        for (int index : reference._indices)
            if (std::find(_indices.begin(), _indices.end(), index) == _indices.end())
                _indices.push_back(index);
}

inline std::vector<Triangle> MeshReference::mesh() const
{
    std::vector<Triangle> triangles;

    for (const int index : _indices)
        triangles.push_back(_scene->Mesh[index]);

    return triangles;
}

