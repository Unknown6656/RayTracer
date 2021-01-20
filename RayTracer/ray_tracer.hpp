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
    Vec3 Position;
    Vec3 LookAt;
    double ZoomFactor;
    double FocalLength;
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
extern "C" __declspec(dllexport) void __cdecl RenderImage(const Scene* const, RenderConfiguration const, ARGB* const, double* const = nullptr);
extern "C" __declspec(dllexport) void __cdecl ComputeRenderPass(const Scene* const, const RenderConfiguration&, const int, const int, ARGB* const&);
extern "C" __declspec(dllexport) inline Ray __cdecl CreateRay(const CameraConfiguration&, const double, const double, const double, const double);
extern "C" __declspec(dllexport) inline RayTraceIteration __cdecl TraceRay(const Scene* const, const RenderConfiguration&, RayTraceResult* const, const Ray&);
extern "C" __declspec(dllexport) inline void __cdecl ComputeColor(const Scene* const, const RenderConfiguration&, RayTraceIteration* const);
