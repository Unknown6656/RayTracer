#include "ray_tracer.hpp"

#define INIT_RAND std::srand(std::time(nullptr))
#define float_RAND (float(std::rand()) / RAND_MAX)


void CreateScene(scene** const _scene)
{
    if (_scene)
    {
        *_scene = (scene*)malloc(sizeof(scene));
        memset(*_scene, 0, sizeof(scene));

        **_scene = scene();
        (*_scene)->background_color = ARGB::TRANSPARENT;
        (*_scene)->add_spot_light(vec3(1, 10, 1), vec3(0, -1, 0), ARGB(1, 1, .7), 100);
        // (*_scene)->add_parallel_light(Vec3(-1, -4, 1), ARGB(1, 1, .7), .4);

        mesh_reference cube = (*_scene)->add_cube(vec3(-4, 3, 4), 2);
        // mesh_reference back = (*_scene)->add_planeXY(vec3(0, 10, -10), 20);
        // mesh_reference left = (*_scene)->add_planeXY(vec3(-10, 10, 0), 20, vec3(0, ROT_90, 0));
        mesh_reference floor = (*_scene)->add_planeXY(vec3::Zero, 20, vec3(ROT_90, 0, 0));
        mesh_reference ball = (*_scene)->add_sphere(vec3(0, 2, 0), 1.7);
        mesh_reference ico = (*_scene)->add_icosahedron(vec3(5, 3, -5), 2);

        cube.set_material(Material::diffuse(ARGB::RED));
        // back.set_material(Material::diffuse(ARGB::BLUE));
        // left.set_material(Material::emissive(ARGB::GREEN, 0.2));
        floor.set_material(Material::reflective(ARGB::BLACK, 1));
        ball.set_material(Material::reflective(ARGB(.5, .7, .84), .8));
        ico.set_material(Material::reflective(ARGB(.5, .7, .84), .8));
    }
}

void DeleteScene(scene** const scene)
{
    if (scene && *scene)
    {
        for (primitive* ptr : (*scene)->mesh)
            delete ptr;

        free(*scene);
    }
}

float RenderImage(const scene* const __restrict scene, render_configuration const config, ARGB* const __restrict buffer, float* const __restrict progress)
{
    assert(buffer != nullptr);

    INIT_RAND;

    const int w = config.horizontal_resolution;
    const int h = config.vertical_resolution;
    const int sub = config.subpixels_per_pixel;
    const size_t total_samples = float(w) * h * sub * sub * config.samples_per_subpixel;

    if (config.debug)
        std::cout << std::endl << "----------------------------------------------------------------" << std::endl
            << "Resolution(in pixels) : " << w << "x" << h << std::endl
            << "  Subpixels per pixel : " << sub << "x" << sub << std::endl
            << " Samples per subpixel : " << config.samples_per_subpixel << std::endl
            << "    Maximum ray depth : " << config.maximum_iteration_count << std::endl
            << "    Minimum ray count : " << total_samples << std::endl
            << "    Maximum ray count : " << total_samples * config.maximum_iteration_count << std::endl
            << "       Triangle count : " << scene->mesh.size() << std::endl
            << "          Render mode : " << config.mode << std::endl
            << "----------------------------------------------------------------" << std::endl;

    auto total_timer = std::chrono::high_resolution_clock::now();
    constexpr int CUBE_SZ = 128;
    std::atomic<size_t> pass_counter(size_t(0));

    if (progress)
        *progress = 0;

    for (int base_y = 0; base_y < h; base_y += CUBE_SZ)
        for (int base_x = 0; base_x < w; base_x += CUBE_SZ)
        {
            const int block_w = std::min(w - base_x, CUBE_SZ);
            const int block_h = std::min(h - base_y, CUBE_SZ);

            concurrency::parallel_for(
                size_t(0),
                size_t(block_w * block_h),
                [&](size_t index)
                {
                    const int pixel_x = base_x + index % block_w;
                    const int pixel_y = base_y + index / block_w;

                    for (int sample = 0; sample < config.samples_per_subpixel; ++sample)
                    {
                        ComputeRenderPass(scene, config, pixel_x, pixel_y, buffer, !sample);

                        if (progress)
                            *progress = float(++pass_counter) / (float(w) * h * config.samples_per_subpixel);
                    }
                }
            );
        }

    const auto elapsed = std::chrono::high_resolution_clock::now() - total_timer;

    return std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
}

void ComputeRenderPass(const scene* const scene, const render_configuration& config, const int raw_x, const int raw_y, ARGB* const& buffer, const bool clear)
{
    const int w = config.horizontal_resolution;
    const int h = config.vertical_resolution;
    const int sub = config.subpixels_per_pixel;
    const float subd(sub);
    const float pixel_x = raw_x / float(w) * 2.0 - 1.0;
    const float pixel_y = 1.0 - raw_y / float(h) * 2.0;
    const float norm_factor = 1.0 / (subd * subd);
    const size_t index = raw_x + size_t(raw_y * w);
    ARGB total = ARGB();

    if (clear)
        buffer[index] = total;

    for (int sx = 0; sx < sub; ++sx)
    {
        float x = float((sx + 0.5) / subd) / w + pixel_x;

        for (int sy = 0; sy < sub; ++sy)
        {
            float y = float((sy + 0.5) / subd) / w + pixel_y;
            auto start = std::chrono::high_resolution_clock::now();

            ray_trace_result result;
            const ray ray = CreateRay(config.camera, w, h, x, y);
            const ray_trace_iteration iteration = TraceRay(scene, config, &result, ray);

            const std::chrono::nanoseconds elapsed = std::chrono::high_resolution_clock::now() - start;
            ARGB color = ARGB();

            switch (config.mode)
            {
                case render_mode::colors:
                default_mode:
                    color = iteration.ComputedColor;

                    break;
                case render_mode::depths:
                    if (iteration.Hit)
                        color = ARGB(1.0 / (1 + 0.1 * iteration.Distance));

                    break;
                case render_mode::wireframe:
                    if (iteration.Hit)
                    {
                        const float u = std::get<0>(iteration.UVCoordinates);
                        const float v = std::get<1>(iteration.UVCoordinates);
                        const float w = 1 - u - v;
                        const int h = std::min(w, std::min(u, v)) <= .01;

                        color = ARGB(h * u, h * v, h * w);
                    }

                    break;
                case render_mode::uv_coords:
                    if (iteration.Hit)
                    {
                        const float u = std::get<0>(iteration.UVCoordinates);
                        const float v = std::get<1>(iteration.UVCoordinates);

                        color = ARGB(u, v, 1 - u - v);
                    }

                    break;
                case render_mode::surface_normals:
                    if (iteration.Hit)
                        color = iteration.SurfaceNormal.add(vec3(1)).scale(.5f);

                    break;
                case render_mode::ray_direction:
                    color = ray.direction.add(vec3(1)).scale(.5f);

                    break;
                case render_mode::ray_incidence_angle:
                    if (iteration.Hit)
                        color = ARGB(1 - std::abs(ray.direction.angle_to(iteration.SurfaceNormal) / ROT_90));

                    break;
                case render_mode::iterations:
                    color = ARGB(1.f - result.size() / float(config.maximum_iteration_count));

                    break;
                case render_mode::render_time:
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

    buffer[index] = buffer[index] + total / float(config.samples_per_subpixel);
}

inline ray CreateRay(const camera_configuration& camera, const float w, const float h, const float x, const float y)
{
    const float fov = 1.57079632679f / camera.zoom_factor;
    const vec3 gaze = camera.look_at.sub(camera.position).normalize();
    const vec3 up = vec3::UnitY;
    const vec3 camx = gaze.cross(up).normalize().scale(w * fov / h);
    const vec3 camy = camx.cross(gaze).normalize().scale(fov);
    const float dx = 2 * float_RAND / w;
    const float dy = 2 * float_RAND / h;
    const vec3 dir = gaze.add(camx.scale(x + dx))
                         .add(camy.scale(y + dy))
                         .normalize();

    ray r(camera.position + dir.scale(camera.focal_length), dir, 0);

    return r;
}

inline ray_trace_iteration TraceRay(const scene* const __restrict scene, const render_configuration& config, ray_trace_result* const __restrict result, const ray& ray)
{
    if (ray.iteration_depth < config.maximum_iteration_count)
    {
        int iter_index = result->size();
        ray_trace_iteration iteration;
        bool hit = false;
        float distance = INFINITY;
        bool inside = false;

        result->push_back(iteration);

        for (int i = 0, l = scene->mesh.size(); i < l; ++i)
        {
            const primitive* primitive = scene->mesh[i];
            std::tuple<float, float> uv;
            float dist = INFINITY;
            bool ins = false;

            if (primitive->intersect(ray, &dist, &ins, &uv) && dist < distance)
            {
                iteration.IntersectionPoint = ray.evaluate(dist);
                iteration.SurfaceNormal = primitive->normal_at(iteration.IntersectionPoint);
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

    return ray_trace_iteration();
}

inline void ComputeColor(const scene* const __restrict scene, const render_configuration& config, ray_trace_iteration* const __restrict iteration)
{
    const Material& mat = scene->mesh[iteration->TriangleIndex]->material;
    const vec3& normal = iteration->SurfaceNormal;

    ARGB diffuse = ARGB::TRANSPARENT;
    ARGB specular = ARGB::TRANSPARENT;


    // LAMBERT DIFFUSE SHADING
    for (const light& light : scene->lights)
        if (light.mode == light::light_mode::Global)
            diffuse = diffuse + mat.DiffuseColor * (light.diffuse_color * light.diffuse_intensity);
        else if (light.mode == light::light_mode::Parallel)
        {
            const float intensity = light.diffuse_intensity * light.direction.angle_to(normal);

            if (intensity > 0)
                diffuse = diffuse + (mat.DiffuseColor * (light.diffuse_color * intensity));
        }
        else if (light.mode == light::light_mode::Spot)
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
