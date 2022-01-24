
#include <vkcv/Core.hpp>
#include <vkcv/camera/CameraManager.hpp>
#include <vkcv/gui/GUI.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>

struct Particle {
	glm::vec3 position;
	float size;
	glm::vec3 velocity;
	float mass;
	glm::mat4 deformation;
};

float randomFloat(float min, float max) {
	return min + (max - min) * static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

void distributeParticles(Particle *particles, size_t count, const glm::vec3& center, float radius, float mass) {
	float volume = 0.0f;
	
	for (size_t i = 0; i < count; i++) {
		glm::vec3 offset (
				randomFloat(-1.0f, +1.0f),
				randomFloat(-1.0f, +1.0f),
				randomFloat(-1.0f, +1.0f)
		);
		
		if (glm::length(offset) > 0.0f)
			offset = glm::normalize(offset);
		
		offset *= randomFloat(0.0f, radius);
		
		const float size = (radius - glm::length(offset));
		
		particles[i].position = center + offset;
		particles[i].size = size;
		particles[i].velocity = glm::vec3(0.0f);
		
		volume += size;
	}
	
	for (size_t i = 0; i < count; i++) {
		particles[i].mass = (mass * particles[i].size / volume);
	}
}

vkcv::ComputePipelineHandle createComputePipeline(vkcv::Core& core, vkcv::shader::GLSLCompiler& compiler,
												  const std::string& path,
												  const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts) {
	vkcv::ShaderProgram shaderProgram;
	
	compiler.compile(
			vkcv::ShaderStage::COMPUTE,
			path,
			[&shaderProgram](vkcv::ShaderStage stage, const std::filesystem::path& path) {
				shaderProgram.addShader(stage, path);
			}
	);
	
	vkcv::ComputePipelineConfig config {
			shaderProgram,
			{}
	};
	
	return core.createComputePipeline(config);
}

int main(int argc, const char **argv) {
	const char* applicationName = "Wobble Bobble";
	
	uint32_t windowWidth = 800;
	uint32_t windowHeight = 600;
	
	vkcv::Features features;
	features.requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	
	vkcv::Core core = vkcv::Core::create(
			applicationName,
			VK_MAKE_VERSION(0, 0, 1),
			{vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute},
			features
	);
	vkcv::WindowHandle windowHandle = core.createWindow(applicationName, windowWidth, windowHeight, true);
	vkcv::Window& window = core.getWindow(windowHandle);
	vkcv::camera::CameraManager cameraManager(window);
	
	vkcv::gui::GUI gui (core, windowHandle);
	
	cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);
	
	auto swapchainExtent = core.getSwapchain(windowHandle).getExtent();
	
	vkcv::ImageHandle depthBuffer = core.createImage(
			vk::Format::eD32Sfloat,
			swapchainExtent.width,
			swapchainExtent.height
	).getHandle();
	
	vkcv::Buffer<Particle> particles = core.createBuffer<Particle>(
			vkcv::BufferType::STORAGE,
			1024
	);
	
	std::vector<Particle> particles_vec (1024);
	
	distributeParticles(
			particles_vec.data(),
			particles_vec.size(),
			glm::vec3(0.5f),
			0.25f,
			1.0f
	);
	
	particles.fill(particles_vec);
	
	vkcv::Image grid = core.createImage(
			vk::Format::eR32G32B32A32Sfloat,
			64,
			64,
			64,
			false,
			true
	);
	
	/* TODO: clear grid via compute shader?
	std::vector<glm::vec4> grid_vec (grid.getWidth() * grid.getHeight() * grid.getDepth());
	
	for (size_t i = 0; i < grid_vec.size(); i++) {
		grid_vec[i] = glm::vec4(0);
	}
	
	grid.fill(grid_vec.data()); // FIXME: gets limited by staging buffer size...
	 */
	
	vkcv::shader::GLSLCompiler compiler;
	
	vkcv::ComputePipelineHandle transformParticlesToGridPipeline = createComputePipeline(
			core, compiler,
			"shaders/transform_particles_to_grid.comp",
			{}
	);
	
	vkcv::ComputePipelineHandle initParticleVolumesPipeline = createComputePipeline(
			core, compiler,
			"shaders/init_particle_volumes.comp",
			{}
	);
	
	vkcv::ComputePipelineHandle updateGridForcesPipeline = createComputePipeline(
			core, compiler,
			"shaders/update_grid_forces.comp",
			{}
	);
	
	vkcv::ComputePipelineHandle updateGridVelocitiesPipeline = createComputePipeline(
			core, compiler,
			"shaders/update_grid_velocities.comp",
			{}
	);
	
	vkcv::ComputePipelineHandle updateParticleDeformationPipeline = createComputePipeline(
			core, compiler,
			"shaders/update_particle_deformation.comp",
			{}
	);
	
	vkcv::ComputePipelineHandle updateParticleVelocitiesPipeline = createComputePipeline(
			core, compiler,
			"shaders/update_particle_velocities.comp",
			{}
	);
	
	vkcv::ComputePipelineHandle updateParticlePositionsPipeline = createComputePipeline(
			core, compiler,
			"shaders/update_particle_positions.comp",
			{}
	);
	
	vkcv::ShaderProgram gfxProgram;
	
	compiler.compile(
			vkcv::ShaderStage::VERTEX,
			"shaders/particle.vert",
			[&gfxProgram](vkcv::ShaderStage stage, const std::filesystem::path& path) {
				gfxProgram.addShader(stage, path);
			}
	);
	
	compiler.compile(
			vkcv::ShaderStage::GEOMETRY,
			"shaders/particle.geom",
			[&gfxProgram](vkcv::ShaderStage stage, const std::filesystem::path& path) {
				gfxProgram.addShader(stage, path);
			}
	);
	
	compiler.compile(
			vkcv::ShaderStage::FRAGMENT,
			"shaders/particle.frag",
			[&gfxProgram](vkcv::ShaderStage stage, const std::filesystem::path& path) {
				gfxProgram.addShader(stage, path);
			}
	);
	
	vkcv::PassConfig passConfig ({
		vkcv::AttachmentDescription(
				vkcv::AttachmentOperation::STORE,
				vkcv::AttachmentOperation::CLEAR,
				core.getSwapchain(windowHandle).getFormat()
		),
		vkcv::AttachmentDescription(
				vkcv::AttachmentOperation::STORE,
				vkcv::AttachmentOperation::CLEAR,
				vk::Format::eD32Sfloat
		)
	});
	
	vkcv::PassHandle gfxPass = core.createPass(passConfig);
	
	std::vector<vkcv::VertexBinding> vertexBindings;
	vkcv::VertexLayout vertexLayout (vertexBindings);
	
	vkcv::GraphicsPipelineConfig gfxPipelineConfig;
	gfxPipelineConfig.m_ShaderProgram = gfxProgram;
	gfxPipelineConfig.m_Width = windowWidth;
	gfxPipelineConfig.m_Height = windowHeight;
	gfxPipelineConfig.m_PassHandle = gfxPass;
	gfxPipelineConfig.m_VertexLayout = vertexLayout;
	gfxPipelineConfig.m_DescriptorLayouts = {};
	gfxPipelineConfig.m_UseDynamicViewport = true;
	
	vkcv::GraphicsPipelineHandle gfxPipeline = core.createGraphicsPipeline(gfxPipelineConfig);
	
	vkcv::PushConstants pushConstants (0);
	std::vector<vkcv::DrawcallInfo> drawcalls;
	
	bool initializedParticleVolumes = false;
	
	auto start = std::chrono::system_clock::now();
	while (vkcv::Window::hasOpenWindow()) {
		vkcv::Window::pollEvents();
		
		if(window.getHeight() == 0 || window.getWidth() == 0)
			continue;
		
		uint32_t swapchainWidth, swapchainHeight;
		if (!core.beginFrame(swapchainWidth, swapchainHeight,windowHandle)) {
			continue;
		}
		
		if ((swapchainWidth != swapchainExtent.width) || ((swapchainHeight != swapchainExtent.height))) {
			depthBuffer = core.createImage(
					vk::Format::eD32Sfloat,
					swapchainWidth,
					swapchainHeight
			).getHandle();
			
			swapchainExtent.width = swapchainWidth;
			swapchainExtent.height = swapchainHeight;
		}
		
		auto end = std::chrono::system_clock::now();
		auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		
		start = end;
		cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));
		
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);
		
		const uint32_t dispatchSize [3] = { 1, 0, 0 };
		
		core.recordBeginDebugLabel(cmdStream, "TRANSFORM PARTICLES TO GRID", { 0.47f, 0.77f, 0.85f, 1.0f });
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				transformParticlesToGridPipeline,
				dispatchSize,
				{},
				pushConstants
		);
		core.recordEndDebugLabel(cmdStream);
		
		if (!initializedParticleVolumes) {
			core.recordBeginDebugLabel(cmdStream, "INIT PARTICLE VOLUMES", { 0.78f, 0.89f, 0.94f, 1.0f });
			core.recordComputeDispatchToCmdStream(
					cmdStream,
					initParticleVolumesPipeline,
					dispatchSize,
					{},
					pushConstants
			);
			core.recordEndDebugLabel(cmdStream);
			initializedParticleVolumes = true;
		}
		
		core.recordBeginDebugLabel(cmdStream, "UPDATE GRID FORCES", { 0.47f, 0.77f, 0.85f, 1.0f });
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				updateGridForcesPipeline,
				dispatchSize,
				{},
				pushConstants
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "UPDATE GRID VELOCITIES", { 0.47f, 0.77f, 0.85f, 1.0f });
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				updateGridVelocitiesPipeline,
				dispatchSize,
				{},
				pushConstants
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "UPDATE PARTICLE DEFORMATION", { 0.78f, 0.89f, 0.94f, 1.0f });
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				updateParticleDeformationPipeline,
				dispatchSize,
				{},
				pushConstants
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "UPDATE PARTICLE VELOCITIES", { 0.78f, 0.89f, 0.94f, 1.0f });
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				updateParticleVelocitiesPipeline,
				dispatchSize,
				{},
				pushConstants
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "UPDATE PARTICLE POSITIONS", { 0.78f, 0.89f, 0.94f, 1.0f });
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				updateParticlePositionsPipeline,
				dispatchSize,
				{},
				pushConstants
		);
		core.recordEndDebugLabel(cmdStream);
		
		std::vector<vkcv::ImageHandle> renderTargets {
				vkcv::ImageHandle::createSwapchainImageHandle(),
				depthBuffer
		};
		
		core.recordBeginDebugLabel(cmdStream, "RENDER PARTICLES", { 0.13f, 0.20f, 0.22f, 1.0f });
		core.recordDrawcallsToCmdStream(
				cmdStream,
				gfxPass,
				gfxPipeline,
				pushConstants,
				drawcalls,
				renderTargets,
				windowHandle
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		
		gui.beginGUI();
		
		ImGui::Begin("Settings");
		ImGui::End();
		
		gui.endGUI();
		
		core.endFrame(windowHandle);
	}
	
	return 0;
}
