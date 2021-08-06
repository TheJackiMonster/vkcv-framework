#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <chrono>
#include <limits>
#include <cmath>
#include <vector>
#include <string.h>	// memcpy(3)

/*
* Light struct with a position and intensity of the light source
*/
struct Light {
    Light(const glm::vec3 &p, const float &i) : position(p), intensity(i) {}
    glm::vec3 position;
    float intensity;
};

/*
* Material struct with defuse color, albedo and specular component
*/
struct Material {
    Material(const glm::vec3 &a, const glm::vec3 &color, const float &spec) : albedo(a),  diffuse_color(color), specular_exponent(spec) {}
    Material() : albedo(1,0, 0), diffuse_color(), specular_exponent() {}
    glm::vec3 diffuse_color;
    glm::vec3 albedo;
    float specular_exponent;
};

/*
* the sphere is defined by it's center, the radius and the material
* 
* the ray_intersect function checks, if a ray from the raytracer passes through the sphere, hits the sphere or passes by the the sphere
* @param vec3: origin of ray
* @param vec3: direction of ray
* @param float: distance of the ray to the sphere
* @return bool: if ray interesects sphere or not
*/
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

/* 
* @param vec3 dir: direction of the ray
* @param vec3 hit_center: normalized vector between hit on the sphere and center of the sphere
* @return vec3: returns reflected vector for the new direction of the ray
*/
glm::vec3 reflect(const glm::vec3 &dir, const glm::vec3 & hit_center) {
    return dir - hit_center*2.f*(glm::dot(dir, hit_center));
}

/*
* @param orig: Origin of the ray
* @param dir: direction of the ray
* @param vector: vector of all spheres in the scene
* @param vec3 hit: returns the vector from the origin of the ray to the closest sphere
* @param vec3 N: normalizes the vector from the origin of the ray to the closest sphere center
* @param Material: returns the material of the closest sphere
* @return: closest sphere distance if it's < 1000
*/
bool sceneIntersect(const glm::vec3 &orig, const glm::vec3 &dir, const std::vector<Sphere> &spheres, 
    glm::vec3 &hit, glm::vec3 &hit_center, Material &material) {
    float spheres_dist = std::numeric_limits<float>::max();
    for (size_t i=0; i < spheres.size(); i++) {
        float dist_i;
        if (spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < spheres_dist) {
            spheres_dist = dist_i;
            hit = orig + dir*dist_i;
            hit_center = glm::normalize(hit - spheres[i].center);
            material = spheres[i].material;
        }
    }
    return spheres_dist<1000;
}

/*
* @param vec3 orig: origin of the ray
* @param vec3 dir: direction of the ray
* @param vector: all spheres in the scene
* @param vector: all light sources of the scene
* @param depth = 0: initial recrusive depth
* @return color of the pixel depending on material and light
*/
glm::vec3 castRay(const glm::vec3 &orig, const glm::vec3 &dir, const std::vector<Sphere> &spheres, 
    const std::vector<Light> &lights, size_t depth = 0) {
    glm::vec3 point, hit_center;
    Material material;

    //return background color if a max recursive depth is reached
    if (depth > 4 || !sceneIntersect(orig, dir, spheres, point, hit_center, material)) {
        return glm::vec3(0.2, 0.7, 0.8);
    }

    //compute recursive directions and origins of rays and then call the function
    glm::vec3 reflect_dir = glm::normalize(reflect(dir, hit_center));
    glm::vec3 reflect_orig = (glm::dot(reflect_dir, hit_center) < 0) ? point - hit_center *static_cast<float>(1e-3) :
        point + hit_center *static_cast<float>(1e-3); // offset the original point to avoid occlusion by the object itself
    glm::vec3 reflect_color = castRay(reflect_orig, reflect_dir, spheres, lights, depth + 1);

    //compute shadows and other light properties for the returned ray color
    float diffuse_light_intensity = 0, specular_light_intensity = 0;
    for (size_t i=0; i<lights.size(); i++) {
        glm::vec3 light_dir = glm::normalize(lights[i].position - point);
        float light_distance = glm::distance(lights[i].position, point);

        glm::vec3 shadow_orig = (glm::dot(light_dir, hit_center) < 0) ? point - hit_center *static_cast<float>(1e-3) :
            point + hit_center*static_cast<float>(1e-3); // checking if the point lies in the shadow of the lights[i]
        glm::vec3 shadow_pt, shadow_hit_center;
        Material tmpmaterial;
        if (sceneIntersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_hit_center, tmpmaterial) 
            && glm::distance(shadow_pt, shadow_orig) < light_distance)
            continue;
        diffuse_light_intensity  += lights[i].intensity * std::max(0.f, glm::dot(light_dir, hit_center));
        specular_light_intensity += powf(std::max(0.f, glm::dot(reflect(light_dir, hit_center),dir)), material.specular_exponent)*lights[i].intensity;
    }
    return material.diffuse_color * diffuse_light_intensity * material.albedo[0] + 
        glm::vec3(1., 1., 1.)*specular_light_intensity * material.albedo[1] + reflect_color*material.albedo[2];
}

/*
* @param vector: all spheres in the scene
* @param vector: all light sources in the scene
* @return TextureData: texture data for the buffers
*/
vkcv::asset::TextureData render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights) {
    //constants for the image data
    const int width    = 800;
    const int height   = 600;
    const int fov      = M_PI/2.;

    //compute image format for the framebuffer and compute the ray colors for the image
    std::vector<glm::vec3> framebuffer(width*height);
#pragma omp parallel for
    for (size_t j = 0; j<height; j++) {
        for (size_t i = 0; i<width; i++) {
            framebuffer[i+j*width] = glm::vec3(j/float(height),i/float(width), 0);
            float x =  (2*(i + 0.5f)/(float)width  - 1)*tan(fov/2.f)*width/(float)height;
            float y = -(2*(j + 0.5f)/(float)height - 1)*tan(fov/2.f);
            glm::vec3 dir = glm::normalize(glm::vec3(x, y, -1));
            framebuffer[i+j*width] = castRay(glm::vec3(0,0,0), dir, spheres, lights);
        }
    }

    std::vector<uint8_t> data;
    for (size_t i = 0; i < height*width; ++i) {
        glm::vec3 &c = framebuffer[i];
        float max = std::max(c[0], std::max(c[1], c[2]));
        if (max>1) c = c*(1.f/max);
        data.push_back(static_cast<uint8_t>(255.f * framebuffer[i].x));
        data.push_back(static_cast<uint8_t>(255.f * framebuffer[i].y));
        data.push_back(static_cast<uint8_t>(255.f * framebuffer[i].z));
        data.push_back(static_cast<uint8_t>(255.f));
    }

    vkcv::asset::TextureData textureData;
    textureData.width = width;
    textureData.height = height;
    textureData.componentCount = 4;

    textureData.data.resize(textureData.width * textureData.height * textureData.componentCount);
    memcpy(textureData.data.data(), data.data(), textureData.data.size());
    return textureData;
}


int main(int argc, const char** argv) {
    const char* applicationName = "SAF_R";

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

    vkcv::ShaderProgram safrShaderProgram;
    vkcv::shader::GLSLCompiler compiler;

    compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("shaders/shader.vert"),
                     [&safrShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
                         safrShaderProgram.addShader(shaderStage, path);
                     });

    compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("shaders/shader.frag"),
                     [&safrShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
                         safrShaderProgram.addShader(shaderStage, path);
                     });

    uint32_t setID = 0;
    std::vector<vkcv::DescriptorBinding> descriptorBindings = { safrShaderProgram.getReflectedDescriptors()[setID] };
    vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorBindings);

    //materials for the spheres
    Material ivory(glm::vec3(0.6, 0.3, 0.1), glm::vec3(0.4, 0.4, 0.3), 50.);
    Material red_rubber(glm::vec3(0.9, 0.1, 0.0), glm::vec3(0.3, 0.1, 0.1), 10.);
    Material mirror(glm::vec3(0.0, 10.0, 0.8), glm::vec3(1.0, 1.0, 1.0), 1425.);

    //spheres for the scene
    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(glm::vec3(-3,    0,   -16), 2, ivory));
    spheres.push_back(Sphere(glm::vec3(-1.0, -1.5, -12), 2, mirror));
    spheres.push_back(Sphere(glm::vec3( 1.5, -0.5, -18), 3, red_rubber));
    spheres.push_back(Sphere(glm::vec3( 7,    5,   -18), 4, mirror));

    //lights for the scene
    std::vector<Light> lights;
    lights.push_back(Light(glm::vec3(-20, 20, 20), 1.5));
    lights.push_back(Light(glm::vec3( 30, 50, -25), 1.8));
    lights.push_back(Light(glm::vec3( 30, 20,  30), 1.7));
    //create the raytracer image for rendering
    vkcv::asset::TextureData texData = render(spheres, lights);

//  texData = vkcv::asset::loadTexture("textures/texture.png");
    vkcv::Image texture = core.createImage(vk::Format::eR8G8B8A8Unorm, texData.width, texData.height);
    texture.fill( texData.data.data());
    texture.generateMipChainImmediate();
    texture.switchLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    vkcv::SamplerHandle sampler = core.createSampler(
            vkcv::SamplerFilterType::LINEAR,
            vkcv::SamplerFilterType::LINEAR,
            vkcv::SamplerMipmapMode::LINEAR,
            vkcv::SamplerAddressMode::REPEAT
    );


    vkcv::DescriptorWrites setWrites;
    setWrites.sampledImageWrites    = { vkcv::SampledImageDescriptorWrite(0, texture.getHandle()) };
    setWrites.samplerWrites         = { vkcv::SamplerDescriptorWrite(1, sampler) };

    core.writeDescriptorSet(descriptorSet, setWrites);

    const auto& context = core.getContext();

    auto safrIndexBuffer = core.createBuffer<uint16_t>(vkcv::BufferType::INDEX, 3, vkcv::BufferMemoryType::DEVICE_LOCAL);
    uint16_t indices[3] = { 0, 1, 2 };
    safrIndexBuffer.fill(&indices[0], sizeof(indices));

    // an example attachment for passes that output to the window
    const vkcv::AttachmentDescription present_color_attachment(
            vkcv::AttachmentOperation::STORE,
            vkcv::AttachmentOperation::CLEAR,
            core.getSwapchain().getFormat());

    vkcv::PassConfig safrPassDefinition({ present_color_attachment });
    vkcv::PassHandle safrPass = core.createPass(safrPassDefinition);

    if (!safrPass)
    {
        std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
        return EXIT_FAILURE;
    }



    const vkcv::PipelineConfig safrPipelineDefinition {
            safrShaderProgram,
            (uint32_t)windowWidth,
            (uint32_t)windowHeight,
            safrPass,
            {},
            { core.getDescriptorSet(descriptorSet).layout },
            false
    };

    vkcv::PipelineHandle safrPipeline = core.createGraphicsPipeline(safrPipelineDefinition);

    if (!safrPipeline)
    {
        std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
        return EXIT_FAILURE;
    }

    auto start = std::chrono::system_clock::now();

    const vkcv::Mesh renderMesh({}, safrIndexBuffer.getVulkanHandle(), 3);
    vkcv::DescriptorSetUsage descriptorUsage(0, core.getDescriptorSet(descriptorSet).vulkanHandle);
    vkcv::DrawcallInfo drawcall(renderMesh, { descriptorUsage },1);

    const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

    vkcv::camera::CameraManager cameraManager(window);
    uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
    uint32_t camIndex1 = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);

    cameraManager.getCamera(camIndex0).setPosition(glm::vec3(0, 0, -2));
    cameraManager.getCamera(camIndex1).setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    cameraManager.getCamera(camIndex1).setCenter(glm::vec3(0.0f, 0.0f, -1.0f));

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
        glm::mat4 proj = cameraManager.getActiveCamera().getProjection();

        vkcv::PushConstants pushConstants (sizeof(glm::mat4) * 2);
        pushConstants.appendDrawcall(std::array<glm::mat4, 2>{ mvp, proj });

        auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

        core.recordDrawcallsToCmdStream(
                cmdStream,
                safrPass,
                safrPipeline,
                pushConstants,
                { drawcall },
                { swapchainInput });

        core.prepareSwapchainImageForPresent(cmdStream);
        core.submitCommandStream(cmdStream);

        core.endFrame();
    }
    return 0;
}
