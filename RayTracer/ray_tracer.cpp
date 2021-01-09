#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <ctime>
#include <ppl.h>

#include "ray_tracer.hpp"

#define INIT_RAND std::srand(std::time(nullptr))
#define DOUBLE_RAND (double(std::rand()) / RAND_MAX)


void RenderImage(RenderConfiguration const config, ARGB* const buffer)
{
    assert(buffer != nullptr);

    const int w = config.HorizontalResolution;
    const int h = config.VerticalResolution;
    const int sub = config.SubpixelsPerPixel;
    const double subd(sub);
    const size_t total_samples = double(w) * h * sub * sub * config.SamplesPerSubpixel;

    std::cout << std::endl << "----------------------------------------------------------------" << std::endl
        << "Resolution(in pixels) : " << w << "x" << h << std::endl
        << "  Subpixels per pixel : " << sub << "x" << sub << std::endl
        << " Samples per subpixel : " << config.SamplesPerSubpixel << std::endl
        << "    Maximum ray depth : " << config.MaximumIterationCount << std::endl
        << "    Minimum ray count : " << total_samples << std::endl
        << "    Maximum ray count : " << total_samples * config.MaximumIterationCount << std::endl
        << "----------------------------------------------------------------" << std::endl;

    INIT_RAND;

    auto total_timer = std::chrono::high_resolution_clock::now();



    Scene scene;
    scene.Geometry = std::vector<Triangle>();
    scene.Geometry.push_back(Triangle(
        Vec3(-30, 0, 0),
        Vec3(0, 0, 0),
        Vec3(0, 30, 0)
    ));



    for (int sample = 0; sample < config.SamplesPerSubpixel; ++sample)
        concurrency::parallel_for(
            size_t(0),
            size_t(w) * size_t(h),
            [&](size_t index)
            {
                const size_t raw_x = index % w;
                const size_t raw_y = index / w;
                const double pixel_x = raw_x / double(w)* 2.0 - 1.0;
                const double pixel_y = 1.0 - raw_y / double(h) * 2.0;
                const double norm_factor = 1.0 / (subd * subd);
                ARGB total;

                for (int sx = 0; sx < sub; ++sx)
                {
                    double x = double((sx + 0.5) / subd) / w + pixel_x;

                    for (int sy = 0; sy < sub; ++sy)
                    {
                        double y = double((sy + 0.5) / subd) / w + pixel_y;
                        auto start = std::chrono::high_resolution_clock::now();

                        const Ray ray = CreateRay(config.Camera, w, h, x, y);
                        RayTraceResult result(config.MaximumIterationCount);

                        TraceRay(config, scene, &result, ray);

                        const std::chrono::nanoseconds elapsed = std::chrono::high_resolution_clock::now() - start;
                        ARGB color(0);

                        switch (config.RenderMode)
                        {
                            case RenderMode::SurfaceNormals:
                                color = result[0].SurfaceNormal;

                                break;
                            case RenderMode::RayDirection:
                                color = ray.Direction.add(Vec3(1)).scale(.5);

                                break;
                            case RenderMode::Iterations:
                                color = ARGB(1.0 - result.size() / double(config.MaximumIterationCount));

                                break;
                            case RenderMode::RenderTime:
                            {
                                const double µs = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

                                color = ARGB(std::log10(µs) * 0.30293575075);
                            } break;
                            case RenderMode::Depths:
                                // TODO 

                                break;
                            case RenderMode::Colors:
                            default:
                                color = result[0].ComputedColor;

                                break;
                        }

                        total = total + color * norm_factor;
                    }
                }

                buffer[index] = buffer[index] + total / double(config.SamplesPerSubpixel);
            }
        );

    const auto elapsed = std::chrono::high_resolution_clock::now() - total_timer;
    const double µs = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

    printf("Render time: %fs\n", µs * 1e-6);
}

Ray CreateRay(const CameraConfiguration& camera, const double w, const double h, const double x, const double y)
{
    const double fov = 1.57079632679 / camera.ZoomFactor;
    const Vec3 gaze = camera.LookAt.sub(camera.Position).normalize();
    const Vec3 up = Vec3::UnitY;
    const Vec3 camx = gaze.cross(up).normalize().scale(w * fov / h);
    const Vec3 camy = camx.cross(gaze).normalize().scale(fov);
    const double dx = 2 * DOUBLE_RAND / w;
    const double dy = 2 * DOUBLE_RAND / h;
    const Vec3 dir = gaze.add(camx.scale(x + dx))
                         .add(camy.scale(y + dy))
                         .normalize();

    Ray r(camera.Position + dir.scale(camera.FocalLength), dir, 0);

    return r;
}

RayTraceIteration TraceRay(const RenderConfiguration& config, const Scene& scene, RayTraceResult* const result, const Ray& ray)
{
    if (ray.IterationDepth < config.MaximumIterationCount)
    {
        result->push_back(RayTraceIteration());

        RayTraceIteration* iteration = &result->back();
        bool intersection = false;
        double distance;
        bool inside;

        for (const Triangle& geometry : scene.Geometry)
            if (intersection |= geometry.intersect(ray, &distance, &inside))
            {
                iteration->SurfaceNormal = geometry.normal_at(ray.evaluate(distance));

                break;
            }

        iteration->Distance = distance;

        if (!intersection)
            iteration->ComputedColor = ARGB(0, 0); // TODO : handle environment colors?
        else
        {
            Vec3 direction; // TODO



            RayTraceIteration i1 = TraceRay(config, scene, result, ray.create_next(distance, direction));


            // TODO
            iteration->ComputedColor = ARGB(1.0 / ray.IterationDepth);
        }

        return *iteration;
    }

    return RayTraceIteration();
}
