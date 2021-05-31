#pragma once

#include "scene.hpp"


namespace ray_tracer_3d
{
    enum render_mode
    {
        realistic_colors,
        diffuse_colors,
        // TODO
        wireframe,
        uv_coords,
        hit_type,
        depths,
        surface_normals,
        ray_incidence_angle,
        ray_direction,
        iterations,
        render_time,
    };

    struct camera_configuration
    {
        vec3 position;
        vec3 look_at;
        float zoom_factor;
        float focal_length;
    };

    struct render_configuration
    {
        size_t horizontal_resolution;
        size_t vertical_resolution;
        size_t subpixels_per_pixel;
        size_t samples_per_subpixel;
        size_t maximum_iteration_count;
        camera_configuration camera;
        render_mode mode;
        bool debug;
        ARGB background_color;
        float air_refraction_index;
    };

    struct ray_trace_iteration
    {
        ray3 ray;
        hit_test hit;
        ARGB computed_color;
        vec3 surface_normal;
        vec3 intersection_point;
        primitive* primitive;


        TO_STRING(ray_trace_iteration, "R=" << ray
                                    << ",H=" << hit
                                    << ",N=" << surface_normal
                                    << ",C=" << computed_color
                                    << ",I=" << intersection_point
                                    << ",P=" << (primitive ? primitive->to_string() : "[null]"));
    };

    typedef std::vector<ray_trace_iteration> ray_trace_result;


    extern "C" __declspec(dllexport) scene* __cdecl CreateScene3();
    extern "C" __declspec(dllexport) void __cdecl DeleteScene3(scene* const);
    extern "C" __declspec(dllexport) float __cdecl RenderImage3(const scene* const __restrict, render_configuration const, ARGB* const __restrict, float* const __restrict = nullptr);
    extern "C" __declspec(dllexport) void __cdecl ComputeRenderPass3(const scene* const, const render_configuration&, const int, const int, ARGB* const&, const bool = true);
    extern "C" __declspec(dllexport) inline ray3 __cdecl CreateRay3(const render_configuration&, const float, const float, const float, const float);
    extern "C" __declspec(dllexport) inline ray_trace_iteration __cdecl TraceRay3(const scene* const __restrict, const render_configuration&, ray_trace_result* const __restrict, const ray3&);
    extern "C" __declspec(dllexport) inline void __cdecl ComputeColor3(const scene* const __restrict, const render_configuration&, ray_trace_iteration* const __restrict);
};