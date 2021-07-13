#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <chrono>
#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>


struct Light {
    Light(const glm::vec3 &p, const float &i) : position(p), intensity(i) {}
    glm::vec3 position;
    float intensity;
};

struct Material {
    Material(const glm::vec3 &a, const glm::vec3 &color, const float &spec) : albedo(a),  diffuse_color(color), specular_exponent(spec) {}
    Material() : albedo(1,0, 0), diffuse_color(), specular_exponent() {}
    glm::vec3 diffuse_color;
    glm::vec3 albedo;
    float specular_exponent;
};

struct Sphere {
    glm::vec3 center;
    float radius;
    Material material;

    Sphere(const glm::vec3 &c, const float &r, const Material &m) : center(c), radius(r), material(m) {}

    bool ray_intersect(const glm::vec3 &origin, const glm::vec3 &dir, float &t0) const {
        glm::vec3 L = center - origin;
        float tca = glm::dot(L, dir);
        float d2 = glm::dot(L,L) - tca*tca;
        if (d2 > radius*radius) return false;
        float thc = sqrtf(radius*radius - d2);
        t0       = tca - thc;
        float t1 = tca + thc;
        if (t0 < 0) t0 = t1;
        if (t0 < 0) return false;
        return true;
    }
};

glm::vec3 reflect(const glm::vec3 &I, const glm::vec3 &N) {
    return I - N*2.f*(glm::dot(I,N));
}

bool scene_intersect(const glm::vec3 &orig, const glm::vec3 &dir, const std::vector<Sphere> &spheres, glm::vec3 &hit, glm::vec3 &N, Material &material) {
    float spheres_dist = std::numeric_limits<float>::max();
    for (size_t i=0; i < spheres.size(); i++) {
        float dist_i;
        if (spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < spheres_dist) {
            spheres_dist = dist_i;
            hit = orig + dir*dist_i;
            N = glm::normalize(hit - spheres[i].center);
            material = spheres[i].material;
        }
    }
    return spheres_dist<1000;
}

glm::vec3 cast_ray(const glm::vec3 &orig, const glm::vec3 &dir, const std::vector<Sphere> &spheres, const std::vector<Light> &lights, size_t depth = 0) {
    glm::vec3 point, N;
    Material material;

    if (depth > 4 || !scene_intersect(orig, dir, spheres, point, N, material)) {
        return glm::vec3(0.2, 0.7, 0.8); // background color
    }

    glm::vec3 reflect_dir = glm::normalize(reflect(dir, N));
    glm::vec3 reflect_orig = (glm::dot(reflect_dir,N) < 0) ? point - N*static_cast<float>(1e-3) : point + N*static_cast<float>(1e-3); // offset the original point to avoid occlusion by the object itself
    glm::vec3 reflect_color = cast_ray(reflect_orig, reflect_dir, spheres, lights, depth + 1);

    float diffuse_light_intensity = 0, specular_light_intensity = 0;
    for (size_t i=0; i<lights.size(); i++) {
        glm::vec3 light_dir      = glm::normalize(lights[i].position - point);
        float light_distance = glm::distance(lights[i].position,point);

        glm::vec3 shadow_orig = (glm::dot(light_dir,N) < 0) ? point - N*static_cast<float>(1e-3) : point + N*static_cast<float>(1e-3); // checking if the point lies in the shadow of the lights[i]
        glm::vec3 shadow_pt, shadow_N;
        Material tmpmaterial;
        if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && glm::distance(shadow_pt, shadow_orig) < light_distance)
            continue;
        diffuse_light_intensity  += lights[i].intensity * std::max(0.f, glm::dot(light_dir,N));
        specular_light_intensity += powf(std::max(0.f, glm::dot(reflect(light_dir, N),dir)), material.specular_exponent)*lights[i].intensity;
    }
    return material.diffuse_color * diffuse_light_intensity * material.albedo[0] + glm::vec3(1., 1., 1.)*specular_light_intensity * material.albedo[1] + reflect_color*material.albedo[2];
}


void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights) {
    const int width    = 800;
    const int height   = 600;
    const int fov      = M_PI/2.;
    std::vector<glm::vec3> framebuffer(width*height);
    std::string path = "textures";
#pragma omp parallel for
    for (size_t j = 0; j<height; j++) {
        for (size_t i = 0; i<width; i++) {
            framebuffer[i+j*width] = glm::vec3(j/float(height),i/float(width), 0);
            float x =  (2*(i + 0.5f)/(float)width  - 1)*tan(fov/2.f)*width/(float)height;
            float y = -(2*(j + 0.5f)/(float)height - 1)*tan(fov/2.f);
            glm::vec3 dir = glm::normalize(glm::vec3(x, y, -1));
            framebuffer[i+j*width] = cast_ray(glm::vec3(0,0,0), dir, spheres, lights);
        }
    }

    std::vector<int> img;
    std::ofstream ofs; // save the framebuffer to file
    ofs.open(path + "./texture.ppm");
    ofs << "P3\n" << width << " " << height << "\n255\n";

    for (size_t i = 0; i < height*width; ++i) {
        glm::vec3 &c = framebuffer[i];
        float max = std::max(c[0], std::max(c[1], c[2]));
        if (max>1) c = c*(1.f/max);
        ofs << static_cast<int>(255.f * framebuffer[i].x);
        ofs << " ";
        ofs << static_cast<int>(255.f * framebuffer[i].y);
        ofs << " ";
        ofs << static_cast<int>(255.f * framebuffer[i].z);
        ofs << " ";
    }
    ofs.close();
}


int main(int argc, const char** argv) {
    const char* applicationName = "First Triangle";

    const int windowWidth = 800;
    const int windowHeight = 600;
    vkcv::Window window = vkcv::Window::create(
            applicationName,
            windowWidth,
            windowHeight,
            false
    );

    vkcv::Core core = vkcv::Core::create(
            window,
            applicationName,
            VK_MAKE_VERSION(0, 0, 1),
            { vk::QueueFlagBits::eTransfer,vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute },
            {},
            { "VK_KHR_swapchain" }
    );

    vkcv::asset::TextureData texData = vkcv::asset::loadTexture("textures/texture.png");
    vkcv::Image texture = core.createImage(vk::Format::eR8G8B8A8Srgb, 800, 600);
    texture.fill( texData.data.data());
    texture.generateMipChainImmediate();
    texture.switchLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    const auto& context = core.getContext();

    auto triangleIndexBuffer = core.createBuffer<uint16_t>(vkcv::BufferType::INDEX, 3, vkcv::BufferMemoryType::DEVICE_LOCAL);
    uint16_t indices[3] = { 0, 1, 2 };
    triangleIndexBuffer.fill(&indices[0], sizeof(indices));

    // an example attachment for passes that output to the window
    const vkcv::AttachmentDescription present_color_attachment(
            vkcv::AttachmentOperation::STORE,
            vkcv::AttachmentOperation::CLEAR,
            core.getSwapchain().getFormat());

    vkcv::PassConfig trianglePassDefinition({ present_color_attachment });
    vkcv::PassHandle trianglePass = core.createPass(trianglePassDefinition);

    if (!trianglePass)
    {
        std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
        return EXIT_FAILURE;
    }

    vkcv::ShaderProgram triangleShaderProgram;
    vkcv::shader::GLSLCompiler compiler;

    compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("shaders/shader.vert"),
                     [&triangleShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
                         triangleShaderProgram.addShader(shaderStage, path);
                     });

    compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("shaders/shader.frag"),
                     [&triangleShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
                         triangleShaderProgram.addShader(shaderStage, path);
                     });

    const vkcv::PipelineConfig trianglePipelineDefinition {
            triangleShaderProgram,
            (uint32_t)windowWidth,
            (uint32_t)windowHeight,
            trianglePass,
            {},
            {},
            false
    };

    vkcv::PipelineHandle trianglePipeline = core.createGraphicsPipeline(trianglePipelineDefinition);

    if (!trianglePipeline)
    {
        std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
        return EXIT_FAILURE;
    }

    auto start = std::chrono::system_clock::now();

    const vkcv::Mesh renderMesh({}, triangleIndexBuffer.getVulkanHandle(), 3);
    vkcv::DrawcallInfo drawcall(renderMesh, {},1);

    const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

    vkcv::camera::CameraManager cameraManager(window);
    uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
    uint32_t camIndex1 = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);

    cameraManager.getCamera(camIndex0).setPosition(glm::vec3(0, 0, -2));
    cameraManager.getCamera(camIndex1).setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    cameraManager.getCamera(camIndex1).setCenter(glm::vec3(0.0f, 0.0f, -1.0f));


    Material      ivory(glm::vec3(0.6,  0.3, 0.1), glm::vec3(0.4, 0.4, 0.3),   50.);
    Material red_rubber(glm::vec3(0.9,  0.1, 0.0), glm::vec3(0.3, 0.1, 0.1),   10.);
    Material     mirror(glm::vec3(0.0, 10.0, 0.8), glm::vec3(1.0, 1.0, 1.0), 1425.);

    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(glm::vec3(-3,    0,   -16), 2,      ivory));
    spheres.push_back(Sphere(glm::vec3(-1.0, -1.5, -12), 2, mirror));
    spheres.push_back(Sphere(glm::vec3( 1.5, -0.5, -18), 3, red_rubber));
    spheres.push_back(Sphere(glm::vec3( 7,    5,   -18), 4,      mirror));

    std::vector<Light> lights;
    lights.push_back(Light(glm::vec3(-20, 20, 20), 1.5));
    lights.push_back(Light(glm::vec3( 30, 50, -25), 1.8));
    lights.push_back(Light(glm::vec3( 30, 20,  30), 1.7));
    render(spheres, lights);

    while (window.isWindowOpen())
    {
        vkcv::Window::pollEvents();

        uint32_t swapchainWidth, swapchainHeight; // No resizing = No problem
        if (!core.beginFrame(swapchainWidth, swapchainHeight)) {
            continue;
        }

        auto end = std::chrono::system_clock::now();
        auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        start = end;

        cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));
        glm::mat4 mvp = cameraManager.getActiveCamera().getMVP();

        vkcv::PushConstants pushConstants (sizeof(glm::mat4));
        pushConstants.appendDrawcall(mvp);

        auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

        core.recordDrawcallsToCmdStream(
                cmdStream,
                trianglePass,
                trianglePipeline,
                pushConstants,
                { drawcall },
                { swapchainInput });

        core.prepareSwapchainImageForPresent(cmdStream);
        core.submitCommandStream(cmdStream);

        core.endFrame();
    }
    return 0;
}
