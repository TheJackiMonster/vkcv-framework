#include "AppSetup.hpp"
#include "AppConfig.hpp"
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>

bool loadSphereMesh(vkcv::Core& core, MeshResources* outMesh) {
    assert(outMesh);

    vkcv::asset::Scene sphereScene;
    const int meshLoadResult = vkcv::asset::loadScene("resources/models/Sphere.gltf", sphereScene);

    if (meshLoadResult != 1) {
        vkcv_log(vkcv::LogLevel::ERROR, "Mesh loading failed");
        return false;
    }

    if (sphereScene.meshes.size() < 1) {
        vkcv_log(vkcv::LogLevel::ERROR, "Sphere mesh scene does not contain any vertex groups");
        return false;
    }
    assert(!sphereScene.vertexGroups.empty());

    auto& sphereVertexData = sphereScene.vertexGroups[0].vertexBuffer;
    auto& sphereIndexData = sphereScene.vertexGroups[0].indexBuffer;

    vkcv::Buffer vertexBuffer = core.createBuffer<uint8_t>(
        vkcv::BufferType::VERTEX,
        sphereVertexData.data.size(),
        vkcv::BufferMemoryType::DEVICE_LOCAL);

    vkcv::Buffer indexBuffer = core.createBuffer<uint8_t>(
        vkcv::BufferType::INDEX,
        sphereIndexData.data.size(),
        vkcv::BufferMemoryType::DEVICE_LOCAL);

    vertexBuffer.fill(sphereVertexData.data);
    indexBuffer.fill(sphereIndexData.data);

    outMesh->vertexBuffer = vertexBuffer.getHandle();
    outMesh->indexBuffer = indexBuffer.getHandle();

    auto& attributes = sphereVertexData.attributes;

    std::sort(attributes.begin(), attributes.end(),
        [](const vkcv::asset::VertexAttribute& x, const vkcv::asset::VertexAttribute& y) {
        return static_cast<uint32_t>(x.type) < static_cast<uint32_t>(y.type);
    });

    const std::vector<vkcv::VertexBufferBinding> vertexBufferBindings = {
        vkcv::VertexBufferBinding(static_cast<vk::DeviceSize>(attributes[0].offset), vertexBuffer.getVulkanHandle()),
        vkcv::VertexBufferBinding(static_cast<vk::DeviceSize>(attributes[1].offset), vertexBuffer.getVulkanHandle()) };

    outMesh->mesh = vkcv::Mesh(vertexBufferBindings, indexBuffer.getVulkanHandle(), sphereScene.vertexGroups[0].numIndices);

    return true;
}

bool loadMeshGraphicPass(vkcv::Core& core, GraphicPassHandles* outPassHandles) {
    assert(outPassHandles);

    const vkcv::AttachmentDescription present_color_attachment(
        vkcv::AttachmentOperation::STORE,
        vkcv::AttachmentOperation::CLEAR,
        core.getSwapchain().getFormat());

    const vkcv::AttachmentDescription depth_attachment(
        vkcv::AttachmentOperation::STORE,
        vkcv::AttachmentOperation::CLEAR,
        AppConfig::depthBufferFormat);

    vkcv::PassConfig meshPassDefinition({ present_color_attachment, depth_attachment });
    outPassHandles->renderPass = core.createPass(meshPassDefinition);

    if (!outPassHandles->renderPass) {
        vkcv_log(vkcv::LogLevel::ERROR, "Error: Could not create renderpass");
        return false;
    }

    vkcv::ShaderProgram meshProgram;
    vkcv::shader::GLSLCompiler compiler;

    compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("resources/shaders/shader.vert"),
        [&meshProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
        meshProgram.addShader(shaderStage, path);
    });

    compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("resources/shaders/shader.frag"),
        [&meshProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
        meshProgram.addShader(shaderStage, path);
    });

    const std::vector<vkcv::VertexAttachment> vertexAttachments = meshProgram.getVertexAttachments();
    std::vector<vkcv::VertexBinding> bindings;
    for (size_t i = 0; i < vertexAttachments.size(); i++) {
        bindings.push_back(vkcv::VertexBinding(i, { vertexAttachments[i] }));
    }

    const vkcv::VertexLayout meshVertexLayout(bindings);

    const vkcv::PipelineConfig meshPipelineConfig{
        meshProgram,
        UINT32_MAX,
        UINT32_MAX,
        outPassHandles->renderPass,
        { meshVertexLayout },
        {},
        true };
    outPassHandles->pipeline = core.createGraphicsPipeline(meshPipelineConfig);

    if (!outPassHandles->pipeline) {
        vkcv_log(vkcv::LogLevel::ERROR, "Error: Could not create graphics pipeline");
        return false;
    }

    return true;
}

RenderTargets createRenderTargets(vkcv::Core& core, const uint32_t width, const uint32_t height) {

    RenderTargets targets;

    targets.depthBuffer = core.createImage(
        AppConfig::depthBufferFormat,
        width,
        height,
        1,
        false).getHandle();

    return targets;
}