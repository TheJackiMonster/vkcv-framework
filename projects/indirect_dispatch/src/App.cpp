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

	if (!loadMeshPass(m_core, &m_meshPass))
		return false;

	if (!loadSkyPass(m_core, &m_skyPass))
		return false;

	if (!loadPrePass(m_core, &m_prePass))
		false;

	if (!loadSkyPrePass(m_core, &m_skyPrePass))
		return false;

	if (!loadComputePass(m_core, "resources/shaders/gammaCorrection.comp", &m_gammaCorrectionPass))
		return false;

	if (!loadMesh(m_core, "resources/models/sphere.gltf", & m_sphereMesh))
		return false;

	if (!loadMesh(m_core, "resources/models/cube.gltf", &m_cubeMesh))
		return false;

	if (!loadMesh(m_core, "resources/models/ground.gltf", &m_groundMesh))
		return false;

	if (!m_motionBlur.initialize(&m_core, m_windowWidth, m_windowHeight))
		return false;

	m_linearSampler = m_core.createSampler(
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerMipmapMode::LINEAR,
		vkcv::SamplerAddressMode::CLAMP_TO_EDGE);

	m_renderTargets = createRenderTargets(m_core, m_windowWidth, m_windowHeight);

	const int cameraIndex = m_cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	m_cameraManager.getCamera(cameraIndex).setPosition(glm::vec3(0, 1, -3));
	m_cameraManager.getCamera(cameraIndex).setNearFar(0.1f, 30.f);
	
	return true;
}

void App::run() {

	auto                        frameStartTime = std::chrono::system_clock::now();
	const auto                  appStartTime   = std::chrono::system_clock::now();
	const vkcv::ImageHandle     swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();
	const vkcv::DrawcallInfo    drawcallSphere(m_sphereMesh.mesh, {}, 1);
	const vkcv::DrawcallInfo    drawcallGround(m_groundMesh.mesh, {}, 1);
	const vkcv::DrawcallInfo    cubeDrawcall(m_cubeMesh.mesh, {}, 1);

	vkcv::gui::GUI gui(m_core, m_window);

	enum class eMotionVectorVisualisationMode : int {
		None                    = 0,
		FullResolution          = 1,
		MaxTile                 = 2,
		MaxTileNeighbourhood    = 3,
		OptionCount             = 4 };

	const char* motionVectorVisualisationModeLabels[] = {
		"None",
		"Full resolution",
		"Max tiles",
		"Tile neighbourhood max" };

	eMotionVectorVisualisationMode  motionVectorVisualisationMode   = eMotionVectorVisualisationMode::None;
	eMotionVectorMode               motionBlurMotionMode            = eMotionVectorMode::MaxTileNeighbourhood;

	float   objectVerticalSpeed             = 5;
	float   motionBlurMinVelocity           = 0.001;
	int     cameraShutterSpeedInverse       = 24;
	float   motionVectorVisualisationRange  = 0.008;

	glm::mat4 mvpSpherePrevious         = glm::mat4(1.f);
    glm::mat4 mvpGroundPrevious         = glm::mat4(1.f);
	glm::mat4 viewProjectionPrevious    = m_cameraManager.getActiveCamera().getMVP();
	const glm::mat4 modelMatrixGround   = glm::mat4(1.f);

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
			m_motionBlur.setResolution(m_windowWidth, m_windowHeight);
		}

		auto frameEndTime   = std::chrono::system_clock::now();
		auto deltatime      = std::chrono::duration_cast<std::chrono::microseconds>(frameEndTime - frameStartTime);

		m_cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));
		const glm::mat4 viewProjection = m_cameraManager.getActiveCamera().getMVP();

		const auto      time                = frameEndTime - appStartTime;
		const float     fCurrentTime        = std::chrono::duration_cast<std::chrono::milliseconds>(time).count() * 0.001f;
		const float     currentHeight       = glm::sin(fCurrentTime * objectVerticalSpeed);
		const glm::mat4 modelMatrixSphere   = glm::translate(glm::mat4(1), glm::vec3(0, currentHeight, 0));
		const glm::mat4 mvpSphere           = viewProjection * modelMatrixSphere;
		const glm::mat4 mvpGround           = viewProjection * modelMatrixGround;

		const vkcv::CommandStreamHandle cmdStream = m_core.createCommandStream(vkcv::QueueType::Graphics);

		// prepass
		vkcv::PushConstants prepassPushConstants(sizeof(glm::mat4) * 2);

		glm::mat4 sphereMatricesPrepass[2] = {
			mvpSphere,
			mvpSpherePrevious };
		prepassPushConstants.appendDrawcall(sphereMatricesPrepass);

		glm::mat4 groundMatricesPrepass[2] = {
			mvpGround,
			mvpGroundPrevious };
		prepassPushConstants.appendDrawcall(groundMatricesPrepass);

		const std::vector<vkcv::ImageHandle> prepassRenderTargets = {
			m_renderTargets.motionBuffer,
			m_renderTargets.depthBuffer };

		m_core.recordDrawcallsToCmdStream(
			cmdStream,
			m_prePass.renderPass,
			m_prePass.pipeline,
			prepassPushConstants,
			{ drawcallSphere, drawcallGround },
			prepassRenderTargets);

		// sky prepass
		glm::mat4 skyPrepassMatrices[2] = {
			viewProjection,
			viewProjectionPrevious };
		vkcv::PushConstants skyPrepassPushConstants(sizeof(glm::mat4) * 2);
		skyPrepassPushConstants.appendDrawcall(skyPrepassMatrices);

		m_core.recordDrawcallsToCmdStream(
			cmdStream,
			m_skyPrePass.renderPass,
			m_skyPrePass.pipeline,
			skyPrepassPushConstants,
			{ cubeDrawcall },
			prepassRenderTargets);

		// main pass
		const std::vector<vkcv::ImageHandle> renderTargets   = { 
			m_renderTargets.colorBuffer, 
			m_renderTargets.depthBuffer };

		vkcv::PushConstants meshPushConstants(sizeof(glm::mat4));
		meshPushConstants.appendDrawcall(mvpSphere);
		meshPushConstants.appendDrawcall(mvpGround);

		m_core.recordDrawcallsToCmdStream(
			cmdStream,
			m_meshPass.renderPass,
			m_meshPass.pipeline,
			meshPushConstants,
			{ drawcallSphere, drawcallGround },
			renderTargets);

		// sky
		vkcv::PushConstants skyPushConstants(sizeof(glm::mat4));
		skyPushConstants.appendDrawcall(viewProjection);

		m_core.recordDrawcallsToCmdStream(
			cmdStream,
			m_skyPass.renderPass,
			m_skyPass.pipeline,
			skyPushConstants,
			{ cubeDrawcall },
			renderTargets);

		// motion blur
		vkcv::ImageHandle motionBlurOutput;

		if (motionVectorVisualisationMode == eMotionVectorVisualisationMode::None) {
			const float microsecondToSecond = 0.000001;
			const float fDeltaTimeSeconds = microsecondToSecond * std::chrono::duration_cast<std::chrono::microseconds>(frameEndTime - frameStartTime).count();

			float cameraNear;
			float cameraFar;
			m_cameraManager.getActiveCamera().getNearFar(cameraNear, cameraFar);

			motionBlurOutput = m_motionBlur.render(
				cmdStream,
				m_renderTargets.motionBuffer,
				m_renderTargets.colorBuffer,
				m_renderTargets.depthBuffer,
				motionBlurMotionMode,
				cameraNear,
				cameraFar,
				fDeltaTimeSeconds,
				cameraShutterSpeedInverse,
				motionBlurMinVelocity);
		}
		else {
			eMotionVectorMode debugViewMode;
			if (motionVectorVisualisationMode == eMotionVectorVisualisationMode::FullResolution)
				debugViewMode = eMotionVectorMode::FullResolution;
			else if(motionVectorVisualisationMode == eMotionVectorVisualisationMode::MaxTile)
				debugViewMode = eMotionVectorMode::MaxTile;
			else if (motionVectorVisualisationMode == eMotionVectorVisualisationMode::MaxTileNeighbourhood)
				debugViewMode = eMotionVectorMode::MaxTileNeighbourhood;
			else {
				vkcv_log(vkcv::LogLevel::ERROR, "Unknown eMotionVectorMode enum option");
				debugViewMode = eMotionVectorMode::FullResolution;
			}

			motionBlurOutput = m_motionBlur.renderMotionVectorVisualisation(
				cmdStream,
				m_renderTargets.motionBuffer,
				debugViewMode,
				motionVectorVisualisationRange);
		}

		// gamma correction
		vkcv::DescriptorWrites gammaCorrectionDescriptorWrites;
		gammaCorrectionDescriptorWrites.sampledImageWrites = {
			vkcv::SampledImageDescriptorWrite(0, motionBlurOutput) };
		gammaCorrectionDescriptorWrites.samplerWrites = {
			vkcv::SamplerDescriptorWrite(1, m_linearSampler) };
		gammaCorrectionDescriptorWrites.storageImageWrites = {
			vkcv::StorageImageDescriptorWrite(2, swapchainInput) };

		m_core.writeDescriptorSet(m_gammaCorrectionPass.descriptorSet, gammaCorrectionDescriptorWrites);

		m_core.prepareImageForSampling(cmdStream, motionBlurOutput);
		m_core.prepareImageForStorage (cmdStream, swapchainInput);

		const uint32_t fullScreenImageDispatch[3] = {
			static_cast<uint32_t>((m_windowWidth  + 7) / 8),
			static_cast<uint32_t>((m_windowHeight + 7) / 8),
			static_cast<uint32_t>(1) };

		m_core.recordComputeDispatchToCmdStream(
			cmdStream,
			m_gammaCorrectionPass.pipeline,
			fullScreenImageDispatch,
			{ vkcv::DescriptorSetUsage(0, m_core.getDescriptorSet(m_gammaCorrectionPass.descriptorSet).vulkanHandle) },
			vkcv::PushConstants(0));

		m_core.prepareSwapchainImageForPresent(cmdStream);
		m_core.submitCommandStream(cmdStream);

		gui.beginGUI();
		ImGui::Begin("Settings");

		ImGui::Combo(
			"Debug view",
			reinterpret_cast<int*>(&motionVectorVisualisationMode),
			motionVectorVisualisationModeLabels,
			static_cast<int>(eMotionVectorVisualisationMode::OptionCount));

		if (motionVectorVisualisationMode != eMotionVectorVisualisationMode::None)
			ImGui::InputFloat("Motion vector visualisation range", &motionVectorVisualisationRange);

		ImGui::Combo(
			"Motion blur input",
			reinterpret_cast<int*>(&motionBlurMotionMode),
			MotionVectorModeLabels,
			static_cast<int>(eMotionVectorMode::OptionCount));

		ImGui::InputFloat("Object movement speed", &objectVerticalSpeed);
		ImGui::InputInt("Camera shutter speed inverse", &cameraShutterSpeedInverse);
		ImGui::DragFloat("Motion blur min velocity", &motionBlurMinVelocity, 0.01, 0, 1);

		ImGui::End();
		gui.endGUI();

		m_core.endFrame();

		viewProjectionPrevious  = viewProjection;
		mvpSpherePrevious       = mvpSphere;
		mvpGroundPrevious       = mvpGround;
		frameStartTime          = frameEndTime;
	}
}