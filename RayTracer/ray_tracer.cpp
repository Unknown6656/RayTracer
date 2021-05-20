#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <ctime>
#include <ppl.h>

#include "ray_tracer.hpp"

#define INIT_RAND std::srand(std::time(nullptr))
#define float_RAND (float(std::rand()) / RAND_MAX)


void CreateScene(Scene** const scene)
{
    if (scene)
    {
        *scene = (Scene*)malloc(sizeof(Scene));
        memset(*scene, 0, sizeof(Scene));

        **scene = Scene();
        (*scene)->background_color = ARGB::TRANSPARENT;
        (*scene)->add_spot_light(vec3(1, 10, 1), vec3(0, -1, 0), ARGB(1, 1, .7), 100);
        //(*scene)->add_parallel_light(Vec3(-1, -4, 1), ARGB(1, 1, .7), .4);

        MeshReference cube = (*scene)->add_cube(vec3(-4, 3, 4), 2);
        //MeshReference back = (*scene)->add_planeXY(vec3(0, 10, -10), 20);
        //MeshReference left = (*scene)->add_planeXY(vec3(-10, 10, 0), 20, vec3(0, ROT_90, 0));
        MeshReference floor = (*scene)->add_planeXY(vec3::Zero, 20, vec3(ROT_90, 0, 0));
        MeshReference ball = (*scene)->add_sphere(vec3(0, 2, 0), 1.7);
        MeshReference ico = (*scene)->add_icosahedron(vec3(5, 3, -5), 2);

        cube.set_material(Material::diffuse(ARGB::RED));
        //back.set_material(Material::diffuse(ARGB::BLUE));
        //left.set_material(Material::emissive(ARGB::GREEN, 0.2));
        floor.set_material(Material::reflective(ARGB::BLACK, 1));
        ball.set_material(Material::reflective(ARGB(.5, .7, .84), .8));
        ico.set_material(Material::reflective(ARGB(.5, .7, .84), .8));

        // TODO
    }
}

void DeleteScene(Scene** const scene)
{
    if (scene)
        free(*scene);
}

void RenderImage(const Scene* const scene, RenderConfiguration const config, ARGB* const buffer, float* const progress)
{
    assert(buffer != nullptr);

    std::cout << "sizeof(Scene) = " << sizeof(Scene) << std::endl;

    INIT_RAND;

    const int w = config.HorizontalResolution;
    const int h = config.VerticalResolution;
    const int sub = config.SubpixelsPerPixel;
    const size_t total_samples = float(w) * h * sub * sub * config.SamplesPerSubpixel;

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
                            *progress = float(++pass_counter) / (float(w) * h * config.SamplesPerSubpixel);
                    }
                );
            }

    const auto elapsed = std::chrono::high_resolution_clock::now() - total_timer;
    const float µs = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

    printf("Render time: %fs\n", µs * 1e-6);
}

void ComputeRenderPass(const Scene* const scene, const RenderConfiguration& config, const int raw_x, const int raw_y, ARGB* const& buffer)
{
    const int w = config.HorizontalResolution;
    const int h = config.VerticalResolution;
    const int sub = config.SubpixelsPerPixel;
    const float subd(sub);
    const float pixel_x = raw_x / float(w) * 2.0 - 1.0;
    const float pixel_y = 1.0 - raw_y / float(h) * 2.0;
    const float norm_factor = 1.0 / (subd * subd);
    ARGB total;

    for (int sx = 0; sx < sub; ++sx)
    {
        float x = float((sx + 0.5) / subd) / w + pixel_x;

        for (int sy = 0; sy < sub; ++sy)
        {
            float y = float((sy + 0.5) / subd) / w + pixel_y;
            auto start = std::chrono::high_resolution_clock::now();

            RayTraceResult result;
            const ray ray = CreateRay(config.Camera, w, h, x, y);
            const RayTraceIteration iteration = TraceRay(scene, config, &result, ray);

            const std::chrono::nanoseconds elapsed = std::chrono::high_resolution_clock::now() - start;
            ARGB color;

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
                        const float u = std::get<0>(iteration.UVCoordinates);
                        const float v = std::get<1>(iteration.UVCoordinates);
                        const float w = 1 - u - v;
                        const int h = std::min(w, std::min(u, v)) <= .01;

                        color = ARGB(h * u, h * v, h * w);
                    }

                    break;
                case RenderMode::UVCoords:
                    if (iteration.Hit)
                    {
                        const float u = std::get<0>(iteration.UVCoordinates);
                        const float v = std::get<1>(iteration.UVCoordinates);

                        color = ARGB(u, v, 1 - u - v);
                    }

                    break;
                case RenderMode::SurfaceNormals:
                    if (iteration.Hit)
                        color = iteration.SurfaceNormal.add(vec3(1)).scale(.5f);

                    break;
                case RenderMode::RayDirection:
                    color = ray.direction.add(vec3(1)).scale(.5f);

                    break;
                case RenderMode::RayIncidenceAngle:
                    if (iteration.Hit)
                        color = ARGB(1 - std::abs(ray.direction.angle_to(iteration.SurfaceNormal) / ROT_90));

                    break;
                case RenderMode::Iterations:
                    color = ARGB(1.f - result.size() / float(config.MaximumIterationCount));

                    break;
                case RenderMode::RenderTime:
                {
                    const float µs = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

                    color = ARGB(std::log10(µs) * .30293575075f);
                } break;
                default:
                    goto default_mode;
            }

            total = total + color * norm_factor;
        }
    }

    const size_t index = raw_x + size_t(raw_y * w);

    buffer[index] = buffer[index] + total / float(config.SamplesPerSubpixel);
}

inline ray CreateRay(const CameraConfiguration& camera, const float w, const float h, const float x, const float y)
{
    const float fov = 1.57079632679f / camera.ZoomFactor;
    const vec3 gaze = camera.LookAt.sub(camera.Position).normalize();
    const vec3 up = vec3::UnitY;
    const vec3 camx = gaze.cross(up).normalize().scale(w * fov / h);
    const vec3 camy = camx.cross(gaze).normalize().scale(fov);
    const float dx = 2 * float_RAND / w;
    const float dy = 2 * float_RAND / h;
    const vec3 dir = gaze.add(camx.scale(x + dx))
                         .add(camy.scale(y + dy))
                         .normalize();

    ray r(camera.Position + dir.scale(camera.FocalLength), dir, 0);

    return r;
}

inline RayTraceIteration TraceRay(const Scene* const scene, const RenderConfiguration& config, RayTraceResult* const result, const ray& ray)
{
    if (ray.iteration_depth < config.MaximumIterationCount)
    {
        int iter_index = result->size();
        RayTraceIteration iteration;
        bool hit = false;
        float distance = INFINITY;
        bool inside = false;

        result->push_back(iteration);

        for (int i = 0, l = scene->mesh.size(); i < l; ++i)
        {
            const Primitive& primitive = scene->mesh[i];
            std::tuple<float, float> uv;
            float dist = INFINITY;
            bool ins = false;

            if (primitive.intersect(ray, &dist, &ins, &uv) && dist < distance)
            {
                iteration.IntersectionPoint = ray.evaluate(dist);
                iteration.SurfaceNormal = primitive.normal_at(iteration.IntersectionPoint);
                iteration.UVCoordinates = uv;
                iteration.TriangleIndex = i;
                hit = true;
                distance = dist;
                inside = ins;
            }
        }

        iteration.Ray = ray;
        iteration.Hit = hit;
        iteration.Distance = distance;

        if (!hit)
            iteration.ComputedColor = scene->background_color;
        else
        {
            ComputeColor(scene, config, &iteration);
        }

        result->at(iter_index) = iteration;

        return iteration;
    }

    return RayTraceIteration();
}

inline void ComputeColor(const Scene* const scene, const RenderConfiguration& config, RayTraceIteration* const iteration)
{
    const Material& mat = scene->mesh[iteration->TriangleIndex].material;
    const vec3& normal = iteration->SurfaceNormal;

    ARGB diffuse = ARGB::TRANSPARENT;
    ARGB specular = ARGB::TRANSPARENT;


    // LAMBERT DIFFUSE SHADING
    for (const Light& light : scene->lights)
        if (light.mode == Light::LightMode::Global)
            diffuse = diffuse + mat.DiffuseColor * (light.diffuse_color * light.diffuse_intensity);
        else if (light.mode == Light::LightMode::Parallel)
        {
            const float intensity = light.diffuse_intensity * light.direction.angle_to(normal);

            if (intensity > 0)
                diffuse = diffuse + (mat.DiffuseColor * (light.diffuse_color * intensity));
        }
        else if (light.mode == Light::LightMode::Spot)
        {
            const float dist_sq = std::pow(light.position.distance_to(iteration->IntersectionPoint), 2);

            if (light.diffuse_intensity > 0)
            {
                const float diffuse_intensity = normal.normalize().dot(light.direction);

                diffuse = light.diffuse_color * (diffuse_intensity * light.diffuse_intensity / dist_sq);
            }

            if (light.specular_intensity > 0)
            {
                const float specular_intensity = std::pow(light.direction.add(iteration->Ray.direction).normalize().dot(normal), light.falloff_exponent);

                specular = light.specular_color * (specular_intensity * light.specular_intensity / dist_sq);
            }
        }

    iteration->ComputedColor = diffuse;

    return;
    //Vec3 direction = iteration->Ray.Direction.reflect(iteration->SurfaceNormal);
    //RayTraceIteration i1 = TraceRay(scene, config, result, ray.create_next(distance, direction));
}
