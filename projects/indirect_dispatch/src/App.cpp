#include "App.hpp"
#include "AppConfig.hpp"
#include <chrono>
#include <vkcv/gui/GUI.hpp>
#include <functional>

const char* MotionVectorVisualisationModeLabels[6] = {
		"None",
		"Full resolution",
		"Max tile",
		"Tile neighbourhood max",
		"Min Tile",
		"Tile neighbourhood min"
};

const char* MotionBlurModeLabels[3] = {
		"Default",
		"Disabled",
		"Tile visualisation"
};

App::App() : 
	m_applicationName("Indirect Dispatch"),
	m_windowWidth(AppConfig::defaultWindowWidth),
	m_windowHeight(AppConfig::defaultWindowHeight),
	m_core(vkcv::Core::create(
		m_applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eGraphics ,vk::QueueFlagBits::eCompute , vk::QueueFlagBits::eTransfer },
		{ VK_KHR_SWAPCHAIN_EXTENSION_NAME })),
	m_windowHandle(m_core.createWindow(m_applicationName, m_windowWidth, m_windowHeight, false)),
	m_cameraManager(m_core.getWindow(m_windowHandle)){}

bool App::initialize() {

	if (!loadMeshPass(m_core, &m_meshPass))
		return false;

	if (!loadSkyPass(m_core, &m_skyPass))
		return false;

	if (!loadPrePass(m_core, &m_prePass))
		return false;

	if (!loadSkyPrePass(m_core, &m_skyPrePass))
		return false;

	if (!loadComputePass(m_core, "assets/shaders/gammaCorrection.comp", &m_gammaCorrectionPass))
		return false;

	if (!loadMesh(m_core, "assets/models/cube.gltf", &m_cubeMesh))
		return false;

	if (!loadMesh(m_core, "assets/models/ground.gltf", &m_groundMesh))
		return false;

	if(!loadImage(m_core, "assets/models/grid.png", &m_gridTexture))
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
	
	vkcv::DescriptorWrites meshPassDescriptorWrites;
	meshPassDescriptorWrites.sampledImageWrites = { vkcv::SampledImageDescriptorWrite(0, m_gridTexture) };
	meshPassDescriptorWrites.samplerWrites = { vkcv::SamplerDescriptorWrite(1, m_linearSampler) };
	m_core.writeDescriptorSet(m_meshPass.descriptorSet, meshPassDescriptorWrites);

	return true;
}

void App::run() {

	auto                        frameStartTime = std::chrono::system_clock::now();
	const auto                  appStartTime   = std::chrono::system_clock::now();
	const vkcv::ImageHandle     swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();
	const vkcv::DrawcallInfo    skyDrawcall(m_cubeMesh.mesh, {}, 1);

	vkcv::gui::GUI gui(m_core, m_windowHandle);

	eMotionVectorVisualisationMode  motionVectorVisualisationMode   = eMotionVectorVisualisationMode::None;
	eMotionBlurMode                 motionBlurMode                  = eMotionBlurMode::Default;

	bool    freezeFrame                     = false;
	float   motionBlurTileOffsetLength      = 3;
	float   objectVerticalSpeed             = 5;
	float   objectAmplitude                 = 0;
	float   objectMeanHeight                = 1;
	float   objectRotationSpeedX            = 5;
	float   objectRotationSpeedY            = 5;
	int     cameraShutterSpeedInverse       = 24;
	float   motionVectorVisualisationRange  = 0.008;
	float   motionBlurFastPathThreshold     = 1;

	glm::mat4 viewProjection            = m_cameraManager.getActiveCamera().getMVP();
	glm::mat4 viewProjectionPrevious    = m_cameraManager.getActiveCamera().getMVP();

	struct Object {
		MeshResources meshResources;
		glm::mat4 modelMatrix   = glm::mat4(1.f);
		glm::mat4 mvp           = glm::mat4(1.f);
		glm::mat4 mvpPrevious   = glm::mat4(1.f);
		std::function<void(float, Object&)> modelMatrixUpdate;
	};
	std::vector<Object> sceneObjects;

	Object ground;
	ground.meshResources = m_groundMesh;
	sceneObjects.push_back(ground);

	Object sphere;
	sphere.meshResources = m_cubeMesh;
	sphere.modelMatrixUpdate = [&](float time, Object& obj) {
		const float currentHeight   = objectMeanHeight + objectAmplitude * glm::sin(time * objectVerticalSpeed);
		const glm::mat4 translation = glm::translate(glm::mat4(1), glm::vec3(0, currentHeight, 0));
		const glm::mat4 rotationX   = glm::rotate(glm::mat4(1), objectRotationSpeedX * time, glm::vec3(1, 0, 0));
		const glm::mat4 rotationY   = glm::rotate(glm::mat4(1), objectRotationSpeedY * time, glm::vec3(0, 1, 0));
		obj.modelMatrix             = translation * rotationX * rotationY;
	};
	sceneObjects.push_back(sphere);

	bool spaceWasPressed = false;

	m_core.getWindow(m_windowHandle).e_key.add([&](int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_SPACE) {
			if (action == GLFW_PRESS) {
				if (!spaceWasPressed) {
					freezeFrame = !freezeFrame;
				}
				spaceWasPressed = true;
			}
			else if (action == GLFW_RELEASE) {
				spaceWasPressed = false;
			}
		}
	});

	auto frameEndTime = std::chrono::system_clock::now();

	while (vkcv::Window::hasOpenWindow()) {

		vkcv::Window::pollEvents();

		if (!freezeFrame) {

			frameStartTime          = frameEndTime;
			viewProjectionPrevious  = viewProjection;

			for (Object& obj : sceneObjects) {
				obj.mvpPrevious = obj.mvp;
			}
		}

		if (m_core.getWindow(m_windowHandle).getHeight() == 0 || m_core.getWindow(m_windowHandle).getWidth() == 0)
			continue;

		uint32_t swapchainWidth, swapchainHeight;
		if (!m_core.beginFrame(swapchainWidth, swapchainHeight,m_windowHandle))
			continue;

		const bool hasResolutionChanged = (swapchainWidth != m_windowWidth) || (swapchainHeight != m_windowHeight);
		if (hasResolutionChanged) {
			m_windowWidth  = swapchainWidth;
			m_windowHeight = swapchainHeight;

			m_renderTargets = createRenderTargets(m_core, m_windowWidth, m_windowHeight);
			m_motionBlur.setResolution(m_windowWidth, m_windowHeight);
		}

		if(!freezeFrame)
			frameEndTime = std::chrono::system_clock::now();

		const float microsecondToSecond = 0.000001;
		const float fDeltaTimeSeconds = microsecondToSecond * std::chrono::duration_cast<std::chrono::microseconds>(frameEndTime - frameStartTime).count();

		m_cameraManager.update(fDeltaTimeSeconds);

		const auto      time                = frameEndTime - appStartTime;
		const float     fCurrentTime        = std::chrono::duration_cast<std::chrono::milliseconds>(time).count() * 0.001f;

		// update matrices
		if (!freezeFrame) {

			viewProjection = m_cameraManager.getActiveCamera().getMVP();

			for (Object& obj : sceneObjects) {
				if (obj.modelMatrixUpdate) {
					obj.modelMatrixUpdate(fCurrentTime, obj);
				}
				obj.mvp = viewProjection * obj.modelMatrix;
			}
		}

		const vkcv::CommandStreamHandle cmdStream = m_core.createCommandStream(vkcv::QueueType::Graphics);

		// prepass
		vkcv::PushConstants prepassPushConstants(sizeof(glm::mat4) * 2);

		for (const Object& obj : sceneObjects) {
			glm::mat4 prepassMatrices[2] = { obj.mvp, obj.mvpPrevious };
			prepassPushConstants.appendDrawcall(prepassMatrices);
		}

		const std::vector<vkcv::ImageHandle> prepassRenderTargets = {
			m_renderTargets.motionBuffer,
			m_renderTargets.depthBuffer };

		std::vector<vkcv::DrawcallInfo> prepassSceneDrawcalls;
		for (const Object& obj : sceneObjects) {
			prepassSceneDrawcalls.push_back(vkcv::DrawcallInfo(obj.meshResources.mesh, {}));
		}

		m_core.recordDrawcallsToCmdStream(
			cmdStream,
			m_prePass.renderPass,
			m_prePass.pipeline,
			prepassPushConstants,
			prepassSceneDrawcalls,
			prepassRenderTargets,
			m_windowHandle);

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
			{ skyDrawcall },
			prepassRenderTargets,
			m_windowHandle);

		// main pass
		const std::vector<vkcv::ImageHandle> renderTargets   = { 
			m_renderTargets.colorBuffer, 
			m_renderTargets.depthBuffer };

		vkcv::PushConstants meshPushConstants(2 * sizeof(glm::mat4));
		for (const Object& obj : sceneObjects) {
			glm::mat4 matrices[2] = { obj.mvp, obj.modelMatrix };
			meshPushConstants.appendDrawcall(matrices);
		}

		std::vector<vkcv::DrawcallInfo> forwardSceneDrawcalls;
		for (const Object& obj : sceneObjects) {
			forwardSceneDrawcalls.push_back(vkcv::DrawcallInfo(
				obj.meshResources.mesh, 
				{ vkcv::DescriptorSetUsage(0, m_core.getDescriptorSet(m_meshPass.descriptorSet).vulkanHandle) }));
		}

		m_core.recordDrawcallsToCmdStream(
			cmdStream,
			m_meshPass.renderPass,
			m_meshPass.pipeline,
			meshPushConstants,
			forwardSceneDrawcalls,
			renderTargets,
			m_windowHandle);

		// sky
		vkcv::PushConstants skyPushConstants(sizeof(glm::mat4));
		skyPushConstants.appendDrawcall(viewProjection);

		m_core.recordDrawcallsToCmdStream(
			cmdStream,
			m_skyPass.renderPass,
			m_skyPass.pipeline,
			skyPushConstants,
			{ skyDrawcall },
			renderTargets,
			m_windowHandle);

		// motion blur
		vkcv::ImageHandle motionBlurOutput;

		if (motionVectorVisualisationMode == eMotionVectorVisualisationMode::None) {
			float cameraNear;
			float cameraFar;
			m_cameraManager.getActiveCamera().getNearFar(cameraNear, cameraFar);

			motionBlurOutput = m_motionBlur.render(
				cmdStream,
				m_renderTargets.motionBuffer,
				m_renderTargets.colorBuffer,
				m_renderTargets.depthBuffer,
				motionBlurMode,
				cameraNear,
				cameraFar,
				fDeltaTimeSeconds,
				cameraShutterSpeedInverse,
				motionBlurTileOffsetLength,
				motionBlurFastPathThreshold);
		}
		else {
			motionBlurOutput = m_motionBlur.renderMotionVectorVisualisation(
				cmdStream,
				m_renderTargets.motionBuffer,
				motionVectorVisualisationMode,
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

		ImGui::Checkbox("Freeze frame", &freezeFrame);
		ImGui::InputFloat("Motion tile offset length", &motionBlurTileOffsetLength);
		ImGui::InputFloat("Motion blur fast path threshold", &motionBlurFastPathThreshold);

		ImGui::Combo(
			"Motion blur mode",
			reinterpret_cast<int*>(&motionBlurMode),
			MotionBlurModeLabels,
			static_cast<int>(eMotionBlurMode::OptionCount));

		ImGui::Combo(
			"Debug view",
			reinterpret_cast<int*>(&motionVectorVisualisationMode),
			MotionVectorVisualisationModeLabels,
			static_cast<int>(eMotionVectorVisualisationMode::OptionCount));

		if (motionVectorVisualisationMode != eMotionVectorVisualisationMode::None)
			ImGui::InputFloat("Motion vector visualisation range", &motionVectorVisualisationRange);

		ImGui::InputInt("Camera shutter speed inverse", &cameraShutterSpeedInverse);

		ImGui::InputFloat("Object movement speed",      &objectVerticalSpeed);
		ImGui::InputFloat("Object movement amplitude",  &objectAmplitude);
		ImGui::InputFloat("Object mean height",         &objectMeanHeight);
		ImGui::InputFloat("Object rotation speed X",    &objectRotationSpeedX);
		ImGui::InputFloat("Object rotation speed Y",    &objectRotationSpeedY);

		ImGui::End();
		gui.endGUI();

		m_core.endFrame(m_windowHandle);
	}
}