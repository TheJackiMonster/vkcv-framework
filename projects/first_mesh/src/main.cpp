#include <iostream>
#include <vkcv/Buffer.hpp>
#include <vkcv/Core.hpp>
#include <vkcv/Image.hpp>
#include <vkcv/Pass.hpp>
#include <vkcv/Sampler.hpp>
#include <vkcv/camera/CameraManager.hpp>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>

int main(int argc, const char** argv) {
	const std::string applicationName = "First Mesh";

	vkcv::Core core = vkcv::Core::create(
		applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eGraphics ,vk::QueueFlagBits::eCompute , vk::QueueFlagBits::eTransfer },
		{ VK_KHR_SWAPCHAIN_EXTENSION_NAME }
	);

	vkcv::WindowHandle windowHandle = core.createWindow(applicationName, 800, 600, true);
	vkcv::Window& window = core.getWindow(windowHandle);

	vkcv::asset::Scene mesh;

	const char* path = argc > 1 ? argv[1] : "assets/cube/cube.gltf";
	int result = vkcv::asset::loadScene(path, mesh);

	if (result == 1) {
		std::cout << "Mesh loading successful!" << std::endl;
	} else {
		std::cerr << "Mesh loading failed: " << result << std::endl;
		return 1;
	}

	assert(!mesh.vertexGroups.empty());
	auto vertexBuffer = vkcv::buffer<uint8_t>(
			core,
			vkcv::BufferType::VERTEX,
			mesh.vertexGroups[0].vertexBuffer.data.size(),
			vkcv::BufferMemoryType::DEVICE_LOCAL
	);
	
	vertexBuffer.fill(mesh.vertexGroups[0].vertexBuffer.data);

	auto indexBuffer = vkcv::buffer<uint8_t>(
			core,
			vkcv::BufferType::INDEX,
			mesh.vertexGroups[0].indexBuffer.data.size(),
			vkcv::BufferMemoryType::DEVICE_LOCAL
	);
	
	indexBuffer.fill(mesh.vertexGroups[0].indexBuffer.data);

	vkcv::PassHandle firstMeshPass = vkcv::passSwapchain(
			core,
			window.getSwapchain(),
			{ vk::Format::eUndefined, vk::Format::eD32Sfloat }
	);

	if (!firstMeshPass) {
		std::cerr << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	vkcv::ShaderProgram firstMeshProgram;
	vkcv::shader::GLSLCompiler compiler;
	
	compiler.compileProgram(firstMeshProgram, {
		{ vkcv::ShaderStage::VERTEX, "assets/shaders/shader.vert" },
		{ vkcv::ShaderStage::FRAGMENT, "assets/shaders/shader.frag" }
	}, nullptr);
	
	const auto vertexBufferBindings = vkcv::asset::loadVertexBufferBindings(
			mesh.vertexGroups[0].vertexBuffer.attributes,
			vertexBuffer.getHandle(),
			{
					vkcv::asset::PrimitiveType::POSITION,
					vkcv::asset::PrimitiveType::NORMAL,
					vkcv::asset::PrimitiveType::TEXCOORD_0
			}
	);

	std::vector<vkcv::VertexBinding> bindings = vkcv::createVertexBindings(
			firstMeshProgram.getVertexAttachments()
	);
	
	const vkcv::VertexLayout firstMeshLayout { bindings };

	// since we only use one descriptor set (namely, desc set 0), directly address it
	// recreate copies of the bindings and the handles (to check whether they are properly reused instead of actually recreated)
	const vkcv::DescriptorBindings& set0Bindings = firstMeshProgram.getReflectedDescriptors().at(0);
    auto set0BindingsExplicitCopy = set0Bindings;

	vkcv::DescriptorSetLayoutHandle setLayoutHandle = core.createDescriptorSetLayout(set0Bindings);
	vkcv::DescriptorSetLayoutHandle setLayoutHandleCopy = core.createDescriptorSetLayout(set0BindingsExplicitCopy);

	vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(setLayoutHandle);
	
	vkcv::GraphicsPipelineHandle firstMeshPipeline = core.createGraphicsPipeline(
			vkcv::GraphicsPipelineConfig(
					firstMeshProgram,
					firstMeshPass,
					{ firstMeshLayout },
					{ setLayoutHandle }
			)
	);
	
	if (!firstMeshPipeline) {
		std::cerr << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	if (mesh.textures.empty()) {
		std::cerr << "Error. No textures found. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	vkcv::asset::Texture &tex = mesh.textures[0];
	vkcv::Image texture = vkcv::image(core, vk::Format::eR8G8B8A8Srgb, tex.w, tex.h);
	texture.fill(tex.data.data());
	
	{
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);
		texture.recordMipChainGeneration(cmdStream, core.getDownsampler());
		core.submitCommandStream(cmdStream, false);
	}

	vkcv::SamplerHandle sampler = vkcv::samplerLinear(core);

	vkcv::DescriptorWrites setWrites;
	setWrites.writeSampledImage(0, texture.getHandle());
	setWrites.writeSampler(1, sampler);

	core.writeDescriptorSet(descriptorSet, setWrites);
	
	vkcv::ImageHandle depthBuffer;

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();
	
	vkcv::VertexData vertexData (vertexBufferBindings);
	vertexData.setIndexBuffer(indexBuffer.getHandle());
	vertexData.setCount(mesh.vertexGroups[0].numIndices);
	
	vkcv::InstanceDrawcall drawcall (vertexData);
	drawcall.useDescriptorSet(0, descriptorSet);

    vkcv::camera::CameraManager cameraManager(window);
    uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	
	cameraManager.getCamera(camIndex0).setPosition(glm::vec3(0, 0, -3));
	
	core.run([&](const vkcv::WindowHandle &windowHandle, double t, double dt,
				 uint32_t swapchainWidth, uint32_t swapchainHeight) {
		if ((!depthBuffer) ||
			(swapchainWidth != core.getImageWidth(depthBuffer)) ||
			(swapchainHeight != core.getImageHeight(depthBuffer))) {
			depthBuffer = core.createImage(
					vk::Format::eD32Sfloat,
					swapchainWidth,
					swapchainHeight
			);
		}
		
		cameraManager.update(dt);
        glm::mat4 mvp = cameraManager.getActiveCamera().getMVP();

		vkcv::PushConstants pushConstants = vkcv::pushConstants<glm::mat4>();
		pushConstants.appendDrawcall(mvp);

		const std::vector<vkcv::ImageHandle> renderTargets = { swapchainInput, depthBuffer };
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		core.recordDrawcallsToCmdStream(
			cmdStream,
			firstMeshPipeline,
			pushConstants,
			{ drawcall },
			renderTargets,
			windowHandle
		);
		
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
	});
	
	return 0;
}
