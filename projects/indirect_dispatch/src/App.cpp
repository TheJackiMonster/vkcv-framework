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

	if(!loadComputePass(m_core, "resources/shaders/motionBlurDummy.comp", &m_motionBlurDummyPass))
		return false;

	if (!loadComputePass(m_core, "resources/shaders/motionVectorMax.comp", &m_motionVectorMaxPass))
		return false;

	if (!loadComputePass(m_core, "resources/shaders/motionVectorMaxNeighbourhood.comp", &m_motionVectorMaxNeighbourhoodPass))
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
	const auto                  appStartTime   = std::chrono::system_clock::now();
	const vkcv::ImageHandle     swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();
	const vkcv::DrawcallInfo    sphereDrawcall(m_sphereMesh.mesh, {}, 1);
	const vkcv::DrawcallInfo    cubeDrawcall(m_cubeMesh.mesh, {}, 1);

	vkcv::gui::GUI gui(m_core, m_window);
	enum class eDebugView : int { 
		None                                = 0, 
		MotionVector                        = 1, 
		MotionVectorMaxTile                 = 2, 
		MotionVectorMaxTileNeighbourhood    = 3,
		OptionCount                         = 4 };

	const char* debugViewLabels[] = {
		"None",
		"Motion vectors",
		"Motion vector max tiles",
		"Motion vector tile neighbourhood max" };

	enum class eMotionBlurInput : int {
		MotionVector                        = 0,
		MotionVectorMaxTile                 = 1,
		MotionVectorMaxTileNeighbourhood    = 2,
		OptionCount                         = 3 };

	const char* motionInputLabels[] = {
		"Motion vectors",
		"Motion vector max tiles",
		"Motion vector tile neighbourhood max" };

	eDebugView          debugView       = eDebugView::None;
	eMotionBlurInput    motionBlurInput = eMotionBlurInput::MotionVectorMaxTileNeighbourhood;

	float   objectVerticalSpeed         = 0.005;
	float   motionBlurMinVelocity       = 0.001;
	int     cameraShutterSpeedInverse   = 30;

	glm::mat4 mvpPrevious               = glm::mat4(1.f);
	glm::mat4 viewProjectionPrevious    = m_cameraManager.getActiveCamera().getMVP();

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

		m_cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));
		const glm::mat4 viewProjection = m_cameraManager.getActiveCamera().getMVP();

		const auto      time            = frameEndTime - appStartTime;
		const float     fCurrentTime    = std::chrono::duration_cast<std::chrono::milliseconds>(time).count();
		const float     currentHeight   = glm::sin(fCurrentTime * objectVerticalSpeed);
		const glm::mat4 modelMatrix     = glm::translate(glm::mat4(1), glm::vec3(0, currentHeight, 0));
		const glm::mat4 mvp             = viewProjection * modelMatrix;

		const vkcv::CommandStreamHandle cmdStream = m_core.createCommandStream(vkcv::QueueType::Graphics);

		// prepass
		glm::mat4 prepassMatrices[2] = {
			mvp,
			mvpPrevious };
		vkcv::PushConstants prepassPushConstants(sizeof(glm::mat4)*2);
		prepassPushConstants.appendDrawcall(prepassMatrices);

		const std::vector<vkcv::ImageHandle> prepassRenderTargets = {
			m_renderTargets.motionBuffer,
			m_renderTargets.depthBuffer };

		m_core.recordDrawcallsToCmdStream(
			cmdStream,
			m_prePass.renderPass,
			m_prePass.pipeline,
			prepassPushConstants,
			{ sphereDrawcall },
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

		// motion vector max tiles
		vkcv::DescriptorWrites motionVectorMaxTilesDescriptorWrites;
		motionVectorMaxTilesDescriptorWrites.sampledImageWrites = {
			vkcv::SampledImageDescriptorWrite(0, m_renderTargets.motionBuffer) };
		motionVectorMaxTilesDescriptorWrites.samplerWrites = {
			vkcv::SamplerDescriptorWrite(1, m_linearSampler) };
		motionVectorMaxTilesDescriptorWrites.storageImageWrites = {
			vkcv::StorageImageDescriptorWrite(2, m_renderTargets.motionMax)};

		m_core.writeDescriptorSet(m_motionVectorMaxPass.descriptorSet, motionVectorMaxTilesDescriptorWrites);

		m_core.prepareImageForSampling(cmdStream, m_renderTargets.motionBuffer);
		m_core.prepareImageForStorage(cmdStream, m_renderTargets.motionMax);

		const uint32_t motionTileDispatchCounts[3] = {
			(m_core.getImageWidth( m_renderTargets.motionMax) + 7) / 8,
			(m_core.getImageHeight(m_renderTargets.motionMax) + 7) / 8,
			1 };

		m_core.recordComputeDispatchToCmdStream(
			cmdStream,
			m_motionVectorMaxPass.pipeline,
			motionTileDispatchCounts,
			{ vkcv::DescriptorSetUsage(0, m_core.getDescriptorSet(m_motionVectorMaxPass.descriptorSet).vulkanHandle) },
			vkcv::PushConstants(0));

		// motion vector max neighbourhood
		vkcv::DescriptorWrites motionVectorMaxNeighbourhoodDescriptorWrites;
		motionVectorMaxNeighbourhoodDescriptorWrites.sampledImageWrites = {
			vkcv::SampledImageDescriptorWrite(0, m_renderTargets.motionMax) };
		motionVectorMaxNeighbourhoodDescriptorWrites.samplerWrites = {
			vkcv::SamplerDescriptorWrite(1, m_linearSampler) };
		motionVectorMaxNeighbourhoodDescriptorWrites.storageImageWrites = {
			vkcv::StorageImageDescriptorWrite(2, m_renderTargets.motionMaxNeighbourhood) };

		m_core.writeDescriptorSet(m_motionVectorMaxNeighbourhoodPass.descriptorSet, motionVectorMaxNeighbourhoodDescriptorWrites);

		m_core.prepareImageForSampling(cmdStream, m_renderTargets.motionMax);
		m_core.prepareImageForStorage(cmdStream, m_renderTargets.motionMaxNeighbourhood);

		m_core.recordComputeDispatchToCmdStream(
			cmdStream,
			m_motionVectorMaxNeighbourhoodPass.pipeline,
			motionTileDispatchCounts,
			{ vkcv::DescriptorSetUsage(0, m_core.getDescriptorSet(m_motionVectorMaxNeighbourhoodPass.descriptorSet).vulkanHandle) },
			vkcv::PushConstants(0));

		// main pass
		const std::vector<vkcv::ImageHandle> renderTargets   = { 
			m_renderTargets.colorBuffer, 
			m_renderTargets.depthBuffer };

		vkcv::PushConstants meshPushConstants(sizeof(glm::mat4));
		meshPushConstants.appendDrawcall(mvp);

		m_core.recordDrawcallsToCmdStream(
			cmdStream,
			m_meshPass.renderPass,
			m_meshPass.pipeline,
			meshPushConstants,
			{ sphereDrawcall },
			renderTargets);

		// sky
		vkcv::PushConstants skyPushConstants(sizeof(glm::mat4));
		skyPushConstants.appendDrawcall(viewProjectionPrevious);

		m_core.recordDrawcallsToCmdStream(
			cmdStream,
			m_skyPass.renderPass,
			m_skyPass.pipeline,
			skyPushConstants,
			{ cubeDrawcall },
			renderTargets);

		// motion blur
		vkcv::ImageHandle motionBuffer;
		if (motionBlurInput == eMotionBlurInput::MotionVector)
			motionBuffer = m_renderTargets.motionBuffer;
		else if (motionBlurInput == eMotionBlurInput::MotionVectorMaxTile)
			motionBuffer = m_renderTargets.motionMax;
		else if (motionBlurInput == eMotionBlurInput::MotionVectorMaxTileNeighbourhood)
			motionBuffer = m_renderTargets.motionMaxNeighbourhood;
		else {
			vkcv_log(vkcv::LogLevel::ERROR, "Unknown eMotionInput enum value");
			motionBuffer = m_renderTargets.motionBuffer;
		}

		vkcv::DescriptorWrites motionBlurDescriptorWrites;
		motionBlurDescriptorWrites.sampledImageWrites = {
			vkcv::SampledImageDescriptorWrite(0, m_renderTargets.colorBuffer),
			vkcv::SampledImageDescriptorWrite(1, motionBuffer) };
		motionBlurDescriptorWrites.samplerWrites = {
			vkcv::SamplerDescriptorWrite(2, m_linearSampler) };
		motionBlurDescriptorWrites.storageImageWrites = {
			vkcv::StorageImageDescriptorWrite(3, m_renderTargets.motionBlurOutput) };

		m_core.writeDescriptorSet(m_motionBlurDummyPass.descriptorSet, motionBlurDescriptorWrites);

		uint32_t fullScreenImageDispatch[3] = {
			static_cast<uint32_t>((m_windowWidth + 7) / 8),
			static_cast<uint32_t>((m_windowHeight + 7) / 8),
			static_cast<uint32_t>(1) };

		m_core.prepareImageForStorage(cmdStream, m_renderTargets.motionBlurOutput);
		m_core.prepareImageForSampling(cmdStream, m_renderTargets.colorBuffer);
		m_core.prepareImageForSampling(cmdStream, motionBuffer);

		const float microsecondToSecond     = 0.000001;
		const float fDeltatimeSeconds       = microsecondToSecond * std::chrono::duration_cast<std::chrono::microseconds>(frameEndTime - frameStartTime).count();
		const float motionBlurMotionFactor  = 1 / (fDeltatimeSeconds * cameraShutterSpeedInverse);

		vkcv::PushConstants motionBlurPushConstants(sizeof(float) * 2);

		float motionBlurConstantData[2] = { motionBlurMotionFactor, motionBlurMinVelocity };

		motionBlurPushConstants.appendDrawcall(motionBlurConstantData);

		m_core.recordComputeDispatchToCmdStream(
			cmdStream,
			m_motionBlurDummyPass.pipeline,
			fullScreenImageDispatch,
			{ vkcv::DescriptorSetUsage(0, m_core.getDescriptorSet(m_motionBlurDummyPass.descriptorSet).vulkanHandle) },
			motionBlurPushConstants);

		// gamma correction
		vkcv::ImageHandle gammaCorrectionInput;
		if (debugView == eDebugView::None)
			gammaCorrectionInput = m_renderTargets.motionBlurOutput;
		else if (debugView == eDebugView::MotionVector)
			gammaCorrectionInput = m_renderTargets.motionBuffer;
		else if (debugView == eDebugView::MotionVectorMaxTile)
			gammaCorrectionInput = m_renderTargets.motionMax;
		else if (debugView == eDebugView::MotionVectorMaxTileNeighbourhood)
			gammaCorrectionInput = m_renderTargets.motionMaxNeighbourhood;
		else {
			vkcv_log(vkcv::LogLevel::ERROR, "Unknown eDebugView enum value");
			gammaCorrectionInput = m_renderTargets.motionBlurOutput;
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
			reinterpret_cast<int*>(&debugView),
			debugViewLabels,
			static_cast<int>(eDebugView::OptionCount));

		ImGui::Combo(
			"Motion blur input",
			reinterpret_cast<int*>(&motionBlurInput),
			motionInputLabels,
			static_cast<int>(eMotionBlurInput::OptionCount));

		ImGui::InputFloat("Object movement speed", &objectVerticalSpeed);
		ImGui::InputInt("Camera shutter speed inverse", &cameraShutterSpeedInverse);
		ImGui::DragFloat("Motion blur min velocity", &motionBlurMinVelocity, 0.01, 0, 1);

		ImGui::End();
		gui.endGUI();

		m_core.endFrame();

		viewProjectionPrevious  = viewProjection;
		mvpPrevious             = mvp;
		frameStartTime          = frameEndTime;
	}
}