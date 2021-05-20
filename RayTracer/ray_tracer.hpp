#pragma once

#include "scene.hpp"


enum RenderMode
{
    Colors,
    Wireframe,
    UVCoords,
    Depths,
    SurfaceNormals,
    RayIncidenceAngle,
    RayDirection,
    Iterations,
    RenderTime,
};

struct CameraConfiguration
{
    vec3 Position;
    vec3 LookAt;
    float ZoomFactor;
    float FocalLength;
};

struct RenderConfiguration
{
    size_t HorizontalResolution;
    size_t VerticalResolution;
    size_t SubpixelsPerPixel;
    size_t SamplesPerSubpixel;
    size_t MaximumIterationCount;
    CameraConfiguration Camera;
    RenderMode RenderMode;
};


extern "C" __declspec(dllexport) void __cdecl CreateScene(Scene** const);
extern "C" __declspec(dllexport) void __cdecl DeleteScene(Scene** const);
extern "C" __declspec(dllexport) void __cdecl RenderImage(const Scene* const, RenderConfiguration const, ARGB* const, float* const = nullptr);
extern "C" __declspec(dllexport) void __cdecl ComputeRenderPass(const Scene* const, const RenderConfiguration&, const int, const int, ARGB* const&);
extern "C" __declspec(dllexport) inline ray __cdecl CreateRay(const CameraConfiguration&, const float, const float, const float, const float);
extern "C" __declspec(dllexport) inline RayTraceIteration __cdecl TraceRay(const Scene* const, const RenderConfiguration&, RayTraceResult* const, const ray&);
extern "C" __declspec(dllexport) inline void __cdecl ComputeColor(const Scene* const, const RenderConfiguration&, RayTraceIteration* const);
