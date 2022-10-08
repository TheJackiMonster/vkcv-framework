#include "Voxelization.hpp"

#include <vkcv/Pass.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

vkcv::ShaderProgram loadVoxelizationShader() {
	vkcv::shader::GLSLCompiler compiler;
	vkcv::ShaderProgram shader;
	compiler.compile(vkcv::ShaderStage::VERTEX, "assets/shaders/voxelization.vert",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shader.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::GEOMETRY, "assets/shaders/voxelization.geom",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shader.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, "assets/shaders/voxelization.frag",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shader.addShader(shaderStage, path);
	});
	return shader;
}

vkcv::ShaderProgram loadVoxelVisualisationShader() {
	vkcv::shader::GLSLCompiler compiler;
	vkcv::ShaderProgram shader;
	compiler.compile(vkcv::ShaderStage::VERTEX, "assets/shaders/voxelVisualisation.vert",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shader.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::GEOMETRY, "assets/shaders/voxelVisualisation.geom",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shader.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, "assets/shaders/voxelVisualisation.frag",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shader.addShader(shaderStage, path);
	});
	return shader;
}

vkcv::ShaderProgram loadVoxelResetShader() {
	vkcv::shader::GLSLCompiler compiler;
	vkcv::ShaderProgram shader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "assets/shaders/voxelReset.comp",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shader.addShader(shaderStage, path);
	});
	return shader;
}

vkcv::ShaderProgram loadVoxelBufferToImageShader() {
	vkcv::shader::GLSLCompiler compiler;
	vkcv::ShaderProgram shader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "assets/shaders/voxelBufferToImage.comp",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shader.addShader(shaderStage, path);
	});
	return shader;
}

vkcv::ShaderProgram loadSecondaryBounceShader() {
	vkcv::shader::GLSLCompiler compiler;
	vkcv::ShaderProgram shader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "assets/shaders/voxelSecondaryBounce.comp",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shader.addShader(shaderStage, path);
	});
	return shader;
}

const uint32_t      voxelResolution = 128;
uint32_t            voxelCount = voxelResolution * voxelResolution * voxelResolution;
const vk::Format    voxelizationDummyFormat = vk::Format::eR8Unorm;
const int           maxStableMip = 4;	// must be the same as in voxelConeTrace shader function

Voxelization::Voxelization(
	vkcv::Core* corePtr,
	const Dependencies& dependencies,
	vkcv::BufferHandle  lightInfoBuffer,
	vkcv::ImageHandle   shadowMap,
	vkcv::SamplerHandle shadowSampler,
	vkcv::SamplerHandle voxelSampler,
	vkcv::Multisampling msaa)
	:
	m_corePtr(corePtr),
	m_voxelImageIntermediate(vkcv::image(*m_corePtr, vk::Format::eR16G16B16A16Sfloat, voxelResolution, voxelResolution, voxelResolution, true, true)),
	m_voxelImage(vkcv::image(*m_corePtr, vk::Format::eR16G16B16A16Sfloat, voxelResolution, voxelResolution, voxelResolution, true, true)),
	m_voxelBuffer(buffer<VoxelBufferContent>(*m_corePtr, vkcv::BufferType::STORAGE, voxelCount)),
	m_dummyRenderTarget(vkcv::image(*m_corePtr, voxelizationDummyFormat, voxelResolution, voxelResolution, 1, false, false, true)),
	m_voxelInfoBuffer(buffer<VoxelizationInfo>(*m_corePtr, vkcv::BufferType::UNIFORM, 1)) {

	const vkcv::ShaderProgram voxelizationShader = loadVoxelizationShader();

	const vkcv::PassConfig voxelizationPassConfig {{
		  vkcv::AttachmentDescription(
				  voxelizationDummyFormat,
				  vkcv::AttachmentOperation::DONT_CARE,
				  vkcv::AttachmentOperation::DONT_CARE
		  )
	}, vkcv::Multisampling::None };
	m_voxelizationPass = m_corePtr->createPass(voxelizationPassConfig);

	m_voxelizationDescriptorSetLayout = m_corePtr->createDescriptorSetLayout(voxelizationShader.getReflectedDescriptors().at(0));
	m_voxelizationDescriptorSet = m_corePtr->createDescriptorSet(m_voxelizationDescriptorSetLayout);

	vkcv::DescriptorSetLayoutHandle dummyPerMeshDescriptorSetLayout = m_corePtr->createDescriptorSetLayout(voxelizationShader.getReflectedDescriptors().at(1));
	vkcv::DescriptorSetHandle dummyPerMeshDescriptorSet = m_corePtr->createDescriptorSet(dummyPerMeshDescriptorSetLayout);

	vkcv::GraphicsPipelineConfig voxelizationPipeConfig (
		voxelizationShader,
		m_voxelizationPass,
		dependencies.vertexLayout,
		{ m_voxelizationDescriptorSetLayout, dummyPerMeshDescriptorSetLayout }
	);
	
	voxelizationPipeConfig.setResolution(voxelResolution, voxelResolution);
	voxelizationPipeConfig.setUsingConservativeRasterization(true);
	
	m_voxelizationPipe = m_corePtr->createGraphicsPipeline(voxelizationPipeConfig);

	vkcv::DescriptorWrites voxelizationDescriptorWrites;
	voxelizationDescriptorWrites.writeStorageBuffer(0, m_voxelBuffer.getHandle());
	voxelizationDescriptorWrites.writeUniformBuffer(
			1, m_voxelInfoBuffer.getHandle()
	).writeUniformBuffer(
			3, lightInfoBuffer
	);
	
	voxelizationDescriptorWrites.writeSampledImage(4, shadowMap);
	voxelizationDescriptorWrites.writeSampler(5, shadowSampler);
	voxelizationDescriptorWrites.writeStorageImage(2, m_voxelImageIntermediate.getHandle());
	m_corePtr->writeDescriptorSet(m_voxelizationDescriptorSet, voxelizationDescriptorWrites);

	vkcv::ShaderProgram voxelVisualisationShader = loadVoxelVisualisationShader();

	m_visualisationDescriptorSetLayout = m_corePtr->createDescriptorSetLayout(voxelVisualisationShader.getReflectedDescriptors().at(0));
	m_visualisationDescriptorSet = m_corePtr->createDescriptorSet(m_visualisationDescriptorSetLayout);
	
	m_visualisationPass = vkcv::passFormats(
			*m_corePtr,
			{ dependencies.colorBufferFormat, dependencies.depthBufferFormat },
			false,
			msaa
	);

	vkcv::GraphicsPipelineConfig voxelVisualisationPipeConfig (
		voxelVisualisationShader,
		m_visualisationPass,
		{},
		{ m_visualisationDescriptorSetLayout }
	);	// points are extended to cubes in the geometry shader
	
	voxelVisualisationPipeConfig.setPrimitiveTopology(vkcv::PrimitiveTopology::PointList);
	m_visualisationPipe = m_corePtr->createGraphicsPipeline(voxelVisualisationPipeConfig);

	std::vector<uint16_t> voxelIndexData;
	for (uint32_t i = 0; i < voxelCount; i++) {
		voxelIndexData.push_back(static_cast<uint16_t>(i));
	}

	const auto voxelizationDescriptorUsage = vkcv::useDescriptorSet(0, m_visualisationDescriptorSet);

	vkcv::ShaderProgram resetVoxelShader = loadVoxelResetShader();

	m_voxelResetDescriptorSetLayout = m_corePtr->createDescriptorSetLayout(resetVoxelShader.getReflectedDescriptors().at(0));
	m_voxelResetDescriptorSet = m_corePtr->createDescriptorSet(m_voxelResetDescriptorSetLayout);
	m_voxelResetPipe = m_corePtr->createComputePipeline({
		resetVoxelShader,
		{ m_voxelResetDescriptorSetLayout }
	});

	vkcv::DescriptorWrites resetVoxelWrites;
	resetVoxelWrites.writeStorageBuffer(0, m_voxelBuffer.getHandle());
	m_corePtr->writeDescriptorSet(m_voxelResetDescriptorSet, resetVoxelWrites);

	// buffer to image
	vkcv::ShaderProgram bufferToImageShader = loadVoxelBufferToImageShader();

	m_bufferToImageDescriptorSetLayout = m_corePtr->createDescriptorSetLayout(bufferToImageShader.getReflectedDescriptors().at(0));
	m_bufferToImageDescriptorSet = m_corePtr->createDescriptorSet(m_bufferToImageDescriptorSetLayout);
	m_bufferToImagePipe = m_corePtr->createComputePipeline({
		bufferToImageShader,
		{ m_bufferToImageDescriptorSetLayout }
	});

	vkcv::DescriptorWrites bufferToImageDescriptorWrites;
	bufferToImageDescriptorWrites.writeStorageBuffer(0, m_voxelBuffer.getHandle());
	bufferToImageDescriptorWrites.writeStorageImage(1, m_voxelImageIntermediate.getHandle());
	m_corePtr->writeDescriptorSet(m_bufferToImageDescriptorSet, bufferToImageDescriptorWrites);

	// secondary bounce
	vkcv::ShaderProgram secondaryBounceShader = loadSecondaryBounceShader();

	m_secondaryBounceDescriptorSetLayout = m_corePtr->createDescriptorSetLayout(secondaryBounceShader.getReflectedDescriptors().at(0));
	m_secondaryBounceDescriptorSet = m_corePtr->createDescriptorSet(m_secondaryBounceDescriptorSetLayout);
	m_secondaryBouncePipe = m_corePtr->createComputePipeline({
		secondaryBounceShader,
		{ m_secondaryBounceDescriptorSetLayout }
	});

	vkcv::DescriptorWrites secondaryBounceDescriptorWrites;
	secondaryBounceDescriptorWrites.writeStorageBuffer(0, m_voxelBuffer.getHandle());
	secondaryBounceDescriptorWrites.writeSampledImage(1, m_voxelImageIntermediate.getHandle());
	secondaryBounceDescriptorWrites.writeSampler(2, voxelSampler);
	secondaryBounceDescriptorWrites.writeStorageImage(3, m_voxelImage.getHandle());
	secondaryBounceDescriptorWrites.writeUniformBuffer(4, m_voxelInfoBuffer.getHandle());
	m_corePtr->writeDescriptorSet(m_secondaryBounceDescriptorSet, secondaryBounceDescriptorWrites);
}

void Voxelization::voxelizeMeshes(
	vkcv::CommandStreamHandle                       cmdStream,
	const std::vector<vkcv::VertexData>&            meshes,
	const std::vector<glm::mat4>&                   modelMatrices,
	const std::vector<vkcv::DescriptorSetHandle>&   perMeshDescriptorSets,
	const vkcv::WindowHandle&                       windowHandle,
	vkcv::Downsampler&								downsampler) {

	m_voxelInfoBuffer.fill({ m_voxelInfo });

	const float voxelizationHalfExtent = 0.5f * m_voxelInfo.extent;
	const glm::mat4 voxelizationProjection = glm::ortho(
		-voxelizationHalfExtent,
		voxelizationHalfExtent,
		-voxelizationHalfExtent,
		voxelizationHalfExtent,
		-voxelizationHalfExtent,
		voxelizationHalfExtent);

	const glm::mat4 voxelizationView = glm::translate(glm::mat4(1.f), -m_voxelInfo.offset);
	const glm::mat4 voxelizationViewProjection = voxelizationProjection * voxelizationView;
	
	vkcv::PushConstants voxelizationPushConstants (2 * sizeof(glm::mat4));
	
	for (const auto& m : modelMatrices) {
		voxelizationPushConstants.appendDrawcall(std::array<glm::mat4, 2>{ voxelizationViewProjection * m, m });
	}

	// reset voxels
	const uint32_t resetVoxelGroupSize = 64;
	
	vkcv::PushConstants voxelCountPushConstants = vkcv::pushConstants<uint32_t>();
	voxelCountPushConstants.appendDrawcall(voxelCount);

	m_corePtr->recordBeginDebugLabel(cmdStream, "Voxel reset", { 1, 1, 1, 1 });
	m_corePtr->recordComputeDispatchToCmdStream(
		cmdStream,
		m_voxelResetPipe,
		vkcv::dispatchInvocations(voxelCount, resetVoxelGroupSize),
		{ vkcv::useDescriptorSet(0, m_voxelResetDescriptorSet) },
		voxelCountPushConstants
	);
	m_corePtr->recordBufferMemoryBarrier(cmdStream, m_voxelBuffer.getHandle());
	m_corePtr->recordEndDebugLabel(cmdStream);

	// voxelization
	std::vector<vkcv::InstanceDrawcall> drawcalls;
	for (size_t i = 0; i < meshes.size(); i++) {
		vkcv::InstanceDrawcall drawcall (meshes[i]);
		drawcall.useDescriptorSet(0, m_voxelizationDescriptorSet);
		drawcall.useDescriptorSet(1, perMeshDescriptorSets[i]);
		drawcalls.push_back(drawcall);
	}

	m_corePtr->recordBeginDebugLabel(cmdStream, "Voxelization", { 1, 1, 1, 1 });
	m_corePtr->prepareImageForStorage(cmdStream, m_voxelImageIntermediate.getHandle());
	m_corePtr->recordDrawcallsToCmdStream(
		cmdStream,
		m_voxelizationPipe,
		voxelizationPushConstants,
		drawcalls,
		{ m_dummyRenderTarget.getHandle() },
		windowHandle
	);
	m_corePtr->recordEndDebugLabel(cmdStream);

	// buffer to image
	const auto bufferToImageDispatchCount = vkcv::dispatchInvocations(
			vkcv::DispatchSize(voxelResolution, voxelResolution, voxelResolution),
			vkcv::DispatchSize(4, 4, 4)
	);
	
	m_corePtr->recordBeginDebugLabel(cmdStream, "Voxel buffer to image", { 1, 1, 1, 1 });
	m_corePtr->recordComputeDispatchToCmdStream(
		cmdStream,
		m_bufferToImagePipe,
		bufferToImageDispatchCount,
		{ vkcv::useDescriptorSet(0, m_bufferToImageDescriptorSet) },
		vkcv::PushConstants(0)
	);

	m_corePtr->recordImageMemoryBarrier(cmdStream, m_voxelImageIntermediate.getHandle());
	m_corePtr->recordEndDebugLabel(cmdStream);

	// intermediate image mipchain
	m_corePtr->recordBeginDebugLabel(cmdStream, "Intermediate Voxel mipmap generation", { 1, 1, 1, 1 });
	m_voxelImageIntermediate.recordMipChainGeneration(cmdStream, downsampler);
	m_corePtr->recordEndDebugLabel(cmdStream);

	// secondary bounce
	m_corePtr->recordBeginDebugLabel(cmdStream, "Voxel secondary bounce", { 1, 1, 1, 1 });
	m_corePtr->prepareImageForStorage(cmdStream, m_voxelImage.getHandle());
	m_corePtr->recordComputeDispatchToCmdStream(
		cmdStream,
		m_secondaryBouncePipe,
		bufferToImageDispatchCount,
		{ vkcv::useDescriptorSet(0, m_secondaryBounceDescriptorSet) },
		vkcv::PushConstants(0));
	m_voxelImage.recordMipChainGeneration(cmdStream, downsampler);
	m_corePtr->recordEndDebugLabel(cmdStream);

	// final image mipchain
	m_corePtr->recordBeginDebugLabel(cmdStream, "Voxel mipmap generation", { 1, 1, 1, 1 });
	m_voxelImage.recordMipChainGeneration(cmdStream, downsampler);
	m_corePtr->recordEndDebugLabel(cmdStream);
}

void Voxelization::renderVoxelVisualisation(
	vkcv::CommandStreamHandle               cmdStream, 
	const glm::mat4&                        viewProjectin,
	const std::vector<vkcv::ImageHandle>&   renderTargets,
	uint32_t                                mipLevel,
	const vkcv::WindowHandle&               windowHandle) {

	vkcv::PushConstants voxelVisualisationPushConstants = vkcv::pushConstants<glm::mat4>();
	voxelVisualisationPushConstants.appendDrawcall(viewProjectin);

	mipLevel = std::clamp(mipLevel, (uint32_t) 0, m_voxelImage.getMipLevels() - 1);

	// write descriptor set
	vkcv::DescriptorWrites voxelVisualisationDescriptorWrite;
	voxelVisualisationDescriptorWrite.writeStorageImage(0, m_voxelImage.getHandle(), mipLevel);
	voxelVisualisationDescriptorWrite.writeUniformBuffer(1, m_voxelInfoBuffer.getHandle());
	m_corePtr->writeDescriptorSet(m_visualisationDescriptorSet, voxelVisualisationDescriptorWrite);

	uint32_t drawVoxelCount = voxelCount / exp2(mipLevel);
	
	vkcv::VertexData voxelData;
	voxelData.setCount(drawVoxelCount);
	
	vkcv::InstanceDrawcall drawcall (voxelData);
	drawcall.useDescriptorSet(0, m_visualisationDescriptorSet);

	m_corePtr->recordBeginDebugLabel(cmdStream, "Voxel visualisation", { 1, 1, 1, 1 });
	m_corePtr->prepareImageForStorage(cmdStream, m_voxelImage.getHandle());
	m_corePtr->recordDrawcallsToCmdStream(
		cmdStream,
		m_visualisationPipe,
		voxelVisualisationPushConstants,
		{ drawcall },
		renderTargets,
		windowHandle
	);
	m_corePtr->recordEndDebugLabel(cmdStream);
}

void Voxelization::updateVoxelOffset(const vkcv::camera::Camera& camera) {

	// move voxel offset with camera in voxel sized steps
	const float voxelSize   = m_voxelInfo.extent / voxelResolution;
	const float snapSize    = voxelSize * exp2(maxStableMip);

	glm::vec3 voxelVolumeCenter = camera.getPosition() + (1.f / 3.f) * m_voxelInfo.extent * glm::normalize(camera.getFront());
	voxelVolumeCenter.y         = camera.getPosition().y;
	m_voxelInfo.offset          = glm::floor(voxelVolumeCenter / snapSize) * snapSize;
}

void Voxelization::setVoxelExtent(float extent) {
	m_voxelInfo.extent = extent;
}

vkcv::ImageHandle Voxelization::getVoxelImageHandle() const {
	return m_voxelImage.getHandle();
}

vkcv::BufferHandle Voxelization::getVoxelInfoBufferHandle() const {
	return m_voxelInfoBuffer.getHandle();
}

glm::vec3 Voxelization::getVoxelOffset() const{
	return m_voxelInfo.offset;
}

float Voxelization::getVoxelExtent() const {
	return m_voxelInfo.extent;
}
