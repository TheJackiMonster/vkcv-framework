#include "App.hpp"
#include "AppConfig.hpp"
#include <chrono>
#include <vkcv/gui/GUI.hpp>

App::App() : 
	m_applicationName("Indirect Dispatch"),
	m_windowWidth(AppConfig::defaultWindowWidth),
	m_windowHeight(AppConfig::defaultWindowHeight),
	m_window(vkcv::Window::create(
		m_applicationName,
		m_windowWidth,
		m_windowHeight,
		true)),
	m_core(vkcv::Core::create(
		m_window,
		m_applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eGraphics ,vk::QueueFlagBits::eCompute , vk::QueueFlagBits::eTransfer },
		{},
		{ "VK_KHR_swapchain" })),
	m_cameraManager(m_window){}

bool App::initialize() {

	if (!loadMeshPass(m_core, &m_meshPassHandles))
		return false;

	if (!loadSkyPass(m_core, &m_skyPassHandles))
		return false;

	if (!loadPrePass(m_core, &m_prePassHandles))
		false;

	if (!loadComputePass(m_core, "resources/shaders/gammaCorrection.comp", &m_gammaCorrectionPass))
		return false;

	if (!loadMesh(m_core, "resources/models/sphere.gltf", & m_sphereMesh))
		return false;

	if (!loadMesh(m_core, "resources/models/cube.gltf", &m_cubeMesh))
		return false;

	m_linearSampler = m_core.createSampler(
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerMipmapMode::LINEAR,
		vkcv::SamplerAddressMode::CLAMP_TO_EDGE);

	m_renderTargets = createRenderTargets(m_core, m_windowWidth, m_windowHeight);

	const int cameraIndex = m_cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	m_cameraManager.getCamera(cameraIndex).setPosition(glm::vec3(0, 0, -3));
}

void App::run() {

	auto                        frameStartTime = std::chrono::system_clock::now();
	const vkcv::ImageHandle     swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();
	const vkcv::DrawcallInfo    sphereDrawcall(m_sphereMesh.mesh, {}, 1);
	const vkcv::DrawcallInfo    cubeDrawcall(m_cubeMesh.mesh, {}, 1);

	vkcv::gui::GUI gui(m_core, m_window);

	bool drawMotionVectors = false;

	glm::mat4 previousFrameViewProjection = m_cameraManager.getActiveCamera().getMVP();

	while (m_window.isWindowOpen()) {
		vkcv::Window::pollEvents();

		if (m_window.getHeight() == 0 || m_window.getWidth() == 0)
			continue;

		uint32_t swapchainWidth, swapchainHeight;
		if (!m_core.beginFrame(swapchainWidth, swapchainHeight))
			continue;

		const bool hasResolutionChanged = (swapchainWidth != m_windowWidth) || (swapchainHeight != m_windowHeight);
		if (hasResolutionChanged) {
			m_windowWidth  = swapchainWidth;
			m_windowHeight = swapchainHeight;

			m_renderTargets = createRenderTargets(m_core, m_windowWidth, m_windowHeight);
		}

		auto frameEndTime   = std::chrono::system_clock::now();
		auto deltatime      = std::chrono::duration_cast<std::chrono::microseconds>(frameEndTime - frameStartTime);
		frameStartTime      = frameEndTime;

		m_cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));
		const glm::mat4 viewProjection = m_cameraManager.getActiveCamera().getMVP();

		const vkcv::CommandStreamHandle cmdStream = m_core.createCommandStream(vkcv::QueueType::Graphics);

		// prepass
		glm::mat4 prepassMatrices[2] = {
			viewProjection,
			previousFrameViewProjection };
		vkcv::PushConstants prepassPushConstants(sizeof(glm::mat4)*2);
		prepassPushConstants.appendDrawcall(prepassMatrices);

		const std::vector<vkcv::ImageHandle> prepassRenderTargets = {
			m_renderTargets.motionBuffer,
			m_renderTargets.depthBuffer };

		m_core.recordDrawcallsToCmdStream(
			cmdStream,
			m_prePassHandles.renderPass,
			m_prePassHandles.pipeline,
			prepassPushConstants,
			{ sphereDrawcall },
			prepassRenderTargets);

		// main pass
		const std::vector<vkcv::ImageHandle> renderTargets   = { 
			m_renderTargets.colorBuffer, 
			m_renderTargets.depthBuffer };

		vkcv::PushConstants meshPushConstants(sizeof(glm::mat4));
		meshPushConstants.appendDrawcall(viewProjection);

		m_core.recordDrawcallsToCmdStream(
			cmdStream,
			m_meshPassHandles.renderPass,
			m_meshPassHandles.pipeline,
			meshPushConstants,
			{ sphereDrawcall },
			renderTargets);

		// sky
		vkcv::PushConstants skyPushConstants(sizeof(glm::mat4));
		skyPushConstants.appendDrawcall(viewProjection);

		m_core.recordDrawcallsToCmdStream(
			cmdStream,
			m_skyPassHandles.renderPass,
			m_skyPassHandles.pipeline,
			skyPushConstants,
			{ cubeDrawcall },
			renderTargets);

		// gamma correction
		vkcv::ImageHandle gammaCorrectionInput = m_renderTargets.colorBuffer;
		if (drawMotionVectors) {
			gammaCorrectionInput = m_renderTargets.motionBuffer;
		}

		vkcv::DescriptorWrites gammaCorrectionDescriptorWrites;
		gammaCorrectionDescriptorWrites.sampledImageWrites = {
			vkcv::SampledImageDescriptorWrite(0, gammaCorrectionInput) };
		gammaCorrectionDescriptorWrites.samplerWrites = {
			vkcv::SamplerDescriptorWrite(1, m_linearSampler) };
		gammaCorrectionDescriptorWrites.storageImageWrites = {
			vkcv::StorageImageDescriptorWrite(2, swapchainInput) };

		m_core.writeDescriptorSet(m_gammaCorrectionPass.descriptorSet, gammaCorrectionDescriptorWrites);

		m_core.prepareImageForSampling(cmdStream, gammaCorrectionInput);
		m_core.prepareImageForStorage (cmdStream, swapchainInput);

		uint32_t gammaCorrectionDispatch[3] = {
			static_cast<uint32_t>((m_windowWidth  + 7) / 8),
			static_cast<uint32_t>((m_windowHeight + 7) / 8),
			static_cast<uint32_t>(1) };

		m_core.recordComputeDispatchToCmdStream(
			cmdStream,
			m_gammaCorrectionPass.pipeline,
			gammaCorrectionDispatch,
			{ vkcv::DescriptorSetUsage(0, m_core.getDescriptorSet(m_gammaCorrectionPass.descriptorSet).vulkanHandle) },
			vkcv::PushConstants(0));

		m_core.prepareSwapchainImageForPresent(cmdStream);
		m_core.submitCommandStream(cmdStream);

		gui.beginGUI();
		ImGui::Begin("Settings");
		ImGui::Checkbox("View motion vectors", &drawMotionVectors);
		ImGui::End();
		gui.endGUI();

		m_core.endFrame();

		previousFrameViewProjection = viewProjection;
	}
}