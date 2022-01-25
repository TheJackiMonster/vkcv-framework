
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
												  std::vector<vkcv::DescriptorSetHandle>& descriptorSets) {
	vkcv::ShaderProgram shaderProgram;
	
	compiler.compile(
			vkcv::ShaderStage::COMPUTE,
			path,
			[&shaderProgram](vkcv::ShaderStage stage, const std::filesystem::path& path) {
				shaderProgram.addShader(stage, path);
			}
	);
	
	const auto& descriptors = shaderProgram.getReflectedDescriptors();
	
	size_t count = 0;
	
	for (const auto& descriptor : descriptors) {
		if (descriptor.first >= count) {
			count = (descriptor.first + 1);
		}
	}
	
	std::vector<vkcv::DescriptorSetLayoutHandle> descriptorSetLayouts;
	
	descriptorSetLayouts.resize(count);
	descriptorSets.resize(count);
	
	for (const auto& descriptor : descriptors) {
		descriptorSetLayouts[descriptor.first] = core.createDescriptorSetLayout(descriptor.second);
		descriptorSets[descriptor.first] = core.createDescriptorSet(descriptorSetLayouts[descriptor.first]);
	}
	
	vkcv::ComputePipelineConfig config {
			shaderProgram,
			descriptorSetLayouts
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
	
	grid.switchLayout(vk::ImageLayout::eGeneral);
	
	vkcv::Image tmpGrid = core.createImage(
			vk::Format::eR32G32B32A32Sfloat,
			64,
			64,
			64,
			false,
			true
	);
	
	tmpGrid.switchLayout(vk::ImageLayout::eGeneral);
	
	/* TODO: clear grid via compute shader?
	std::vector<glm::vec4> grid_vec (grid.getWidth() * grid.getHeight() * grid.getDepth());
	
	for (size_t i = 0; i < grid_vec.size(); i++) {
		grid_vec[i] = glm::vec4(0);
	}
	
	grid.fill(grid_vec.data()); // FIXME: gets limited by staging buffer size...
	 */
	
	vkcv::SamplerHandle gridSampler = core.createSampler(
			vkcv::SamplerFilterType::LINEAR,
			vkcv::SamplerFilterType::LINEAR,
			vkcv::SamplerMipmapMode::NEAREST,
			vkcv::SamplerAddressMode::REPEAT
	);
	
	vkcv::shader::GLSLCompiler compiler;
	
	std::vector<vkcv::DescriptorSetHandle> transformParticlesToGridSets;
	vkcv::ComputePipelineHandle transformParticlesToGridPipeline = createComputePipeline(
			core, compiler,
			"shaders/transform_particles_to_grid.comp",
			transformParticlesToGridSets
	);
	
	{
		vkcv::DescriptorWrites writes;
		writes.storageBufferWrites.push_back(vkcv::BufferDescriptorWrite(0, particles.getHandle()));
		writes.storageImageWrites.push_back(vkcv::StorageImageDescriptorWrite(1, grid.getHandle()));
		core.writeDescriptorSet(transformParticlesToGridSets[0], writes);
	}
	
	std::vector<vkcv::DescriptorSetHandle> initParticleVolumesSets;
	vkcv::ComputePipelineHandle initParticleVolumesPipeline = createComputePipeline(
			core, compiler,
			"shaders/init_particle_volumes.comp",
			initParticleVolumesSets
	);
	
	std::vector<vkcv::DescriptorSetHandle> updateGridForcesSets;
	vkcv::ComputePipelineHandle updateGridForcesPipeline = createComputePipeline(
			core, compiler,
			"shaders/update_grid_forces.comp",
			updateGridForcesSets
	);
	
	std::vector<vkcv::DescriptorSetHandle> updateGridVelocitiesSets;
	vkcv::ComputePipelineHandle updateGridVelocitiesPipeline = createComputePipeline(
			core, compiler,
			"shaders/update_grid_velocities.comp",
			updateGridVelocitiesSets
	);
	
	{
		vkcv::DescriptorWrites writes;
		writes.sampledImageWrites.push_back(vkcv::SampledImageDescriptorWrite(0, grid.getHandle()));
		writes.samplerWrites.push_back(vkcv::SamplerDescriptorWrite(1, gridSampler));
		writes.storageImageWrites.push_back(vkcv::StorageImageDescriptorWrite(2, tmpGrid.getHandle()));
		core.writeDescriptorSet(updateGridVelocitiesSets[0], writes);
	}
	
	std::vector<vkcv::DescriptorSetHandle> updateParticleDeformationSets;
	vkcv::ComputePipelineHandle updateParticleDeformationPipeline = createComputePipeline(
			core, compiler,
			"shaders/update_particle_deformation.comp",
			updateParticleDeformationSets
	);
	
	std::vector<vkcv::DescriptorSetHandle> updateParticleVelocitiesSets;
	vkcv::ComputePipelineHandle updateParticleVelocitiesPipeline = createComputePipeline(
			core, compiler,
			"shaders/update_particle_velocities.comp",
			updateParticleVelocitiesSets
	);
	
	{
		vkcv::DescriptorWrites writes;
		writes.storageBufferWrites.push_back(vkcv::BufferDescriptorWrite(0, particles.getHandle()));
		writes.sampledImageWrites.push_back(vkcv::SampledImageDescriptorWrite(1, tmpGrid.getHandle()));
		writes.samplerWrites.push_back(vkcv::SamplerDescriptorWrite(2, gridSampler));
		core.writeDescriptorSet(updateParticleVelocitiesSets[0], writes);
	}
	
	std::vector<vkcv::DescriptorSetHandle> updateParticlePositionsSets;
	vkcv::ComputePipelineHandle updateParticlePositionsPipeline = createComputePipeline(
			core, compiler,
			"shaders/update_particle_positions.comp",
			updateParticlePositionsSets
	);
	
	{
		vkcv::DescriptorWrites writes;
		writes.storageBufferWrites.push_back(vkcv::BufferDescriptorWrite(0, particles.getHandle()));
		core.writeDescriptorSet(updateParticlePositionsSets[0], writes);
	}
	
	vkcv::ShaderProgram gfxProgram;
	
	compiler.compile(
			vkcv::ShaderStage::VERTEX,
			"shaders/particle.vert",
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
	
	vkcv::DescriptorSetLayoutHandle gfxSetLayout = core.createDescriptorSetLayout(
			gfxProgram.getReflectedDescriptors().at(0)
	);
	
	vkcv::DescriptorSetHandle gfxSet = core.createDescriptorSet(gfxSetLayout);
	
	{
		vkcv::DescriptorWrites writes;
		writes.storageBufferWrites.push_back(vkcv::BufferDescriptorWrite(0, particles.getHandle()));
		core.writeDescriptorSet(gfxSet, writes);
	}
	
	vkcv::PassHandle gfxPass = core.createPass(passConfig);
	
	vkcv::VertexLayout vertexLayout ({
		vkcv::VertexBinding(0, gfxProgram.getVertexAttachments())
	});
	
	vkcv::GraphicsPipelineConfig gfxPipelineConfig;
	gfxPipelineConfig.m_ShaderProgram = gfxProgram;
	gfxPipelineConfig.m_Width = windowWidth;
	gfxPipelineConfig.m_Height = windowHeight;
	gfxPipelineConfig.m_PassHandle = gfxPass;
	gfxPipelineConfig.m_VertexLayout = vertexLayout;
	gfxPipelineConfig.m_DescriptorLayouts = { core.getDescriptorSetLayout(gfxSetLayout).vulkanHandle };
	gfxPipelineConfig.m_UseDynamicViewport = true;
	gfxPipelineConfig.m_blendMode = vkcv::BlendMode::Additive;
	
	vkcv::GraphicsPipelineHandle gfxPipeline = core.createGraphicsPipeline(gfxPipelineConfig);
	
	vkcv::Buffer<glm::vec2> trianglePositions = core.createBuffer<glm::vec2>(vkcv::BufferType::VERTEX, 3);
	trianglePositions.fill({
		glm::vec2(-1.0f, -1.0f),
		glm::vec2(+0.0f, +1.5f),
		glm::vec2(+1.0f, -1.0f),
	});
	
	vkcv::Buffer<uint16_t> triangleIndices = core.createBuffer<uint16_t>(vkcv::BufferType::INDEX, 3);
	triangleIndices.fill({
		0, 1, 2
	});
	
	vkcv::Mesh triangleMesh (
			{ vkcv::VertexBufferBinding(0, trianglePositions.getVulkanHandle()) },
			triangleIndices.getVulkanHandle(),
			triangleIndices.getCount()
	);
	
	std::vector<vkcv::DrawcallInfo> drawcalls;
	
	drawcalls.push_back(vkcv::DrawcallInfo(
			triangleMesh,
			{ vkcv::DescriptorSetUsage(0, core.getDescriptorSet(gfxSet).vulkanHandle) },
			particles.getCount()
	));
	
	bool initializedParticleVolumes = false;
	
	auto start = std::chrono::system_clock::now();
	auto current = start;
	
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
		
		auto next = std::chrono::system_clock::now();
		
		auto time = std::chrono::duration_cast<std::chrono::microseconds>(next - start);
		auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(next - current);
		
		current = next;
		
		double  t = 0.000001 * static_cast<double>(time.count());
		double dt = 0.000001 * static_cast<double>(deltatime.count());
		
		vkcv::PushConstants timePushConstants (sizeof(float));
		timePushConstants.appendDrawcall(static_cast<float>(t));
		timePushConstants.appendDrawcall(static_cast<float>(dt));
		
		cameraManager.update(dt);
		
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);
		
		const uint32_t dispatchSizeGrid [3] = { 16, 16, 16 };
		const uint32_t dispatchSizeParticles [3] = { static_cast<uint32_t>(particles.getCount() + 63) / 64, 1, 1 };
		
		core.recordBeginDebugLabel(cmdStream, "TRANSFORM PARTICLES TO GRID", { 0.47f, 0.77f, 0.85f, 1.0f });
		core.prepareImageForStorage(cmdStream, grid.getHandle());
		
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				transformParticlesToGridPipeline,
				dispatchSizeGrid,
				{ vkcv::DescriptorSetUsage(
						0, core.getDescriptorSet(transformParticlesToGridSets[0]).vulkanHandle
				) },
				vkcv::PushConstants(0)
		);
		
		core.recordImageMemoryBarrier(cmdStream, grid.getHandle());
		core.recordEndDebugLabel(cmdStream);
		
		if (!initializedParticleVolumes) {
			core.recordBeginDebugLabel(cmdStream, "INIT PARTICLE VOLUMES", { 0.78f, 0.89f, 0.94f, 1.0f });
			core.recordComputeDispatchToCmdStream(
					cmdStream,
					initParticleVolumesPipeline,
					dispatchSizeParticles,
					{},
					vkcv::PushConstants(0)
			);
			core.recordEndDebugLabel(cmdStream);
			initializedParticleVolumes = true;
		}
		
		core.recordBeginDebugLabel(cmdStream, "UPDATE GRID FORCES", { 0.47f, 0.77f, 0.85f, 1.0f });
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				updateGridForcesPipeline,
				dispatchSizeGrid,
				{},
				vkcv::PushConstants(0)
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "UPDATE GRID VELOCITIES", { 0.47f, 0.77f, 0.85f, 1.0f });
		core.prepareImageForSampling(cmdStream, grid.getHandle());
		core.prepareImageForStorage(cmdStream, tmpGrid.getHandle());
		
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				updateGridVelocitiesPipeline,
				dispatchSizeGrid,
				{ vkcv::DescriptorSetUsage(
						0, core.getDescriptorSet(updateGridVelocitiesSets[0]).vulkanHandle
				) },
				timePushConstants
		);
		
		core.recordImageMemoryBarrier(cmdStream, tmpGrid.getHandle());
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "UPDATE PARTICLE DEFORMATION", { 0.78f, 0.89f, 0.94f, 1.0f });
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				updateParticleDeformationPipeline,
				dispatchSizeParticles,
				{},
				vkcv::PushConstants(0)
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "UPDATE PARTICLE VELOCITIES", { 0.78f, 0.89f, 0.94f, 1.0f });
		core.prepareImageForSampling(cmdStream, tmpGrid.getHandle());
		
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				updateParticleVelocitiesPipeline,
				dispatchSizeParticles,
				{ vkcv::DescriptorSetUsage(
						0, core.getDescriptorSet(updateParticleVelocitiesSets[0]).vulkanHandle
				) },
				vkcv::PushConstants(0)
		);
		
		core.recordBufferMemoryBarrier(cmdStream, particles.getHandle());
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "UPDATE PARTICLE POSITIONS", { 0.78f, 0.89f, 0.94f, 1.0f });
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				updateParticlePositionsPipeline,
				dispatchSizeParticles,
				{ vkcv::DescriptorSetUsage(
						0, core.getDescriptorSet(updateParticlePositionsSets[0]).vulkanHandle
				) },
				timePushConstants
		);
		
		core.recordBufferMemoryBarrier(cmdStream, particles.getHandle());
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
				vkcv::PushConstants(0),
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
