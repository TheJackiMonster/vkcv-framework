#include <iostream>
#include <vkcv/Core.hpp>
#include <vkcv/Image.hpp>
#include <vkcv/Interpolation.hpp>
#include <vkcv/Pass.hpp>
#include <vkcv/Sampler.hpp>
#include <vkcv/camera/CameraManager.hpp>

#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>

#include <vkcv/geometry/Cuboid.hpp>

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

	std::vector<vkcv::VertexBinding> bindings = vkcv::createVertexBindings(
			firstMeshProgram.getVertexAttachments()
	);
	
	const vkcv::VertexLayout firstMeshLayout { bindings };

	// since we only use one descriptor set (namely, desc set 0), directly address it
	// recreate copies of the bindings and the handles (to check whether they are properly reused instead of actually recreated)
	const vkcv::DescriptorBindings& set0Bindings = firstMeshProgram.getReflectedDescriptors().at(0);

	vkcv::DescriptorSetLayoutHandle descriptorSetLayout = core.createDescriptorSetLayout(set0Bindings);
	vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorSetLayout);
	
	vkcv::GraphicsPipelineHandle firstMeshPipeline = core.createGraphicsPipeline(
			vkcv::GraphicsPipelineConfig(
					firstMeshProgram,
					firstMeshPass,
					{ firstMeshLayout },
					{ descriptorSetLayout }
			)
	);
	
	if (!firstMeshPipeline) {
		std::cerr << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	vkcv::asset::Texture tex = vkcv::asset::loadTexture("assets/cube/boards2_vcyc_jpg.jpg");
	
	if (tex.data.empty()) {
		std::cerr << "Error. No texture found. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
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
	
	vkcv::geometry::Cuboid cube (glm::vec3(0), 1.0f);
	
	vkcv::InstanceDrawcall drawcall (cube.generateVertexData(core));
	drawcall.useDescriptorSet(0, descriptorSet);

    vkcv::camera::CameraManager cameraManager(window);
	
    auto camHandle0 = cameraManager.addCamera(vkcv::camera::ControllerType::NONE);
	auto camHandle1 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	auto camHandle2 = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);
	
	cameraManager.getCamera(camHandle1).setPosition(glm::vec3(0, 0, -3));
	cameraManager.getCamera(camHandle2).setPosition(glm::vec3(0, 0, -3));
	
	auto interp = vkcv::linearInterpolation<glm::vec3, float>();
	
	interp.add( 0.0f, glm::vec3(+5, +5, -5));
	interp.add( 2.0f, glm::vec3(+0, +5, -5));
	interp.add( 4.0f, glm::vec3(+0, -3, -3));
	interp.add( 6.0f, glm::vec3(+3, +0, -6));
	interp.add( 8.0f, glm::vec3(+5, +5, +5));
	interp.add(10.0f, glm::vec3(+5, +5, -5));
	
	core.run([&](const vkcv::WindowHandle &windowHandle, double t, double dt,
				 uint32_t swapchainWidth, uint32_t swapchainHeight) {
		if ((!depthBuffer) ||
			(swapchainWidth != core.getImageWidth(depthBuffer)) ||
			(swapchainHeight != core.getImageHeight(depthBuffer))) {
			depthBuffer = core.createImage(
					vk::Format::eD32Sfloat,
					vkcv::ImageConfig(
							swapchainWidth,
							swapchainHeight
					)
			);
		}
		
		cameraManager.update(dt);
		cameraManager.getCamera(camHandle0).setPosition(
				interp(static_cast<float>(fmod(t, 10.0)))
		);
		
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
	
	core.getContext().getDevice().waitIdle();
	
	return 0;
}
