#pragma once

#include "triangle.hpp"


enum RenderMode
{
    Colors,
    Depths,
    SurfaceNormals,
    Iterations,
    RenderTime,
};

struct CameraConfiguration
{
    Vec3 Position;
    Vec3 LookAt;
    double FOV;
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

struct Scene
{
    std::vector<Triangle> Geometry;
};


extern "C" __declspec(dllexport) void __cdecl RenderImage(RenderConfiguration const, ARGB* const);
extern "C" __declspec(dllexport) Ray __cdecl CreateRay(const CameraConfiguration&, const double, const double, const double, const double);
extern "C" __declspec(dllexport) RayTraceIteration __cdecl TraceRay(const RenderConfiguration&, const Scene&, RayTraceResult* const, const Ray&);
