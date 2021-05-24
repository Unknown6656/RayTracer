#include "ray_tracer.hpp"

#define INIT_RAND std::srand(std::time(nullptr))
#define float_RAND (float(std::rand()) / RAND_MAX)


using namespace ray_tracer_3d;


scene* ray_tracer_3d::CreateScene3()
{
    scene* sc = new scene();

    sc->add_spot_light(vec3(1, 10, 1), vec3(0, -1, 0), ARGB(1, 1, .7), 100);
    // sc->add_parallel_light(Vec3(-1, -4, 1), ARGB(1, 1, .7), .4);

    mesh_reference cube = sc->add_cube(vec3(-4, 3, 4), 2);
    // mesh_reference back = sc->add_planeXY(vec3(0, 10, -10), 20);
    // mesh_reference left = sc->add_planeXY(vec3(-10, 10, 0), 20, vec3(0, ROT_90, 0));
    mesh_reference floor = sc->add_planeXY(vec3::Zero, 20, vec3(ROT_90, 0, 0));
    mesh_reference ball = sc->add_sphere(vec3(0, 2, 0), 1.7);
    mesh_reference ico = sc->add_icosahedron(vec3(5, 3, -5), 2);

    cube.set_material(material::diffuse(ARGB::RED));
    // back.set_material(material::diffuse(ARGB::BLUE));
    // left.set_material(material::emissive(ARGB::GREEN, 0.2));
    floor.set_material(material::reflective(ARGB::BLACK, 1));
    ball.set_material(material::reflective(ARGB(.5, .7, .84), .8));
    ico.set_material(material::reflective(ARGB(.5, .7, .84), .8));

    return sc;
}

void ray_tracer_3d::DeleteScene3(scene* const scene)
{
    if (scene)
    {
        for (primitive* ptr : scene->mesh)
            delete ptr;

        delete scene;
    }
}

float ray_tracer_3d::RenderImage3(const scene* const __restrict scene, render_configuration const config, ARGB* const __restrict buffer, float* const __restrict progress)
{
    assert(buffer != nullptr);

    INIT_RAND;

    const int w = config.horizontal_resolution;
    const int h = config.vertical_resolution;
    const int sub = config.subpixels_per_pixel;
    const size_t total_samples = size_t(w) * h * sub * sub * config.samples_per_subpixel;

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
                size_t(block_w) * size_t(block_h),
                [&](size_t index)
                {
                    const int pixel_x = base_x + index % block_w;
                    const int pixel_y = base_y + index / block_w;

                    for (int sample = 0; sample < config.samples_per_subpixel; ++sample)
                    {
                        ComputeRenderPass3(scene, config, pixel_x, pixel_y, buffer, !sample);

                        if (progress)
                            *progress = float(++pass_counter) / (float(w) * h * config.samples_per_subpixel);
                    }
                }
            );
        }

    const auto elapsed = std::chrono::high_resolution_clock::now() - total_timer;

    return std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
}

void ray_tracer_3d::ComputeRenderPass3(const scene* const scene, const render_configuration& config, const int raw_x, const int raw_y, ARGB* const& buffer, const bool clear)
{
    const int w = config.horizontal_resolution;
    const int h = config.vertical_resolution;
    const int sub = config.subpixels_per_pixel;
    const float subd(sub);
    const float pixel_x = raw_x / float(w) * 2.f - 1.f;
    const float pixel_y = 1.f - raw_y / float(h) * 2.f;
    const float norm_factor = 1.f / (subd * subd);
    const size_t index = raw_x + size_t(raw_y * w);
    ARGB total = ARGB();

    if (clear)
        buffer[index] = config.background_color;

    for (int sx = 0; sx < sub; ++sx)
    {
        const float x = float((sx + .5f) / subd) / w + pixel_x;

        for (int sy = 0; sy < sub; ++sy)
        {
            const float y = float((sy + .5f) / subd) / w + pixel_y;
            const auto start = std::chrono::high_resolution_clock::now();

            ray_trace_result result;
            const ray3 ray = CreateRay3(config, w, h, x, y);
            const ray_trace_iteration iteration = TraceRay3(scene, config, &result, ray);
            const std::chrono::nanoseconds elapsed = std::chrono::high_resolution_clock::now() - start;
            const bool is_hit = iteration.hit.type != hit_test::hit_type::no_hit;
            ARGB color = ARGB();

            switch (config.mode)
            {
                case render_mode::depths:
                    if (is_hit)
                        color = ARGB(1.f / (1 + .1f * iteration.hit.distance));

                    break;
                case render_mode::wireframe:
                    if (is_hit)
                    {
                        const float u = iteration.hit.uv.X;
                        const float v = iteration.hit.uv.Y;
                        const float w = 1 - u - v;
                        const int h = std::min(w, std::min(u, v)) <= .01;

                        color = ARGB(h * u, h * v, h * w);
                    }

                    break;
                case render_mode::uv_coords:
                    if (is_hit)
                    {
                        const float u = iteration.hit.uv.X;
                        const float v = iteration.hit.uv.Y;

                        color = ARGB(u, v, 1 - u - v);
                    }

                    break;
                case render_mode::surface_normals:
                    if (is_hit)
                        color = iteration.surface_normal.add(vec3(1)).scale(.5f);

                    break;
                case render_mode::ray_direction:
                    color = ray.direction.add(vec3(1)).scale(.5f);

                    break;
                case render_mode::ray_incidence_angle:
                    if (is_hit)
                        color = ARGB(1 - std::abs(ray.direction.angle_to(iteration.surface_normal) / ROT_90));

                    break;
                case render_mode::iterations:
                    color = ARGB(1.f - result.size() / float(config.maximum_iteration_count));

                    break;
                case render_mode::render_time:
                {
                    const float µs = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

                    color = ARGB(std::log10(µs) * .30293575075f);
                } break;
                case render_mode::colors:
                default:
                    color = iteration.computed_color;

                    break;
            }

            total = total + color * norm_factor;
        }
    }

    buffer[index] = buffer[index] + total / float(config.samples_per_subpixel);
}

ray3 ray_tracer_3d::CreateRay3(const render_configuration& config, const float w, const float h, const float x, const float y)
{
    const float fov = M_PI_2 / config.camera.zoom_factor;
    const vec3 gaze = config.camera.look_at.sub(config.camera.position).normalize();
    const vec3 up = vec3::UnitY;
    const vec3 camx = gaze.cross(up).normalize().scale(w * fov / h);
    const vec3 camy = camx.cross(gaze).normalize().scale(fov);
    const float dx = 2 * float_RAND / w;
    const float dy = 2 * float_RAND / h;
    const vec3 dir = gaze.add(camx.scale(x + dx))
                         .add(camy.scale(y + dy))
                         .normalize();

    ray3 r(config.camera.position + dir.scale(config.camera.focal_length), dir, 0, config.air_refraction_index, false);

    return r;
}

ray_trace_iteration ray_tracer_3d::TraceRay3(const scene* const __restrict scene, const render_configuration& config, ray_trace_result* const __restrict result, const ray3& ray)
{
    if (ray.iteration_depth < config.maximum_iteration_count)
    {
        // int iter_index = result->size();

        hit_test local_hit = hit_test();
        ray_trace_iteration iteration = ray_trace_iteration();

        iteration.ray = ray;
        iteration.hit = hit_test();
        iteration.hit.distance = INFINITY;
        iteration.primitive = nullptr;

        for (int i = 0, l = scene->mesh.size(); i < l; ++i)
        {
            primitive* const primitive = scene->mesh[i];

            primitive->intersect(ray, &local_hit);

            if (local_hit.distance < iteration.hit.distance)
            {
                iteration.hit = local_hit;
                iteration.primitive = primitive;
                iteration.intersection_point = iteration.ray(local_hit.distance);
                iteration.surface_normal = primitive->normal_at(iteration.intersection_point);
            }
        }

        if (iteration.hit.type != hit_test::hit_type::no_hit && iteration.hit.distance < INFINITY)
            ComputeColor3(scene, config, &iteration);
        else
            iteration.computed_color = config.background_color;

        result->push_back(iteration);

        return iteration;
    }

    return ray_trace_iteration();
}

void ray_tracer_3d::ComputeColor3(const scene* const __restrict scene, const render_configuration& config, ray_trace_iteration* const __restrict iteration)
{
    const material& mat = iteration->primitive->material;
    const vec3& normal = iteration->surface_normal;

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
            const float dist_sq = std::pow(light.position.distance_to(iteration->intersection_point), 2);

            if (light.diffuse_intensity > 0)
            {
                const float diffuse_intensity = normal.normalize().dot(light.direction);

                diffuse = light.diffuse_color * (diffuse_intensity * light.diffuse_intensity / dist_sq);
            }

            if (light.specular_intensity > 0)
            {
                const float specular_intensity = std::pow(light.direction.add(iteration->ray.direction).normalize().dot(normal), light.falloff_exponent);

                specular = light.specular_color * (specular_intensity * light.specular_intensity / dist_sq);
            }
        }

    iteration->computed_color = diffuse;

    return;
    //Vec3 direction = iteration->Ray.Direction.reflect(iteration->SurfaceNormal);
    //RayTraceIteration i1 = TraceRay(scene, config, result, ray.create_next(distance, direction));
}
