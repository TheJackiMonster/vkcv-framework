#include "Voxelization.hpp"
#include <vkcv/shader/GLSLCompiler.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

vkcv::ShaderProgram loadVoxelizationShader() {
	vkcv::shader::GLSLCompiler compiler;
	vkcv::ShaderProgram shader;
	compiler.compile(vkcv::ShaderStage::VERTEX, "resources/shaders/voxelization.vert",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shader.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::GEOMETRY, "resources/shaders/voxelization.geom",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shader.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, "resources/shaders/voxelization.frag",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shader.addShader(shaderStage, path);
	});
	return shader;
}

vkcv::ShaderProgram loadVoxelVisualisationShader() {
	vkcv::shader::GLSLCompiler compiler;
	vkcv::ShaderProgram shader;
	compiler.compile(vkcv::ShaderStage::VERTEX, "resources/shaders/voxelVisualisation.vert",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shader.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::GEOMETRY, "resources/shaders/voxelVisualisation.geom",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shader.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, "resources/shaders/voxelVisualisation.frag",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shader.addShader(shaderStage, path);
	});
	return shader;
}

vkcv::ShaderProgram loadVoxelResetShader() {
	vkcv::shader::GLSLCompiler compiler;
	vkcv::ShaderProgram shader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "resources/shaders/voxelReset.comp",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shader.addShader(shaderStage, path);
	});
	return shader;
}

vkcv::ShaderProgram loadVoxelBufferToImageShader() {
	vkcv::shader::GLSLCompiler compiler;
	vkcv::ShaderProgram shader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "resources/shaders/voxelBufferToImage.comp",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shader.addShader(shaderStage, path);
	});
	return shader;
}

const uint32_t voxelResolution = 128;
uint32_t voxelCount = voxelResolution * voxelResolution * voxelResolution;
const vk::Format voxelizationDummyFormat = vk::Format::eR8Unorm;

Voxelization::Voxelization(
	vkcv::Core* corePtr,
	const Dependencies& dependencies,
	vkcv::BufferHandle  lightInfoBuffer,
	vkcv::ImageHandle   shadowMap,
	vkcv::SamplerHandle shadowSampler)
	:
	m_corePtr(corePtr), 
	m_voxelImage(m_corePtr->createImage(vk::Format::eR16G16B16A16Sfloat, voxelResolution, voxelResolution, voxelResolution, true, true)),
	m_dummyRenderTarget(m_corePtr->createImage(voxelizationDummyFormat, voxelResolution, voxelResolution, 1, false, false, true)),
	m_voxelInfoBuffer(m_corePtr->createBuffer<VoxelizationInfo>(vkcv::BufferType::UNIFORM, 1)),
	m_voxelBuffer(m_corePtr->createBuffer<VoxelBufferContent>(vkcv::BufferType::STORAGE, voxelCount)){

	const vkcv::ShaderProgram voxelizationShader = loadVoxelizationShader();

	const vkcv::PassConfig voxelizationPassConfig({vkcv::AttachmentDescription(
		vkcv::AttachmentOperation::DONT_CARE, 
		vkcv::AttachmentOperation::DONT_CARE, 
		voxelizationDummyFormat) });
	m_voxelizationPass = m_corePtr->createPass(voxelizationPassConfig);

	std::vector<vkcv::DescriptorBinding> voxelizationDescriptorBindings = 
	{ voxelizationShader.getReflectedDescriptors()[0] };
	m_voxelizationDescriptorSet = m_corePtr->createDescriptorSet(voxelizationDescriptorBindings);

	vkcv::DescriptorSetHandle dummyPerMeshDescriptorSet =
		m_corePtr->createDescriptorSet({ voxelizationShader.getReflectedDescriptors()[1] });

	const vkcv::PipelineConfig voxelizationPipeConfig{
		voxelizationShader,
		voxelResolution,
		voxelResolution,
		m_voxelizationPass,
		dependencies.vertexLayout,
		{ 
			m_corePtr->getDescriptorSet(m_voxelizationDescriptorSet).layout,
			m_corePtr->getDescriptorSet(dummyPerMeshDescriptorSet).layout},
		false,
		true };
	m_voxelizationPipe = m_corePtr->createGraphicsPipeline(voxelizationPipeConfig);

	vkcv::DescriptorWrites voxelizationDescriptorWrites;
	voxelizationDescriptorWrites.storageBufferWrites = { vkcv::StorageBufferDescriptorWrite(0, m_voxelBuffer.getHandle()) };
	voxelizationDescriptorWrites.uniformBufferWrites = { 
		vkcv::UniformBufferDescriptorWrite(1, m_voxelInfoBuffer.getHandle()),
		vkcv::UniformBufferDescriptorWrite(3, lightInfoBuffer)
	};
	voxelizationDescriptorWrites.sampledImageWrites = { vkcv::SampledImageDescriptorWrite(4, shadowMap) };
	voxelizationDescriptorWrites.samplerWrites      = { vkcv::SamplerDescriptorWrite(5, shadowSampler) };
	voxelizationDescriptorWrites.storageImageWrites = { vkcv::StorageImageDescriptorWrite(2, m_voxelImage.getHandle()) };
	m_corePtr->writeDescriptorSet(m_voxelizationDescriptorSet, voxelizationDescriptorWrites);

	vkcv::ShaderProgram voxelVisualisationShader = loadVoxelVisualisationShader();

	const std::vector<vkcv::DescriptorBinding> voxelVisualisationDescriptorBindings = 
		{ voxelVisualisationShader.getReflectedDescriptors()[0] };
	m_visualisationDescriptorSet = m_corePtr->createDescriptorSet(voxelVisualisationDescriptorBindings);

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
	m_visualisationPass = m_corePtr->createPass(voxelVisualisationPassDefinition);

	const vkcv::PipelineConfig voxelVisualisationPipeConfig{
		voxelVisualisationShader,
		0,
		0,
		m_visualisationPass,
		{},
		{ m_corePtr->getDescriptorSet(m_visualisationDescriptorSet).layout },
		true,
		false,
		vkcv::PrimitiveTopology::PointList };	// points are extended to cubes in the geometry shader
	m_visualisationPipe = m_corePtr->createGraphicsPipeline(voxelVisualisationPipeConfig);

	std::vector<uint16_t> voxelIndexData;
	for (int i = 0; i < voxelCount; i++) {
		voxelIndexData.push_back(i);
	}

	const vkcv::DescriptorSetUsage voxelizationDescriptorUsage(0, m_corePtr->getDescriptorSet(m_visualisationDescriptorSet).vulkanHandle);

	vkcv::ShaderProgram resetVoxelShader = loadVoxelResetShader();

	m_voxelResetDescriptorSet = m_corePtr->createDescriptorSet(resetVoxelShader.getReflectedDescriptors()[0]);
	m_voxelResetPipe = m_corePtr->createComputePipeline(
		resetVoxelShader,
		{ m_corePtr->getDescriptorSet(m_voxelResetDescriptorSet).layout });

	vkcv::DescriptorWrites resetVoxelWrites;
	resetVoxelWrites.storageBufferWrites = { vkcv::StorageBufferDescriptorWrite(0, m_voxelBuffer.getHandle()) };
	m_corePtr->writeDescriptorSet(m_voxelResetDescriptorSet, resetVoxelWrites);


	vkcv::ShaderProgram bufferToImageShader = loadVoxelBufferToImageShader();

	m_bufferToImageDescriptorSet = m_corePtr->createDescriptorSet(bufferToImageShader.getReflectedDescriptors()[0]);
	m_bufferToImagePipe = m_corePtr->createComputePipeline(
		bufferToImageShader,
		{ m_corePtr->getDescriptorSet(m_bufferToImageDescriptorSet).layout });

	vkcv::DescriptorWrites bufferToImageDescriptorWrites;
	bufferToImageDescriptorWrites.storageBufferWrites = { vkcv::StorageBufferDescriptorWrite(0, m_voxelBuffer.getHandle()) };
	bufferToImageDescriptorWrites.storageImageWrites = { vkcv::StorageImageDescriptorWrite(1, m_voxelImage.getHandle()) };
	m_corePtr->writeDescriptorSet(m_bufferToImageDescriptorSet, bufferToImageDescriptorWrites);
}

void Voxelization::voxelizeMeshes(
	vkcv::CommandStreamHandle                       cmdStream, 
	const glm::vec3&                                cameraPosition, 
	const std::vector<vkcv::Mesh>&                  meshes,
	const std::vector<glm::mat4>&                   modelMatrices,
	const std::vector<vkcv::DescriptorSetHandle>&   perMeshDescriptorSets) {

	VoxelizationInfo voxelizationInfo;
	voxelizationInfo.extent = m_voxelExtent;

	// move voxel offset with camera in voxel sized steps
	const float voxelSize = m_voxelExtent / voxelResolution;
	voxelizationInfo.offset = glm::floor(cameraPosition / voxelSize) * voxelSize;

	m_voxelInfoBuffer.fill({ voxelizationInfo });

	const float voxelizationHalfExtent = 0.5f * m_voxelExtent;
	const glm::mat4 voxelizationProjection = glm::ortho(
		-voxelizationHalfExtent,
		voxelizationHalfExtent,
		-voxelizationHalfExtent,
		voxelizationHalfExtent,
		-voxelizationHalfExtent,
		voxelizationHalfExtent);

	const glm::mat4 voxelizationView = glm::translate(glm::mat4(1.f), -voxelizationInfo.offset);
	const glm::mat4 voxelizationViewProjection = voxelizationProjection * voxelizationView;

	std::vector<std::array<glm::mat4, 2>> voxelizationMatrices;
	for (const auto& m : modelMatrices) {
		voxelizationMatrices.push_back({ voxelizationViewProjection * m, m });
	}

	const vkcv::PushConstantData voxelizationPushConstantData((void*)voxelizationMatrices.data(), 2 * sizeof(glm::mat4));

	// reset voxels
	const uint32_t resetVoxelGroupSize = 64;
	uint32_t resetVoxelDispatchCount[3];
	resetVoxelDispatchCount[0] = glm::ceil(voxelCount / float(resetVoxelGroupSize));
	resetVoxelDispatchCount[1] = 1;
	resetVoxelDispatchCount[2] = 1;

	m_corePtr->prepareImageForStorage(cmdStream, m_voxelImage.getHandle());
	m_corePtr->recordComputeDispatchToCmdStream(
		cmdStream,
		m_voxelResetPipe,
		resetVoxelDispatchCount,
		{ vkcv::DescriptorSetUsage(0, m_corePtr->getDescriptorSet(m_voxelResetDescriptorSet).vulkanHandle) },
		vkcv::PushConstantData(&voxelCount, sizeof(voxelCount)));
	m_corePtr->recordBufferMemoryBarrier(cmdStream, m_voxelBuffer.getHandle());

	// voxelization
	std::vector<vkcv::DrawcallInfo> drawcalls;
	for (int i = 0; i < meshes.size(); i++) {
		drawcalls.push_back(vkcv::DrawcallInfo(
			meshes[i], 
			{ 
				vkcv::DescriptorSetUsage(0, m_corePtr->getDescriptorSet(m_voxelizationDescriptorSet).vulkanHandle),
				vkcv::DescriptorSetUsage(1, m_corePtr->getDescriptorSet(perMeshDescriptorSets[i]).vulkanHandle) 
			}));
	}

	m_corePtr->recordDrawcallsToCmdStream(
		cmdStream,
		m_voxelizationPass,
		m_voxelizationPipe,
		voxelizationPushConstantData,
		drawcalls,
		{ m_dummyRenderTarget.getHandle() });

	// buffer to image
	const uint32_t bufferToImageGroupSize[3] = { 4, 4, 4 };
	uint32_t bufferToImageDispatchCount[3];
	for (int i = 0; i < 3; i++) {
		bufferToImageDispatchCount[i] = glm::ceil(voxelResolution / float(bufferToImageGroupSize[i]));
	}

	m_corePtr->recordComputeDispatchToCmdStream(
		cmdStream,
		m_bufferToImagePipe,
		bufferToImageDispatchCount,
		{ vkcv::DescriptorSetUsage(0, m_corePtr->getDescriptorSet(m_bufferToImageDescriptorSet).vulkanHandle) },
		vkcv::PushConstantData(nullptr, 0));

	m_corePtr->recordImageMemoryBarrier(cmdStream, m_voxelImage.getHandle());

	m_voxelImage.recordMipChainGeneration(cmdStream);
}

void Voxelization::renderVoxelVisualisation(
	vkcv::CommandStreamHandle               cmdStream, 
	const glm::mat4&                        viewProjectin,
	const std::vector<vkcv::ImageHandle>&   renderTargets,
	uint32_t                                mipLevel) {

	const vkcv::PushConstantData voxelVisualisationPushConstantData((void*)&viewProjectin, sizeof(glm::mat4));

	mipLevel = std::clamp(mipLevel, (uint32_t)0, m_voxelImage.getMipCount()-1);

	// write descriptor set
	vkcv::DescriptorWrites voxelVisualisationDescriptorWrite;
	voxelVisualisationDescriptorWrite.storageImageWrites =
	{ vkcv::StorageImageDescriptorWrite(0, m_voxelImage.getHandle(), mipLevel) };
	voxelVisualisationDescriptorWrite.uniformBufferWrites =
	{ vkcv::UniformBufferDescriptorWrite(1, m_voxelInfoBuffer.getHandle()) };
	m_corePtr->writeDescriptorSet(m_visualisationDescriptorSet, voxelVisualisationDescriptorWrite);

	uint32_t drawVoxelCount = voxelCount / exp2(mipLevel);

	const auto drawcall = vkcv::DrawcallInfo(
		vkcv::Mesh({}, nullptr, drawVoxelCount),
		{ vkcv::DescriptorSetUsage(0, m_corePtr->getDescriptorSet(m_visualisationDescriptorSet).vulkanHandle) });

	m_corePtr->recordDrawcallsToCmdStream(
		cmdStream,
		m_visualisationPass,
		m_visualisationPipe,
		voxelVisualisationPushConstantData,
		{ drawcall },
		renderTargets);
}