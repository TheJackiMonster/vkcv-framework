
#include <vkcv/Core.hpp>
#include <vkcv/camera/CameraManager.hpp>
#include <vkcv/gui/GUI.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>

#include <random>

struct Particle {
	glm::vec3 position;
	float size;
	glm::vec3 velocity;
	float mass;
	
	glm::vec3 pad;
	float weight_sum;
	
	glm::mat4 deformation;
};

struct Physics {
	float K;
	float E;
	float t;
	float dt;
};

struct Tweaks {
	float alpha;
	float beta;
};

float sphere_volume(float radius) {
	return 4.0f * (radius * radius * radius) * M_PI / 3.0f;
}

float sphere_radius(float volume) {
	return pow(volume * 3.0f / 4.0f / M_PI, 1.0f / 3.0f);
}

std::random_device random_dev;
std::uniform_int_distribution<int> dist(0, RAND_MAX);

float randomFloat(float min, float max) {
	return min + (max - min) * dist(random_dev) / static_cast<float>(RAND_MAX);
}

void distributeParticles(Particle *particles, size_t count, const glm::vec3& center, float radius,
						 float mass, const glm::vec3& velocity) {
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
		particles[i].velocity = velocity;
		
		volume += sphere_volume(size);
	}
	
	// Keep the same densitiy as planned!
	mass *= (volume / sphere_volume(radius));
	
	for (size_t i = 0; i < count; i++) {
		particles[i].mass = (mass * sphere_volume(particles[i].size) / volume);
		particles[i].deformation = glm::mat4(1.0f);
		
		particles[i].pad = glm::vec3(0.0f);
		particles[i].weight_sum = 1.0f;
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

void resetParticles(vkcv::Buffer<Particle>& particles, const glm::vec3& velocity,
					float density, float size) {
	std::vector<Particle> particles_vec (particles.getCount());
	
	distributeParticles(
			particles_vec.data(),
			particles_vec.size(),
			glm::vec3(0.5f),
			size,
			density * sphere_volume(size),
			velocity
	);
	
	particles.fill(particles_vec);
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
	
	uint32_t trackballIdx = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);
	cameraManager.getCamera(trackballIdx).setCenter(glm::vec3(0.5f, 0.5f, 0.5f));   // set camera to look at the center of the particle volume
	cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	
	auto swapchainExtent = core.getSwapchain(windowHandle).getExtent();
	
	vkcv::ImageHandle depthBuffer = core.createImage(
			vk::Format::eD32Sfloat,
			swapchainExtent.width,
			swapchainExtent.height
	).getHandle();
	
	glm::vec3 initialVelocity (0.0f, 1.0f, 0.0f);
	float density = 2500.0f;
	float radius = 0.1f;
	
	vkcv::Buffer<Particle> particles = core.createBuffer<Particle>(
			vkcv::BufferType::STORAGE,
			64
	);
	
	resetParticles(particles, initialVelocity, density, radius);
	
	vkcv::Image grid = core.createImage(
			vk::Format::eR32G32B32A32Sfloat,
			64,
			64,
			64,
			false,
			true
	);
	
	vkcv::Image gridCopy = core.createImage(
			grid.getFormat(),
			grid.getWidth(),
			grid.getHeight(),
			grid.getDepth(),
			false,
			true
	);
	
	std::vector<glm::vec4> grid_vec (grid.getWidth() * grid.getHeight() * grid.getDepth());
	
	for (size_t i = 0; i < grid_vec.size(); i++) {
		grid_vec[i] = glm::vec4(0.0f);
	}
	
	grid.fill(grid_vec.data());
	gridCopy.fill(grid_vec.data());
	
	vkcv::SamplerHandle gridSampler = core.createSampler(
			vkcv::SamplerFilterType::LINEAR,
			vkcv::SamplerFilterType::LINEAR,
			vkcv::SamplerMipmapMode::NEAREST,
			vkcv::SamplerAddressMode::CLAMP_TO_BORDER,
			0.0f,
			vkcv::SamplerBorderColor::FLOAT_ZERO_TRANSPARENT
	);
	
	vkcv::shader::GLSLCompiler compiler;
	
	std::vector<vkcv::DescriptorSetHandle> initParticleWeightsSets;
	vkcv::ComputePipelineHandle initParticleWeightsPipeline = createComputePipeline(
			core, compiler,
			"shaders/init_particle_weights.comp",
			initParticleWeightsSets
	);
	
	{
		vkcv::DescriptorWrites writes;
		writes.storageBufferWrites.push_back(vkcv::BufferDescriptorWrite(0, particles.getHandle()));
		writes.sampledImageWrites.push_back(vkcv::SampledImageDescriptorWrite(1, gridCopy.getHandle()));
		writes.samplerWrites.push_back(vkcv::SamplerDescriptorWrite(2, gridSampler));
		core.writeDescriptorSet(initParticleWeightsSets[0], writes);
	}
	
	std::vector<vkcv::DescriptorSetHandle> transformParticlesToGridSets;
	vkcv::ComputePipelineHandle transformParticlesToGridPipeline = createComputePipeline(
			core, compiler,
			"shaders/transform_particles_to_grid.comp",
			transformParticlesToGridSets
	);
	
	{
		vkcv::DescriptorWrites writes;
		writes.storageBufferWrites.push_back(vkcv::BufferDescriptorWrite(0, particles.getHandle()));
		writes.storageImageWrites.push_back(vkcv::StorageImageDescriptorWrite(1, gridCopy.getHandle()));
		core.writeDescriptorSet(transformParticlesToGridSets[0], writes);
	}
	
	std::vector<vkcv::DescriptorSetHandle> updateGridForcesSets;
	vkcv::ComputePipelineHandle updateGridForcesPipeline = createComputePipeline(
			core, compiler,
			"shaders/update_grid_forces.comp",
			updateGridForcesSets
	);
	
	{
		vkcv::DescriptorWrites writes;
		writes.storageImageWrites.push_back(vkcv::StorageImageDescriptorWrite(0, gridCopy.getHandle()));
		writes.storageImageWrites.push_back(vkcv::StorageImageDescriptorWrite(1, grid.getHandle()));
		writes.storageBufferWrites.push_back(vkcv::BufferDescriptorWrite(2, particles.getHandle()));
		core.writeDescriptorSet(updateGridForcesSets[0], writes);
	}
	
	std::vector<vkcv::DescriptorSetHandle> updateParticleDeformationSets;
	vkcv::ComputePipelineHandle updateParticleDeformationPipeline = createComputePipeline(
			core, compiler,
			"shaders/update_particle_deformation.comp",
			updateParticleDeformationSets
	);
	
	{
		vkcv::DescriptorWrites writes;
		writes.storageBufferWrites.push_back(vkcv::BufferDescriptorWrite(0, particles.getHandle()));
		writes.sampledImageWrites.push_back(vkcv::SampledImageDescriptorWrite(1, grid.getHandle()));
		writes.samplerWrites.push_back(vkcv::SamplerDescriptorWrite(2, gridSampler));
		core.writeDescriptorSet(updateParticleDeformationSets[0], writes);
	}
	
	std::vector<vkcv::DescriptorSetHandle> updateParticleVelocitiesSets;
	vkcv::ComputePipelineHandle updateParticleVelocitiesPipeline = createComputePipeline(
			core, compiler,
			"shaders/update_particle_velocities.comp",
			updateParticleVelocitiesSets
	);
	
	{
		vkcv::DescriptorWrites writes;
		writes.storageBufferWrites.push_back(vkcv::BufferDescriptorWrite(0, particles.getHandle()));
		writes.sampledImageWrites.push_back(vkcv::SampledImageDescriptorWrite(1, grid.getHandle()));
		writes.sampledImageWrites.push_back(vkcv::SampledImageDescriptorWrite(2, gridCopy.getHandle()));
		writes.samplerWrites.push_back(vkcv::SamplerDescriptorWrite(3, gridSampler));
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
	
	vkcv::ShaderProgram gfxProgramGrid;
	
	compiler.compile(
			vkcv::ShaderStage::VERTEX,
			"shaders/grid.vert",
			[&gfxProgramGrid](vkcv::ShaderStage stage, const std::filesystem::path& path) {
				gfxProgramGrid.addShader(stage, path);
			}
	);
	
	compiler.compile(
			vkcv::ShaderStage::FRAGMENT,
			"shaders/grid.frag",
			[&gfxProgramGrid](vkcv::ShaderStage stage, const std::filesystem::path& path) {
				gfxProgramGrid.addShader(stage, path);
			}
	);
	
	vkcv::ShaderProgram gfxProgramParticles;
	
	compiler.compile(
			vkcv::ShaderStage::VERTEX,
			"shaders/particle.vert",
			[&gfxProgramParticles](vkcv::ShaderStage stage, const std::filesystem::path& path) {
				gfxProgramParticles.addShader(stage, path);
			}
	);
	
	compiler.compile(
			vkcv::ShaderStage::FRAGMENT,
			"shaders/particle.frag",
			[&gfxProgramParticles](vkcv::ShaderStage stage, const std::filesystem::path& path) {
				gfxProgramParticles.addShader(stage, path);
			}
	);
	
	vkcv::ShaderProgram gfxProgramLines;
	
	compiler.compile(
			vkcv::ShaderStage::VERTEX,
			"shaders/lines.vert",
			[&gfxProgramLines](vkcv::ShaderStage stage, const std::filesystem::path& path) {
				gfxProgramLines.addShader(stage, path);
			}
	);
	
	compiler.compile(
			vkcv::ShaderStage::FRAGMENT,
			"shaders/lines.frag",
			[&gfxProgramLines](vkcv::ShaderStage stage, const std::filesystem::path& path) {
				gfxProgramLines.addShader(stage, path);
			}
	);
	
	vkcv::PassConfig passConfigGrid ({
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
	
	vkcv::PassConfig passConfigParticles ({
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
	
	vkcv::PassConfig passConfigLines ({
		vkcv::AttachmentDescription(
				vkcv::AttachmentOperation::STORE,
				vkcv::AttachmentOperation::LOAD,
				core.getSwapchain(windowHandle).getFormat()
		),
		vkcv::AttachmentDescription(
				vkcv::AttachmentOperation::STORE,
				vkcv::AttachmentOperation::LOAD,
				vk::Format::eD32Sfloat
		)
	});
	
	vkcv::DescriptorSetLayoutHandle gfxSetLayoutGrid = core.createDescriptorSetLayout(
			gfxProgramGrid.getReflectedDescriptors().at(0)
	);
	
	vkcv::DescriptorSetHandle gfxSetGrid = core.createDescriptorSet(gfxSetLayoutGrid);
	
	{
		vkcv::DescriptorWrites writes;
		writes.sampledImageWrites.push_back(vkcv::SampledImageDescriptorWrite(0, grid.getHandle()));
		writes.samplerWrites.push_back(vkcv::SamplerDescriptorWrite(1, gridSampler));
		core.writeDescriptorSet(gfxSetGrid, writes);
	}
	
	vkcv::DescriptorSetLayoutHandle gfxSetLayoutParticles = core.createDescriptorSetLayout(
			gfxProgramParticles.getReflectedDescriptors().at(0)
	);
	
	vkcv::DescriptorSetHandle gfxSetParticles = core.createDescriptorSet(gfxSetLayoutParticles);
	
	{
		vkcv::DescriptorWrites writes;
		writes.storageBufferWrites.push_back(vkcv::BufferDescriptorWrite(0, particles.getHandle()));
		core.writeDescriptorSet(gfxSetParticles, writes);
	}
	
	vkcv::PassHandle gfxPassGrid = core.createPass(passConfigGrid);
	vkcv::PassHandle gfxPassParticles = core.createPass(passConfigParticles);
	vkcv::PassHandle gfxPassLines = core.createPass(passConfigLines);
	
	vkcv::VertexLayout vertexLayoutGrid ({
		vkcv::VertexBinding(0, gfxProgramGrid.getVertexAttachments())
	});
	
	vkcv::GraphicsPipelineConfig gfxPipelineConfigGrid;
	gfxPipelineConfigGrid.m_ShaderProgram = gfxProgramGrid;
	gfxPipelineConfigGrid.m_Width = windowWidth;
	gfxPipelineConfigGrid.m_Height = windowHeight;
	gfxPipelineConfigGrid.m_PassHandle = gfxPassGrid;
	gfxPipelineConfigGrid.m_VertexLayout = vertexLayoutGrid;
	gfxPipelineConfigGrid.m_DescriptorLayouts = { gfxSetLayoutGrid };
	gfxPipelineConfigGrid.m_UseDynamicViewport = true;
	gfxPipelineConfigGrid.m_blendMode = vkcv::BlendMode::Additive;
	
	vkcv::VertexLayout vertexLayoutParticles ({
		vkcv::VertexBinding(0, gfxProgramParticles.getVertexAttachments())
	});
	
	vkcv::GraphicsPipelineConfig gfxPipelineConfigParticles;
	gfxPipelineConfigParticles.m_ShaderProgram = gfxProgramParticles;
	gfxPipelineConfigParticles.m_Width = windowWidth;
	gfxPipelineConfigParticles.m_Height = windowHeight;
	gfxPipelineConfigParticles.m_PassHandle = gfxPassParticles;
	gfxPipelineConfigParticles.m_VertexLayout = vertexLayoutParticles;
	gfxPipelineConfigParticles.m_DescriptorLayouts = { gfxSetLayoutParticles };
	gfxPipelineConfigParticles.m_UseDynamicViewport = true;
	
	vkcv::VertexLayout vertexLayoutLines ({
		vkcv::VertexBinding(0, gfxProgramLines.getVertexAttachments())
	});
	
	vkcv::GraphicsPipelineConfig gfxPipelineConfigLines;
	gfxPipelineConfigLines.m_ShaderProgram = gfxProgramLines;
	gfxPipelineConfigLines.m_Width = windowWidth;
	gfxPipelineConfigLines.m_Height = windowHeight;
	gfxPipelineConfigLines.m_PassHandle = gfxPassLines;
	gfxPipelineConfigLines.m_VertexLayout = vertexLayoutLines;
	gfxPipelineConfigLines.m_DescriptorLayouts = {};
	gfxPipelineConfigLines.m_UseDynamicViewport = true;
	gfxPipelineConfigLines.m_PrimitiveTopology = vkcv::PrimitiveTopology::LineList;
	
	vkcv::GraphicsPipelineHandle gfxPipelineGrid = core.createGraphicsPipeline(gfxPipelineConfigGrid);
	vkcv::GraphicsPipelineHandle gfxPipelineParticles = core.createGraphicsPipeline(gfxPipelineConfigParticles);
	vkcv::GraphicsPipelineHandle gfxPipelineLines = core.createGraphicsPipeline(gfxPipelineConfigLines);
	
	vkcv::Buffer<glm::vec2> trianglePositions = core.createBuffer<glm::vec2>(vkcv::BufferType::VERTEX, 3);
	trianglePositions.fill({
		glm::vec2(-1.0f, -1.0f),
		glm::vec2(+0.0f, +1.5f),
		glm::vec2(+1.0f, -1.0f)
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
	
	vkcv::Buffer<glm::vec3> linesPositions = core.createBuffer<glm::vec3>(vkcv::BufferType::VERTEX, 8);
	linesPositions.fill({
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(1.0f, 0.0f, +1.0f),
		glm::vec3(0.0f, 1.0f, 1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f)
	});
	
	vkcv::Buffer<uint16_t> linesIndices = core.createBuffer<uint16_t>(vkcv::BufferType::INDEX, 24);
	linesIndices.fill({
		0, 1,
		1, 3,
		3, 2,
		2, 0,
		
		4, 5,
		5, 7,
		7, 6,
		6, 4,
		
		0, 4,
		1, 5,
		2, 6,
		3, 7
	});
	
	vkcv::Mesh linesMesh (
			{ vkcv::VertexBufferBinding(0, linesPositions.getVulkanHandle()) },
			linesIndices.getVulkanHandle(),
			linesIndices.getCount()
	);
	
	std::vector<vkcv::DrawcallInfo> drawcallsGrid;
	
	drawcallsGrid.push_back(vkcv::DrawcallInfo(
			triangleMesh,
			{ vkcv::DescriptorSetUsage(0, core.getDescriptorSet(gfxSetGrid).vulkanHandle) },
			grid.getWidth() * grid.getHeight() * grid.getDepth()
	));
	
	std::vector<vkcv::DrawcallInfo> drawcallsParticles;
	
	drawcallsParticles.push_back(vkcv::DrawcallInfo(
			triangleMesh,
			{ vkcv::DescriptorSetUsage(0, core.getDescriptorSet(gfxSetParticles).vulkanHandle) },
			particles.getCount()
	));
	
	std::vector<vkcv::DrawcallInfo> drawcallsLines;
	
	drawcallsLines.push_back(vkcv::DrawcallInfo(
			linesMesh,
			{},
			1
	));
	
	bool renderGrid = true;
	
	// Glass is glass and glass breaks...
	float compression_modulus = 65.0f;
	int compression_exponent = 9;
	
	float elasticity_modulus = 45.0f;
	int elasticity_exponent = 9;
	
	float alpha = 1.0f;
	float beta = 0.0f;
	
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
		
		Physics physics;
		physics.K = static_cast<float>(compression_modulus * std::pow(10.0, compression_exponent));
		physics.E = static_cast<float>(elasticity_modulus * std::pow(10.0, elasticity_exponent));
		physics.t = static_cast<float>(0.000001 * static_cast<double>(time.count()));
		physics.dt = static_cast<float>(0.000001 * static_cast<double>(deltatime.count()));
		
		vkcv::PushConstants physicsPushConstants (sizeof(physics));
		physicsPushConstants.appendDrawcall(physics);
		
		Tweaks tweaks;
		tweaks.alpha = alpha;
		tweaks.beta = beta;
		
		vkcv::PushConstants tweakPushConstants (sizeof(tweaks));
		tweakPushConstants.appendDrawcall(tweaks);
		
		cameraManager.update(physics.dt);

		glm::mat4 mvp = cameraManager.getActiveCamera().getMVP();
		vkcv::PushConstants cameraPushConstants (sizeof(glm::mat4));
		cameraPushConstants.appendDrawcall(mvp);

		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);
		
		const uint32_t dispatchSizeGrid [3] = { 16, 16, 16 };
		const uint32_t dispatchSizeParticles [3] = { static_cast<uint32_t>(particles.getCount() + 63) / 64, 1, 1 };
		
		core.recordBeginDebugLabel(cmdStream, "INIT PARTICLE WEIGHTS", { 0.78f, 0.89f, 0.94f, 1.0f });
		core.recordBufferMemoryBarrier(cmdStream, particles.getHandle());
		core.prepareImageForSampling(cmdStream, grid.getHandle());
		
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				initParticleWeightsPipeline,
				dispatchSizeParticles,
				{ vkcv::DescriptorSetUsage(
						0, core.getDescriptorSet(initParticleWeightsSets[0]).vulkanHandle
				) },
				vkcv::PushConstants(0)
		);
		
		core.recordBufferMemoryBarrier(cmdStream, particles.getHandle());
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "TRANSFORM PARTICLES TO GRID", { 0.47f, 0.77f, 0.85f, 1.0f });
		core.recordBufferMemoryBarrier(cmdStream, particles.getHandle());
		core.prepareImageForStorage(cmdStream, gridCopy.getHandle());
		
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				transformParticlesToGridPipeline,
				dispatchSizeGrid,
				{ vkcv::DescriptorSetUsage(
						0, core.getDescriptorSet(transformParticlesToGridSets[0]).vulkanHandle
				) },
				vkcv::PushConstants(0)
		);
		
		core.recordImageMemoryBarrier(cmdStream, gridCopy.getHandle());
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "UPDATE GRID FORCES", { 0.47f, 0.77f, 0.85f, 1.0f });
		core.recordBufferMemoryBarrier(cmdStream, particles.getHandle());
		core.prepareImageForStorage(cmdStream, grid.getHandle());
		core.prepareImageForStorage(cmdStream, gridCopy.getHandle());
		
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				updateGridForcesPipeline,
				dispatchSizeGrid,
				{ vkcv::DescriptorSetUsage(
						0, core.getDescriptorSet(updateGridForcesSets[0]).vulkanHandle
				) },
				physicsPushConstants
		);
		
		core.recordImageMemoryBarrier(cmdStream, grid.getHandle());
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "UPDATE PARTICLE DEFORMATION", { 0.78f, 0.89f, 0.94f, 1.0f });
		core.recordBufferMemoryBarrier(cmdStream, particles.getHandle());
		core.prepareImageForSampling(cmdStream, grid.getHandle());
		
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				updateParticleDeformationPipeline,
				dispatchSizeParticles,
				{ vkcv::DescriptorSetUsage(
						0, core.getDescriptorSet(updateParticleDeformationSets[0]).vulkanHandle
				) },
				physicsPushConstants
		);
		
		core.recordBufferMemoryBarrier(cmdStream, particles.getHandle());
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "UPDATE PARTICLE VELOCITIES", { 0.78f, 0.89f, 0.94f, 1.0f });
		core.recordBufferMemoryBarrier(cmdStream, particles.getHandle());
		core.prepareImageForSampling(cmdStream, grid.getHandle());
		core.prepareImageForSampling(cmdStream, gridCopy.getHandle());
		
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				updateParticleVelocitiesPipeline,
				dispatchSizeParticles,
				{ vkcv::DescriptorSetUsage(
						0, core.getDescriptorSet(updateParticleVelocitiesSets[0]).vulkanHandle
				) },
				tweakPushConstants
		);
		
		core.recordBufferMemoryBarrier(cmdStream, particles.getHandle());
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "UPDATE PARTICLE POSITIONS", { 0.78f, 0.89f, 0.94f, 1.0f });
		core.recordBufferMemoryBarrier(cmdStream, particles.getHandle());
		
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				updateParticlePositionsPipeline,
				dispatchSizeParticles,
				{ vkcv::DescriptorSetUsage(
						0, core.getDescriptorSet(updateParticlePositionsSets[0]).vulkanHandle
				) },
				physicsPushConstants
		);
		
		core.recordBufferMemoryBarrier(cmdStream, particles.getHandle());
		core.recordEndDebugLabel(cmdStream);
		
		std::vector<vkcv::ImageHandle> renderTargets {
				vkcv::ImageHandle::createSwapchainImageHandle(),
				depthBuffer
		};
		
		if (renderGrid) {
			core.recordBeginDebugLabel(cmdStream, "RENDER GRID", { 0.13f, 0.20f, 0.22f, 1.0f });
			core.prepareImageForSampling(cmdStream, grid.getHandle());
			
			core.recordDrawcallsToCmdStream(
					cmdStream,
					gfxPassGrid,
					gfxPipelineGrid,
					cameraPushConstants,
					drawcallsGrid,
					renderTargets,
					windowHandle
			);
			
			core.recordEndDebugLabel(cmdStream);
		} else {
			core.recordBeginDebugLabel(cmdStream, "RENDER PARTICLES", { 0.13f, 0.20f, 0.22f, 1.0f });
			core.recordBufferMemoryBarrier(cmdStream, particles.getHandle());
			
			core.recordDrawcallsToCmdStream(
					cmdStream,
					gfxPassParticles,
					gfxPipelineParticles,
					cameraPushConstants,
					drawcallsParticles,
					renderTargets,
					windowHandle
			);
			
			core.recordEndDebugLabel(cmdStream);
		}
		
		core.recordBeginDebugLabel(cmdStream, "RENDER LINES", { 0.13f, 0.20f, 0.22f, 1.0f });
		
		core.recordDrawcallsToCmdStream(
				cmdStream,
				gfxPassLines,
				gfxPipelineLines,
				cameraPushConstants,
				drawcallsLines,
				renderTargets,
				windowHandle
		);
		
		core.recordEndDebugLabel(cmdStream);
		
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		
		gui.beginGUI();
		ImGui::Begin("Settings");
		
		ImGui::SliderFloat("Density", &density, std::numeric_limits<float>::epsilon(), 5000.0f);
		ImGui::SameLine(0.0f, 10.0f);
		if (ImGui::SmallButton("Reset##density")) {
			density = 2500.0f;
		}
		
		ImGui::SliderFloat("Radius", &radius, 0.0f, 0.5f);
		ImGui::SameLine(0.0f, 10.0f);
		if (ImGui::SmallButton("Reset##radius")) {
			radius = 0.1f;
		}
		
		ImGui::BeginGroup();
		ImGui::SliderFloat("Compression Modulus", &compression_modulus, 0.0f, 500.0f);
		ImGui::SliderInt("##compression_exponent", &compression_exponent, 1, 9);
		ImGui::SameLine(0.0f, 10.0f);
		if (ImGui::SmallButton("Reset##compression")) {
			compression_modulus = 65.0f;
			compression_exponent = 9;
		}
		ImGui::EndGroup();
		
		ImGui::BeginGroup();
		ImGui::SliderFloat("Elasticity Modulus", &elasticity_modulus, 0.0f, 1000.0f);
		ImGui::SliderInt("##elasticity_exponent", &elasticity_exponent, 1, 9);
		ImGui::SameLine(0.0f, 10.0f);
		if (ImGui::SmallButton("Reset##elasticity")) {
			elasticity_modulus = 45.0f;
			elasticity_exponent = 9;
		}
		ImGui::EndGroup();
		
		ImGui::Spacing();
		ImGui::Checkbox("Render Grid", &renderGrid);
		
		ImGui::SliderFloat("Alpha (PIC -> FLIP)", &alpha, 0.0f, 1.0f);
		ImGui::SameLine(0.0f, 10.0f);
		if (ImGui::SmallButton("Reset##alpha")) {
		    alpha = 0.5f;
		}
		
		ImGui::SliderFloat("Beta (Alpha -> APIC)", &beta, 0.0f, 1.0f);
		ImGui::SameLine(0.0f, 10.0f);
		if (ImGui::SmallButton("Reset##beta")) {
		    beta = 0.75f;
		}
		
		ImGui::DragFloat3("Initial Velocity", reinterpret_cast<float*>(&initialVelocity), 0.001f);
		ImGui::SameLine(0.0f, 10.0f);
		if (ImGui::Button("Reset##particle_velocity")) {
			resetParticles(particles, initialVelocity, density, radius);
		}
		
		ImGui::End();
		gui.endGUI();
		
		core.endFrame(windowHandle);
	}
	
	return 0;
}
