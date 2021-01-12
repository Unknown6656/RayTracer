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


void CreateScene(Scene* const scene)
{
    if (scene)
    {
        *scene = Scene();

        MeshReference cube = scene->add_cube(Vec3(-4, 3, 4), 2);
        MeshReference back = scene->add_planeXY(Vec3(0, 10, -10), 20);
        MeshReference left = scene->add_planeXY(Vec3(-10, 10, 0), 20, Vec3(0, ROT_90, 0));
        MeshReference floor = scene->add_planeXY(Vec3::Zero, 20, Vec3(ROT_90, 0, 0));
        MeshReference ball = scene->add_sphere(Vec3(0, 2, 0), 1.7);

        cube.set_material(Material::diffuse(ARGB::RED));
        back.set_material(Material::diffuse(ARGB::BLUE));
        left.set_material(Material::emissive(ARGB::GREEN, 0.2));
        floor.set_material(Material::reflective(ARGB::BLACK, 1));
        ball.set_material(Material::reflective(ARGB(.5, .7, .84), .8));

        // TODO
    }
}

void RenderImage(const Scene* const scene, RenderConfiguration const config, ARGB* const buffer, double* const progress)
{
    assert(buffer != nullptr);

    std::cout << "sizeof(Scene) = " << sizeof(Scene) << std::endl;

    INIT_RAND;

    const int w = config.HorizontalResolution;
    const int h = config.VerticalResolution;
    const int sub = config.SubpixelsPerPixel;
    const size_t total_samples = double(w) * h * sub * sub * config.SamplesPerSubpixel;

    std::cout << std::endl << "----------------------------------------------------------------" << std::endl
        << "Resolution(in pixels) : " << w << "x" << h << std::endl
        << "  Subpixels per pixel : " << sub << "x" << sub << std::endl
        << " Samples per subpixel : " << config.SamplesPerSubpixel << std::endl
        << "    Maximum ray depth : " << config.MaximumIterationCount << std::endl
        << "    Minimum ray count : " << total_samples << std::endl
        << "    Maximum ray count : " << total_samples * config.MaximumIterationCount << std::endl
        << "       Triangle count : " << scene->mesh.size() << std::endl
        << "          Render mode : " << config.RenderMode << std::endl
        << "----------------------------------------------------------------" << std::endl;

    auto total_timer = std::chrono::high_resolution_clock::now();
    constexpr int CUBE_SZ = 128;
    std::atomic<size_t> pass_counter(size_t(0));

    if (progress)
        *progress = 0;

    for (int sample = 0; sample < config.SamplesPerSubpixel; ++sample)
        for (int base_y = 0; base_y < h; base_y += CUBE_SZ)
            for (int base_x = 0; base_x < w; base_x += CUBE_SZ)
            {
                const int block_w = std::min(w - base_x, CUBE_SZ);
                const int block_h = std::min(h - base_y, CUBE_SZ);

                concurrency::parallel_for(
                    size_t(0),
                    size_t(block_w) * size_t(block_h),
                    [&](size_t index)
                    {
                        const int pixel_x = base_x + index % block_w;
                        const int pixel_y = base_y + index / block_w;

                        ComputeRenderPass(scene, config, pixel_x, pixel_y, buffer);

                        if (progress)
                            *progress = double(++pass_counter) / (double(w) * h * config.SamplesPerSubpixel);
                    }
                );
            }

    const auto elapsed = std::chrono::high_resolution_clock::now() - total_timer;
    const double µs = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

    printf("Render time: %fs\n", µs * 1e-6);
}

void ComputeRenderPass(const Scene* const scene, const RenderConfiguration& config, const int raw_x, const int raw_y, ARGB* const& buffer)
{
    const int w = config.HorizontalResolution;
    const int h = config.VerticalResolution;
    const int sub = config.SubpixelsPerPixel;
    const double subd(sub);
    const double pixel_x = raw_x / double(w) * 2.0 - 1.0;
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

            RayTraceResult result;
            const Ray ray = CreateRay(config.Camera, w, h, x, y);
            const RayTraceIteration iteration = TraceRay(scene, config, &result, ray);

            const std::chrono::nanoseconds elapsed = std::chrono::high_resolution_clock::now() - start;
            ARGB color(0);

            switch (config.RenderMode)
            {
                case RenderMode::Colors:
                default_mode:
                    color = iteration.ComputedColor;

                    break;
                case RenderMode::Depths:
                    if (iteration.Hit)
                        color = ARGB(1.0 / (1 + 0.1 * iteration.Distance));

                    break;
                case RenderMode::Wireframe:
                    if (iteration.Hit)
                    {
                        const double u = std::get<0>(iteration.UVCoordinates);
                        const double v = std::get<1>(iteration.UVCoordinates);
                        const double w = 1 - u - v;
                        const int h = std::min(w, std::min(u, v)) <= .01;

                        color = ARGB(h * u, h * v, h * w);
                    }

                    break;
                case RenderMode::UVCoords:
                    if (iteration.Hit)
                    {
                        const double u = std::get<0>(iteration.UVCoordinates);
                        const double v = std::get<1>(iteration.UVCoordinates);

                        color = ARGB(u, v, 1 - u - v);
                    }

                    break;
                case RenderMode::SurfaceNormals:
                    if (iteration.Hit)
                        color = iteration.SurfaceNormal.add(Vec3(1)).scale(.5);

                    break;
                case RenderMode::RayDirection:
                    color = ray.Direction.add(Vec3(1)).scale(.5);

                    break;
                case RenderMode::RayIncidenceAngle:
                    if (iteration.Hit)
                        color = ARGB(1 - std::abs(ray.Direction.angle_to(iteration.SurfaceNormal) / ROT_90));

                    break;
                case RenderMode::Iterations:
                    color = ARGB(1.0 - result.size() / double(config.MaximumIterationCount));

                    break;
                case RenderMode::RenderTime:
                {
                    const double µs = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

                    color = ARGB(std::log10(µs) * 0.30293575075);
                } break;
                default:
                    goto default_mode;
            }

            total = total + color * norm_factor;
        }
    }

    const size_t index = raw_x + size_t(raw_y * w);

    buffer[index] = buffer[index] + total / double(config.SamplesPerSubpixel);
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

RayTraceIteration TraceRay(const Scene* const scene, const RenderConfiguration& config, RayTraceResult* const result, const Ray& ray)
{
    if (ray.IterationDepth < config.MaximumIterationCount)
    {
        int iter_index = result->size();
        RayTraceIteration iteration;
        bool hit = false;
        double distance = INFINITY;
        bool inside = false;

        result->push_back(iteration);

        for (const Triangle& triangle : scene->mesh)
        {
            std::tuple<double, double> uv;
            double dist = INFINITY;
            bool ins = false;

            if (triangle.intersect(ray, &dist, &ins, &uv) && dist < distance)
            {
                iteration.IntersectionPoint = ray.evaluate(dist);
                iteration.SurfaceNormal = triangle.normal_at(iteration.IntersectionPoint);
                iteration.UVCoordinates = uv;
                hit = true;
                distance = dist;
                inside = ins;
            }
        }

        iteration.Ray = ray;
        iteration.Hit = hit;
        iteration.Distance = distance;

        if (!hit)
            iteration.ComputedColor = ARGB(0, 0); // TODO : handle environment colors?
        else
        {
            Vec3 direction = ray.Direction.reflect(iteration.SurfaceNormal);



            RayTraceIteration i1 = TraceRay(scene, config, result, ray.create_next(distance, direction));


            // TODO
            iteration.ComputedColor = ARGB(1.0 / (1 + ray.IterationDepth));
        }

        iteration.ComputedColor = ARGB(hit);
        result->at(iter_index) = iteration;

        return iteration;
    }

    return RayTraceIteration();
}
