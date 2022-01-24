#include "Voxelization.hpp"
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
	m_voxelImageIntermediate(m_corePtr->createImage(vk::Format::eR16G16B16A16Sfloat, voxelResolution, voxelResolution, voxelResolution, true, true)),
	m_voxelImage(m_corePtr->createImage(vk::Format::eR16G16B16A16Sfloat, voxelResolution, voxelResolution, voxelResolution, true, true)),
	m_voxelBuffer(m_corePtr->createBuffer<VoxelBufferContent>(vkcv::BufferType::STORAGE, voxelCount)),
	m_dummyRenderTarget(m_corePtr->createImage(voxelizationDummyFormat, voxelResolution, voxelResolution, 1, false, false, true)),
	m_voxelInfoBuffer(m_corePtr->createBuffer<VoxelizationInfo>(vkcv::BufferType::UNIFORM, 1)) {

	const vkcv::ShaderProgram voxelizationShader = loadVoxelizationShader();

	const vkcv::PassConfig voxelizationPassConfig({vkcv::AttachmentDescription(
		vkcv::AttachmentOperation::DONT_CARE, 
		vkcv::AttachmentOperation::DONT_CARE, 
		voxelizationDummyFormat) });
	m_voxelizationPass = m_corePtr->createPass(voxelizationPassConfig);

	m_voxelizationDescriptorSetLayout = m_corePtr->createDescriptorSetLayout(voxelizationShader.getReflectedDescriptors().at(0));
	m_voxelizationDescriptorSet = m_corePtr->createDescriptorSet(m_voxelizationDescriptorSetLayout);

	vkcv::DescriptorSetLayoutHandle dummyPerMeshDescriptorSetLayout = m_corePtr->createDescriptorSetLayout(voxelizationShader.getReflectedDescriptors().at(1));
	vkcv::DescriptorSetHandle dummyPerMeshDescriptorSet = m_corePtr->createDescriptorSet(dummyPerMeshDescriptorSetLayout);

	const vkcv::GraphicsPipelineConfig voxelizationPipeConfig{
		voxelizationShader,
		voxelResolution,
		voxelResolution,
		m_voxelizationPass,
		dependencies.vertexLayout,
		{ 
		    m_corePtr->getDescriptorSetLayout(m_voxelizationDescriptorSetLayout).vulkanHandle,
		    m_corePtr->getDescriptorSetLayout(dummyPerMeshDescriptorSetLayout).vulkanHandle},
		false,
		true };
	m_voxelizationPipe = m_corePtr->createGraphicsPipeline(voxelizationPipeConfig);

	vkcv::DescriptorWrites voxelizationDescriptorWrites;
	voxelizationDescriptorWrites.storageBufferWrites = { vkcv::BufferDescriptorWrite(0, m_voxelBuffer.getHandle()) };
	voxelizationDescriptorWrites.uniformBufferWrites = { 
		vkcv::BufferDescriptorWrite(1, m_voxelInfoBuffer.getHandle()),
		vkcv::BufferDescriptorWrite(3, lightInfoBuffer)
	};
	voxelizationDescriptorWrites.sampledImageWrites = { vkcv::SampledImageDescriptorWrite(4, shadowMap) };
	voxelizationDescriptorWrites.samplerWrites      = { vkcv::SamplerDescriptorWrite(5, shadowSampler) };
	voxelizationDescriptorWrites.storageImageWrites = { vkcv::StorageImageDescriptorWrite(2, m_voxelImageIntermediate.getHandle()) };
	m_corePtr->writeDescriptorSet(m_voxelizationDescriptorSet, voxelizationDescriptorWrites);

	vkcv::ShaderProgram voxelVisualisationShader = loadVoxelVisualisationShader();

	m_visualisationDescriptorSetLayout = m_corePtr->createDescriptorSetLayout(voxelVisualisationShader.getReflectedDescriptors().at(0));
	m_visualisationDescriptorSet = m_corePtr->createDescriptorSet(m_visualisationDescriptorSetLayout);

	const vkcv::AttachmentDescription voxelVisualisationColorAttachments(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::LOAD,
		dependencies.colorBufferFormat
	);

	const vkcv::AttachmentDescription voxelVisualisationDepthAttachments(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::LOAD,
		dependencies.depthBufferFormat
	);

	vkcv::PassConfig voxelVisualisationPassDefinition(
		{ voxelVisualisationColorAttachments, voxelVisualisationDepthAttachments });
	voxelVisualisationPassDefinition.msaa = msaa;
	m_visualisationPass = m_corePtr->createPass(voxelVisualisationPassDefinition);

	vkcv::GraphicsPipelineConfig voxelVisualisationPipeConfig{
		voxelVisualisationShader,
		0,
		0,
		m_visualisationPass,
		{},
		{ m_corePtr->getDescriptorSetLayout(m_visualisationDescriptorSetLayout).vulkanHandle },
		true,
		false,
		vkcv::PrimitiveTopology::PointList };	// points are extended to cubes in the geometry shader
	voxelVisualisationPipeConfig.m_multisampling = msaa;
	m_visualisationPipe = m_corePtr->createGraphicsPipeline(voxelVisualisationPipeConfig);

	std::vector<uint16_t> voxelIndexData;
	for (uint32_t i = 0; i < voxelCount; i++) {
		voxelIndexData.push_back(static_cast<uint16_t>(i));
	}

	const vkcv::DescriptorSetUsage voxelizationDescriptorUsage(0, m_corePtr->getDescriptorSet(m_visualisationDescriptorSet).vulkanHandle);

	vkcv::ShaderProgram resetVoxelShader = loadVoxelResetShader();

	m_voxelResetDescriptorSetLayout = m_corePtr->createDescriptorSetLayout(resetVoxelShader.getReflectedDescriptors().at(0));
	m_voxelResetDescriptorSet = m_corePtr->createDescriptorSet(m_voxelResetDescriptorSetLayout);
	m_voxelResetPipe = m_corePtr->createComputePipeline({
		resetVoxelShader,
		{ m_voxelResetDescriptorSetLayout }
	});

	vkcv::DescriptorWrites resetVoxelWrites;
	resetVoxelWrites.storageBufferWrites = { vkcv::BufferDescriptorWrite(0, m_voxelBuffer.getHandle()) };
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
	bufferToImageDescriptorWrites.storageBufferWrites = { vkcv::BufferDescriptorWrite(0, m_voxelBuffer.getHandle()) };
	bufferToImageDescriptorWrites.storageImageWrites = { vkcv::StorageImageDescriptorWrite(1, m_voxelImageIntermediate.getHandle()) };
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
	secondaryBounceDescriptorWrites.storageBufferWrites = { vkcv::BufferDescriptorWrite(0, m_voxelBuffer.getHandle()) };
	secondaryBounceDescriptorWrites.sampledImageWrites  = { vkcv::SampledImageDescriptorWrite(1, m_voxelImageIntermediate.getHandle()) };
	secondaryBounceDescriptorWrites.samplerWrites       = { vkcv::SamplerDescriptorWrite(2, voxelSampler) };
	secondaryBounceDescriptorWrites.storageImageWrites  = { vkcv::StorageImageDescriptorWrite(3, m_voxelImage.getHandle()) };
	secondaryBounceDescriptorWrites.uniformBufferWrites = { vkcv::BufferDescriptorWrite(4, m_voxelInfoBuffer.getHandle()) };
	m_corePtr->writeDescriptorSet(m_secondaryBounceDescriptorSet, secondaryBounceDescriptorWrites);
}

void Voxelization::voxelizeMeshes(
	vkcv::CommandStreamHandle                       cmdStream,
	const std::vector<vkcv::Mesh>&                  meshes,
	const std::vector<glm::mat4>&                   modelMatrices,
	const std::vector<vkcv::DescriptorSetHandle>&   perMeshDescriptorSets,
	const vkcv::WindowHandle&                       windowHandle) {

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
	uint32_t resetVoxelDispatchCount[3];
	resetVoxelDispatchCount[0] = glm::ceil(voxelCount / float(resetVoxelGroupSize));
	resetVoxelDispatchCount[1] = 1;
	resetVoxelDispatchCount[2] = 1;
	
	vkcv::PushConstants voxelCountPushConstants (sizeof(voxelCount));
	voxelCountPushConstants.appendDrawcall(voxelCount);

	m_corePtr->recordBeginDebugLabel(cmdStream, "Voxel reset", { 1, 1, 1, 1 });
	m_corePtr->recordComputeDispatchToCmdStream(
		cmdStream,
		m_voxelResetPipe,
		resetVoxelDispatchCount,
		{ vkcv::DescriptorSetUsage(0, m_corePtr->getDescriptorSet(m_voxelResetDescriptorSet).vulkanHandle) },
		voxelCountPushConstants);
	m_corePtr->recordBufferMemoryBarrier(cmdStream, m_voxelBuffer.getHandle());
	m_corePtr->recordEndDebugLabel(cmdStream);

	// voxelization
	std::vector<vkcv::DrawcallInfo> drawcalls;
	for (size_t i = 0; i < meshes.size(); i++) {
		drawcalls.push_back(vkcv::DrawcallInfo(
			meshes[i],
			{ 
				vkcv::DescriptorSetUsage(0, m_corePtr->getDescriptorSet(m_voxelizationDescriptorSet).vulkanHandle),
				vkcv::DescriptorSetUsage(1, m_corePtr->getDescriptorSet(perMeshDescriptorSets[i]).vulkanHandle) 
			},1));
	}

	m_corePtr->recordBeginDebugLabel(cmdStream, "Voxelization", { 1, 1, 1, 1 });
	m_corePtr->prepareImageForStorage(cmdStream, m_voxelImageIntermediate.getHandle());
	m_corePtr->recordDrawcallsToCmdStream(
		cmdStream,
		m_voxelizationPass,
		m_voxelizationPipe,
		voxelizationPushConstants,
		drawcalls,
		{ m_dummyRenderTarget.getHandle() },
		windowHandle);
	m_corePtr->recordEndDebugLabel(cmdStream);

	// buffer to image
	const uint32_t bufferToImageGroupSize[3] = { 4, 4, 4 };
	uint32_t bufferToImageDispatchCount[3];
	for (int i = 0; i < 3; i++) {
		bufferToImageDispatchCount[i] = glm::ceil(voxelResolution / float(bufferToImageGroupSize[i]));
	}

	m_corePtr->recordBeginDebugLabel(cmdStream, "Voxel buffer to image", { 1, 1, 1, 1 });
	m_corePtr->recordComputeDispatchToCmdStream(
		cmdStream,
		m_bufferToImagePipe,
		bufferToImageDispatchCount,
		{ vkcv::DescriptorSetUsage(0, m_corePtr->getDescriptorSet(m_bufferToImageDescriptorSet).vulkanHandle) },
		vkcv::PushConstants(0));

	m_corePtr->recordImageMemoryBarrier(cmdStream, m_voxelImageIntermediate.getHandle());
	m_corePtr->recordEndDebugLabel(cmdStream);

	// intermediate image mipchain
	m_corePtr->recordBeginDebugLabel(cmdStream, "Intermediate Voxel mipmap generation", { 1, 1, 1, 1 });
	m_voxelImageIntermediate.recordMipChainGeneration(cmdStream);
	m_corePtr->prepareImageForSampling(cmdStream, m_voxelImageIntermediate.getHandle());
	m_corePtr->recordEndDebugLabel(cmdStream);

	// secondary bounce
	m_corePtr->recordBeginDebugLabel(cmdStream, "Voxel secondary bounce", { 1, 1, 1, 1 });
	m_corePtr->prepareImageForStorage(cmdStream, m_voxelImage.getHandle());
	m_corePtr->recordComputeDispatchToCmdStream(
		cmdStream,
		m_secondaryBouncePipe,
		bufferToImageDispatchCount,
		{ vkcv::DescriptorSetUsage(0, m_corePtr->getDescriptorSet(m_secondaryBounceDescriptorSet).vulkanHandle) },
		vkcv::PushConstants(0));
	m_voxelImage.recordMipChainGeneration(cmdStream);
	m_corePtr->recordImageMemoryBarrier(cmdStream, m_voxelImage.getHandle());
	m_corePtr->recordEndDebugLabel(cmdStream);

	// final image mipchain
	m_corePtr->recordBeginDebugLabel(cmdStream, "Voxel mipmap generation", { 1, 1, 1, 1 });
	m_voxelImage.recordMipChainGeneration(cmdStream);
	m_corePtr->prepareImageForSampling(cmdStream, m_voxelImage.getHandle());
	m_corePtr->recordEndDebugLabel(cmdStream);
}

void Voxelization::renderVoxelVisualisation(
	vkcv::CommandStreamHandle               cmdStream, 
	const glm::mat4&                        viewProjectin,
	const std::vector<vkcv::ImageHandle>&   renderTargets,
	uint32_t                                mipLevel,
	const vkcv::WindowHandle&               windowHandle) {

	vkcv::PushConstants voxelVisualisationPushConstants (sizeof(glm::mat4));
	voxelVisualisationPushConstants.appendDrawcall(viewProjectin);

	mipLevel = std::clamp(mipLevel, (uint32_t)0, m_voxelImage.getMipCount()-1);

	// write descriptor set
	vkcv::DescriptorWrites voxelVisualisationDescriptorWrite;
	voxelVisualisationDescriptorWrite.storageImageWrites =
	{ vkcv::StorageImageDescriptorWrite(0, m_voxelImage.getHandle(), mipLevel) };
	voxelVisualisationDescriptorWrite.uniformBufferWrites =
	{ vkcv::BufferDescriptorWrite(1, m_voxelInfoBuffer.getHandle()) };
	m_corePtr->writeDescriptorSet(m_visualisationDescriptorSet, voxelVisualisationDescriptorWrite);

	uint32_t drawVoxelCount = voxelCount / exp2(mipLevel);

	const auto drawcall = vkcv::DrawcallInfo(
		vkcv::Mesh({}, nullptr, drawVoxelCount),
		{ vkcv::DescriptorSetUsage(0, m_corePtr->getDescriptorSet(m_visualisationDescriptorSet).vulkanHandle) },1);

	m_corePtr->recordBeginDebugLabel(cmdStream, "Voxel visualisation", { 1, 1, 1, 1 });
	m_corePtr->prepareImageForStorage(cmdStream, m_voxelImage.getHandle());
	m_corePtr->recordDrawcallsToCmdStream(
		cmdStream,
		m_visualisationPass,
		m_visualisationPipe,
		voxelVisualisationPushConstants,
		{ drawcall },
		renderTargets,
		windowHandle);
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
