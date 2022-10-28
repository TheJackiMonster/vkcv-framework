
#include <vkcv/Buffer.hpp>
#include <vkcv/Core.hpp>
#include <vkcv/Pass.hpp>
#include <vkcv/Image.hpp>
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
	glm::mat4 mls;
};

#define SIM_FORM_SPHERE 0
#define SIM_FORM_CUBE 1

#define SIM_TYPE_HYPERELASTIC 0
#define SIM_TYPE_FLUID 1

#define SIM_MODE_RANDOM 0
#define SIM_MODE_ORDERED 1

struct Simulation {
	float density;
	float size;
	float lame1;
	float lame2;
	
	int form;
	int type;
	float K;
	float E;
	
	float gamma;
	int mode;
	float gravity;
	int count;
};

struct Physics {
	float t;
	float dt;
	float speedfactor;
};

float sphere_volume(float radius) {
	return 4.0f * (radius * radius * radius) * M_PI / 3.0f;
}

float sphere_radius(float volume) {
	return std::pow(volume * 3.0f / 4.0f / M_PI, 1.0f / 3.0f);
}

float cube_volume(float radius) {
	return 8.0f * (radius * radius * radius);
}

float cube_radius(float volume) {
	return std::pow(volume / 8.0f, 1.0f / 3.0f);
}

std::random_device random_dev;
std::uniform_int_distribution<int> dist(0, RAND_MAX);

float randomFloat(float min, float max) {
	return min + (max - min) * dist(random_dev) / static_cast<float>(RAND_MAX);
}

float mod(float x, float y) {
	return x - std::floor(x / y) * y;
}

void distributeParticlesCube(Particle *particles, size_t count, const glm::vec3& center, float radius,
							 float mass, const glm::vec3& velocity, bool random) {
	const float side = cube_radius(static_cast<float>(count)) * 2.0f;
	
	float volume = 0.0f;
	
	for (size_t i = 0; i < count; i++) {
		glm::vec3 offset;
		
		if (random) {
			offset.x = randomFloat(-1.0f, +1.0f);
			offset.y = randomFloat(-1.0f, +1.0f);
			offset.z = randomFloat(-1.0f, +1.0f);
		} else {
			const float s = static_cast<float>(i) + 0.5f;
			
			offset.x = 2.0f * mod(s, side) / side - 1.0f;
			offset.y = 2.0f * mod(s / side, side) / side - 1.0f;
			offset.z = 2.0f * mod(s / side / side, side) / side - 1.0f;
		}
		
		offset *= radius;
		
		float size = 0.0f;
		
		if (random) {
			const float ax = std::abs(offset.x);
			const float ay = std::abs(offset.y);
			const float az = std::abs(offset.z);
			
			const float a = std::max(std::max(ax, ay), az);
			
			size = (radius - a);
		} else {
			size = 2.0f * radius / side;
		}
		
		particles[i].position = center + offset;
		particles[i].size = size;
		particles[i].velocity = velocity;
		
		volume += cube_volume(size);
	}
	
	for (size_t i = 0; i < count; i++) {
		particles[i].mass = (mass * cube_volume(particles[i].size) / volume);
		particles[i].deformation = glm::mat4(1.0f);
		
		particles[i].pad = glm::vec3(0.0f);
		particles[i].weight_sum = 1.0f;
		
		particles[i].mls = glm::mat4(0.0f);
	}
}

void distributeParticlesSphere(Particle *particles, size_t count, const glm::vec3& center, float radius,
							   float mass, const glm::vec3& velocity, bool random) {
	const float side = sphere_radius(static_cast<float>(count)) * 2.0f;
	
	float volume = 0.0f;
	
	for (size_t i = 0; i < count; i++) {
		glm::vec3 offset;
		
		if (random) {
			offset.x = randomFloat(-1.0f, +1.0f);
			offset.y = randomFloat(-1.0f, +1.0f);
			offset.z = randomFloat(-1.0f, +1.0f);
			
			if (glm::length(offset) > 0.0f)
				offset = glm::normalize(offset);
			
			offset *= randomFloat(0.0f, 1.0f);
		} else {
			const float range = 0.5f * side;
			const float s = static_cast<float>(i) + 0.5f;
			
			float a = mod(s, range) / range;
			float b = mod(s / range, M_PI * range);
			float c = mod(s / range / M_PI / range, M_PI * range * 2.0f);
			
			offset.x = a * std::sin(c) * std::sin(b);
			offset.y = a * std::cos(b);
			offset.z = a * std::cos(c) * std::sin(b);
		}
		
		offset *= radius;
		
		float size = 0.0f;
		
		if (random) {
			size = (radius - glm::length(offset));
		} else {
			size = 2.0f * radius / side;
		}
		
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
		
		particles[i].mls = glm::mat4(0.0f);
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

vkcv::BufferHandle resetParticles(vkcv::Core& core, size_t count, const glm::vec3& velocity,
					float density, float size, int form, int mode) {
	vkcv::Buffer<Particle> particles = vkcv::buffer<Particle>(
			core,
			vkcv::BufferType::STORAGE,
			count
	);
	
	std::vector<Particle> particles_vec (particles.getCount());
	
	switch (form) {
		case SIM_FORM_SPHERE:
			distributeParticlesSphere(
					particles_vec.data(),
					particles_vec.size(),
					glm::vec3(0.5f),
					size,
					density * sphere_volume(size),
					velocity,
					(mode == 0)
			);
			break;
		case SIM_FORM_CUBE:
			distributeParticlesCube(
					particles_vec.data(),
					particles_vec.size(),
					glm::vec3(0.5f),
					size,
					density * sphere_volume(size),
					velocity,
					(mode == 0)
			);
			break;
		default:
			break;
	}
	
	particles.fill(particles_vec);
	return particles.getHandle();
}

int main(int argc, const char **argv) {
	const std::string applicationName = "MPM";
	
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
	
	auto trackballHandle = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);
	cameraManager.getCamera(trackballHandle).setCenter(glm::vec3(0.5f, 0.5f, 0.5f));   // set camera to look at the center of the particle volume
	cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	
	auto swapchainExtent = core.getSwapchainExtent(window.getSwapchain());
	
	vkcv::ImageHandle depthBuffer = core.createImage(
			vk::Format::eD32Sfloat,
			vkcv::ImageConfig(
					swapchainExtent.width,
					swapchainExtent.height
			)
	);
	
	vkcv::Image grid = vkcv::image(
			core,
			vk::Format::eR16G16B16A16Sfloat,
			32,
			32,
			32,
			false,
			true
	);
	
	vkcv::SamplerHandle gridSampler = core.createSampler(
			vkcv::SamplerFilterType::LINEAR,
			vkcv::SamplerFilterType::LINEAR,
			vkcv::SamplerMipmapMode::NEAREST,
			vkcv::SamplerAddressMode::CLAMP_TO_BORDER,
			0.0f,
			vkcv::SamplerBorderColor::FLOAT_ZERO_TRANSPARENT
	);
	
	vkcv::Buffer<Simulation> simulation = vkcv::buffer<Simulation>(
			core, vkcv::BufferType::UNIFORM, 1, vkcv::BufferMemoryType::HOST_VISIBLE
	);
	
	Simulation* sim = simulation.map();
	
	glm::vec3 initialVelocity (0.0f, 0.1f, 0.0f);
	
	sim->density = 2500.0f;
	sim->size = 0.1f;
	sim->lame1 = 10.0f;
	sim->lame2 = 20.0f;
	sim->form = SIM_FORM_SPHERE;
	sim->type = SIM_TYPE_HYPERELASTIC;
	sim->K = 2.2f;
	sim->E = 35.0f;
	sim->gamma = 1.330f;
	sim->mode = SIM_MODE_RANDOM;
	sim->gravity = 9.81f;
	sim->count = 1024;
	
	vkcv::BufferHandle particlesHandle = resetParticles(
			core,
			sim->count,
			initialVelocity,
			sim->density,
			sim->size,
			sim->form,
			sim->mode
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
		writes.writeStorageBuffer(0, particlesHandle);
		core.writeDescriptorSet(initParticleWeightsSets[0], writes);
	}
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeSampledImage(0, grid.getHandle());
		writes.writeSampler(1, gridSampler);
		core.writeDescriptorSet(initParticleWeightsSets[1], writes);
	}
	
	std::vector<vkcv::DescriptorSetHandle> transformParticlesToGridSets;
	vkcv::ComputePipelineHandle transformParticlesToGridPipeline = createComputePipeline(
			core, compiler,
			"shaders/transform_particles_to_grid.comp",
			transformParticlesToGridSets
	);
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeStorageBuffer(0, particlesHandle);
		core.writeDescriptorSet(transformParticlesToGridSets[0], writes);
	}
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeUniformBuffer(0, simulation.getHandle());
		core.writeDescriptorSet(transformParticlesToGridSets[1], writes);
	}
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeStorageImage(0, grid.getHandle());
		core.writeDescriptorSet(transformParticlesToGridSets[2], writes);
	}
	
	std::vector<vkcv::DescriptorSetHandle> updateParticleVelocitiesSets;
	vkcv::ComputePipelineHandle updateParticleVelocitiesPipeline = createComputePipeline(
			core, compiler,
			"shaders/update_particle_velocities.comp",
			updateParticleVelocitiesSets
	);
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeStorageBuffer(0, particlesHandle);
		core.writeDescriptorSet(updateParticleVelocitiesSets[0], writes);
	}
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeUniformBuffer(0, simulation.getHandle());
		core.writeDescriptorSet(updateParticleVelocitiesSets[1], writes);
	}
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeSampledImage(0, grid.getHandle());
		writes.writeSampler(1, gridSampler);
		core.writeDescriptorSet(updateParticleVelocitiesSets[2], writes);
	}
	
	vkcv::ShaderProgram gfxProgramGrid;
	
	compiler.compileProgram(gfxProgramGrid, {
			{ vkcv::ShaderStage::VERTEX, "shaders/grid.vert" },
			{ vkcv::ShaderStage::FRAGMENT, "shaders/grid.frag" }
	}, nullptr);
	
	vkcv::ShaderProgram gfxProgramParticles;
	
	compiler.compileProgram(gfxProgramParticles, {
		{ vkcv::ShaderStage::VERTEX, "shaders/particle.vert" },
		{ vkcv::ShaderStage::FRAGMENT, "shaders/particle.frag" }
	}, nullptr);
	
	vkcv::ShaderProgram gfxProgramLines;
	
	compiler.compileProgram(gfxProgramLines, {
			{ vkcv::ShaderStage::VERTEX, "shaders/lines.vert" },
			{ vkcv::ShaderStage::FRAGMENT, "shaders/lines.frag" }
	}, nullptr);
	
	vkcv::DescriptorSetLayoutHandle gfxSetLayoutGrid = core.createDescriptorSetLayout(
			gfxProgramGrid.getReflectedDescriptors().at(0)
	);
	
	vkcv::DescriptorSetHandle gfxSetGrid = core.createDescriptorSet(gfxSetLayoutGrid);
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeSampledImage(0, grid.getHandle());
		writes.writeSampler(1, gridSampler);
		writes.writeUniformBuffer(2, simulation.getHandle());
		core.writeDescriptorSet(gfxSetGrid, writes);
	}
	
	vkcv::DescriptorSetLayoutHandle gfxSetLayoutParticles = core.createDescriptorSetLayout(
			gfxProgramParticles.getReflectedDescriptors().at(0)
	);
	
	vkcv::DescriptorSetHandle gfxSetParticles = core.createDescriptorSet(gfxSetLayoutParticles);
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeStorageBuffer(0, particlesHandle);
		core.writeDescriptorSet(gfxSetParticles, writes);
	}
	
	vkcv::PassHandle gfxPassGrid = vkcv::passSwapchain(
			core,
			window.getSwapchain(),
			{ vk::Format::eUndefined, vk::Format::eD32Sfloat }
	);
	
	vkcv::PassHandle gfxPassParticles = vkcv::passSwapchain(
			core,
			window.getSwapchain(),
			{ vk::Format::eUndefined, vk::Format::eD32Sfloat }
	);
	
	vkcv::PassHandle gfxPassLines = vkcv::passSwapchain(
			core,
			window.getSwapchain(),
			{ vk::Format::eUndefined, vk::Format::eD32Sfloat },
			false
	);
	
	vkcv::VertexLayout vertexLayoutGrid = vkcv::createVertexLayout({
		vkcv::createVertexBinding(0, gfxProgramGrid.getVertexAttachments())
	});
	
	vkcv::GraphicsPipelineConfig gfxPipelineConfigGrid (
			gfxProgramGrid,
			gfxPassGrid,
			vertexLayoutGrid,
			{ gfxSetLayoutGrid }
	);
	
	vkcv::VertexLayout vertexLayoutParticles = vkcv::createVertexLayout({
		vkcv::createVertexBinding(0, gfxProgramParticles.getVertexAttachments())
	});
	
	vkcv::GraphicsPipelineConfig gfxPipelineConfigParticles (
			gfxProgramParticles,
			gfxPassParticles,
			vertexLayoutParticles,
			{ gfxSetLayoutParticles }
	);
	
	vkcv::VertexLayout vertexLayoutLines = vkcv::createVertexLayout({
		vkcv::createVertexBinding(0, gfxProgramLines.getVertexAttachments())
	});
	
	vkcv::GraphicsPipelineConfig gfxPipelineConfigLines (
			gfxProgramLines,
			gfxPassLines,
			vertexLayoutLines,
			{}
	);
	
	gfxPipelineConfigLines.setPrimitiveTopology(vkcv::PrimitiveTopology::LineList);
	
	vkcv::GraphicsPipelineHandle gfxPipelineGrid = core.createGraphicsPipeline(gfxPipelineConfigGrid);
	vkcv::GraphicsPipelineHandle gfxPipelineParticles = core.createGraphicsPipeline(gfxPipelineConfigParticles);
	vkcv::GraphicsPipelineHandle gfxPipelineLines = core.createGraphicsPipeline(gfxPipelineConfigLines);
	
	vkcv::Buffer<glm::vec2> trianglePositions = vkcv::buffer<glm::vec2>(core, vkcv::BufferType::VERTEX, 3);
	trianglePositions.fill({
		glm::vec2(-1.0f, -1.0f),
		glm::vec2(+0.0f, +1.5f),
		glm::vec2(+1.0f, -1.0f)
	});
	
	vkcv::VertexData triangleData ({ vkcv::vertexBufferBinding(trianglePositions.getHandle()) });
	triangleData.setCount(trianglePositions.getCount());
	
	vkcv::Buffer<glm::vec3> linesPositions = vkcv::buffer<glm::vec3>(core, vkcv::BufferType::VERTEX, 8);
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
	
	vkcv::Buffer<uint16_t> linesIndices = vkcv::buffer<uint16_t>(core, vkcv::BufferType::INDEX, 24);
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
	
	vkcv::VertexData linesData ({ vkcv::vertexBufferBinding(linesPositions.getHandle()) });
	linesData.setIndexBuffer(linesIndices.getHandle());
	linesData.setCount(linesIndices.getCount());
	
	vkcv::InstanceDrawcall drawcallGrid (
			triangleData,
			grid.getWidth() * grid.getHeight() * grid.getDepth()
	);
	
	drawcallGrid.useDescriptorSet(0, gfxSetGrid);
	
	vkcv::InstanceDrawcall drawcallParticle (triangleData, sim->count);
	drawcallParticle.useDescriptorSet(0, gfxSetParticles);
	
	vkcv::InstanceDrawcall drawcallLines (linesData);
	
	bool renderGrid = true;
	float speed_factor = 1.0f;
	
	core.run([&](const vkcv::WindowHandle &windowHandle, double t, double dt,
				 uint32_t swapchainWidth, uint32_t swapchainHeight) {
		if ((swapchainWidth != swapchainExtent.width) || ((swapchainHeight != swapchainExtent.height))) {
			depthBuffer = core.createImage(
					vk::Format::eD32Sfloat,
					vkcv::ImageConfig(
							swapchainWidth,
							swapchainHeight
					)
			);
			
			swapchainExtent.width = swapchainWidth;
			swapchainExtent.height = swapchainHeight;
		}
		
		Physics physics;
		physics.t = static_cast<float>(t);
		physics.dt = static_cast<float>(dt);
		physics.speedfactor = speed_factor;
		
		vkcv::PushConstants physicsPushConstants = vkcv::pushConstants<Physics>();
		physicsPushConstants.appendDrawcall(physics);
		
		cameraManager.update(physics.dt);
		
		glm::mat4 mvp = cameraManager.getActiveCamera().getMVP();
		vkcv::PushConstants cameraPushConstants = vkcv::pushConstants<glm::mat4>();
		cameraPushConstants.appendDrawcall(mvp);
		
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);
		
		const auto dispatchSizeGrid = vkcv::dispatchInvocations(
				vkcv::DispatchSize(grid.getWidth(), grid.getHeight(), grid.getDepth()),
				vkcv::DispatchSize(4, 4, 4)
		);
		
		const auto dispatchSizeParticles = vkcv::dispatchInvocations(sim->count, 64);
		
		for (int step = 0; step < 1; step++) {
			core.recordBeginDebugLabel(cmdStream, "INIT PARTICLE WEIGHTS", {0.78f, 0.89f, 0.94f, 1.0f});
			core.recordBufferMemoryBarrier(cmdStream, particlesHandle);
			core.prepareImageForSampling(cmdStream, grid.getHandle());
			
			core.recordComputeDispatchToCmdStream(
					cmdStream,
					initParticleWeightsPipeline,
					dispatchSizeParticles,
					{
						vkcv::useDescriptorSet(
								0, initParticleWeightsSets[0]
						),
						vkcv::useDescriptorSet(
								1, initParticleWeightsSets[1]
						)
					},
					vkcv::PushConstants(0)
			);
			
			core.recordBufferMemoryBarrier(cmdStream, particlesHandle);
			core.recordEndDebugLabel(cmdStream);
			
			core.recordBeginDebugLabel(cmdStream, "TRANSFORM PARTICLES TO GRID", {0.47f, 0.77f, 0.85f, 1.0f});
			core.recordBufferMemoryBarrier(cmdStream, particlesHandle);
			core.prepareImageForStorage(cmdStream, grid.getHandle());
			
			core.recordComputeDispatchToCmdStream(
					cmdStream,
					transformParticlesToGridPipeline,
					dispatchSizeGrid,
					{
						vkcv::useDescriptorSet(
								0, transformParticlesToGridSets[0]
						),
						vkcv::useDescriptorSet(
								1, transformParticlesToGridSets[1]
						),
						vkcv::useDescriptorSet(
								2, transformParticlesToGridSets[2]
						)
					},
					physicsPushConstants
			);
			
			core.recordImageMemoryBarrier(cmdStream, grid.getHandle());
			core.recordEndDebugLabel(cmdStream);
			
			core.recordBeginDebugLabel(cmdStream, "UPDATE PARTICLE VELOCITIES", {0.78f, 0.89f, 0.94f, 1.0f});
			core.recordBufferMemoryBarrier(cmdStream, particlesHandle);
			core.recordBufferMemoryBarrier(cmdStream, simulation.getHandle());
			core.prepareImageForSampling(cmdStream, grid.getHandle());
			
			core.recordComputeDispatchToCmdStream(
					cmdStream,
					updateParticleVelocitiesPipeline,
					dispatchSizeParticles,
					{
						vkcv::useDescriptorSet(
								0, updateParticleVelocitiesSets[0]
						),
						vkcv::useDescriptorSet(
								1, updateParticleVelocitiesSets[1]
						),
						vkcv::useDescriptorSet(
								2, updateParticleVelocitiesSets[2]
						)
					},
					physicsPushConstants
			);
			
			core.recordBufferMemoryBarrier(cmdStream, particlesHandle);
			core.recordEndDebugLabel(cmdStream);
		}
		
		std::vector<vkcv::ImageHandle> renderTargets {
				vkcv::ImageHandle::createSwapchainImageHandle(),
				depthBuffer
		};
		
		if (renderGrid) {
			core.recordBeginDebugLabel(cmdStream, "RENDER GRID", { 0.13f, 0.20f, 0.22f, 1.0f });
			core.recordBufferMemoryBarrier(cmdStream, simulation.getHandle());
			core.prepareImageForSampling(cmdStream, grid.getHandle());
			
			core.recordDrawcallsToCmdStream(
					cmdStream,
					gfxPipelineGrid,
					cameraPushConstants,
					{ drawcallGrid },
					renderTargets,
					windowHandle
			);
			
			core.recordEndDebugLabel(cmdStream);
		} else {
			core.recordBeginDebugLabel(cmdStream, "RENDER PARTICLES", { 0.13f, 0.20f, 0.22f, 1.0f });
			core.recordBufferMemoryBarrier(cmdStream, particlesHandle);
			
			core.recordDrawcallsToCmdStream(
					cmdStream,
					gfxPipelineParticles,
					cameraPushConstants,
					{ drawcallParticle },
					renderTargets,
					windowHandle
			);
			
			core.recordEndDebugLabel(cmdStream);
		}
		
		core.recordBeginDebugLabel(cmdStream, "RENDER LINES", { 0.13f, 0.20f, 0.22f, 1.0f });
		
		core.recordDrawcallsToCmdStream(
				cmdStream,
				gfxPipelineLines,
				cameraPushConstants,
				{ drawcallLines },
				renderTargets,
				windowHandle
		);
		
		core.recordEndDebugLabel(cmdStream);
		
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		
		gui.beginGUI();
		ImGui::Begin("Settings");
		
		ImGui::BeginGroup();
		ImGui::Combo("Mode", &(sim->mode), "Random\0Ordered\0", 2);
		ImGui::Combo("Form", &(sim->form), "Sphere\0Cube\0", 2);
		ImGui::Combo("Type", &(sim->type), "Hyperelastic\0Fluid\0", 2);
		ImGui::EndGroup();
		
		ImGui::Spacing();
		
		ImGui::SliderInt("Particle Count", &(sim->count), 1, 100000);
		ImGui::SliderFloat("Density", &(sim->density), std::numeric_limits<float>::epsilon(), 5000.0f);
		ImGui::SameLine(0.0f, 10.0f);
		if (ImGui::SmallButton("Reset##density")) {
			sim->density = 2500.0f;
		}
		
		ImGui::SliderFloat("Radius", &(sim->size), 0.0f, 0.5f);
		ImGui::SameLine(0.0f, 10.0f);
		if (ImGui::SmallButton("Reset##radius")) {
			sim->size = 0.1f;
		}
		
		ImGui::Spacing();
		
		ImGui::BeginGroup();
		ImGui::SliderFloat("Bulk Modulus", &(sim->K), 0.0f, 1000.0f);
		ImGui::SliderFloat("Young's Modulus", &(sim->E), 0.0f, 1000.0f);
		ImGui::SliderFloat("Heat Capacity Ratio", &(sim->gamma), 1.0f, 2.0f);
		ImGui::SliderFloat("Lame1", &(sim->lame1), 0.0f, 1000.0f);
		ImGui::SliderFloat("Lame2", &(sim->lame2), 0.0f, 1000.0f);
		ImGui::EndGroup();

		ImGui::Spacing();

		ImGui::SliderFloat("Simulation Speed", &speed_factor, 0.0f, 2.0f);
		
		ImGui::Spacing();
		ImGui::Checkbox("Render Grid", &renderGrid);
		
		ImGui::DragFloat3("Initial Velocity", reinterpret_cast<float*>(&initialVelocity), 0.001f);
		ImGui::SameLine(0.0f, 10.0f);
		if (ImGui::Button("Reset##particle_velocity")) {
			particlesHandle = resetParticles(
					core,
					sim->count,
					initialVelocity,
					sim->density,
					sim->size,
					sim->form,
					sim->mode
			);
			
			vkcv::DescriptorWrites writes;
			writes.writeStorageBuffer(0, particlesHandle);
			
			core.writeDescriptorSet(initParticleWeightsSets[0], writes);
			core.writeDescriptorSet(transformParticlesToGridSets[0], writes);
			core.writeDescriptorSet(updateParticleVelocitiesSets[0], writes);
			
			core.writeDescriptorSet(gfxSetParticles, writes);
		}
		
		ImGui::SliderFloat("Gravity", &(sim->gravity), 0.0f, 10.0f);
		
		ImGui::End();
		gui.endGUI();
	});
	
	simulation.unmap();
	return 0;
}
