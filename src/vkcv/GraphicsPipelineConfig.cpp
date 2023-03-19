
#include "vkcv/GraphicsPipelineConfig.hpp"

namespace vkcv {

	GraphicsPipelineConfig::GraphicsPipelineConfig() :
		PipelineConfig(), m_PassHandle(), m_VertexLayout(),
		m_Width(std::numeric_limits<uint32_t>::max()),
		m_Height(std::numeric_limits<uint32_t>::max()) {}

	GraphicsPipelineConfig::GraphicsPipelineConfig(
		const ShaderProgram &program,
		const PassHandle &pass,
		const VertexLayout &vertexLayout,
		const Vector<DescriptorSetLayoutHandle> &layouts) :
		PipelineConfig(program, layouts),
		m_PassHandle(pass),
		m_VertexLayout(vertexLayout),
		m_Width(std::numeric_limits<uint32_t>::max()),
		m_Height(std::numeric_limits<uint32_t>::max()) {}

	const PassHandle &GraphicsPipelineConfig::getPass() const {
		return m_PassHandle;
	}

	const VertexLayout &GraphicsPipelineConfig::getVertexLayout() const {
		return m_VertexLayout;
	}

	uint32_t GraphicsPipelineConfig::getWidth() const {
		return m_Width;
	}

	uint32_t GraphicsPipelineConfig::getHeight() const {
		return m_Height;
	}

	void GraphicsPipelineConfig::setResolution(uint32_t width, uint32_t height) {
		m_Width = width;
		m_Height = height;
	}

	bool GraphicsPipelineConfig::isViewportDynamic() const {
		return ((m_Width == std::numeric_limits<uint32_t>::max())
				&& (m_Height == std::numeric_limits<uint32_t>::max()));
	}

	bool GraphicsPipelineConfig::isUsingConservativeRasterization() const {
		return m_UseConservativeRasterization;
	}

	void GraphicsPipelineConfig::setUsingConservativeRasterization(bool conservativeRasterization) {
		m_UseConservativeRasterization = conservativeRasterization;
	}

	PrimitiveTopology GraphicsPipelineConfig::getPrimitiveTopology() const {
		return m_PrimitiveTopology;
	}

	void GraphicsPipelineConfig::setPrimitiveTopology(PrimitiveTopology primitiveTopology) {
		m_PrimitiveTopology = primitiveTopology;
	}

	BlendMode GraphicsPipelineConfig::getBlendMode() const {
		return m_blendMode;
	}

	void GraphicsPipelineConfig::setBlendMode(BlendMode blendMode) {
		m_blendMode = blendMode;
	}

	bool GraphicsPipelineConfig::isDepthClampingEnabled() const {
		return m_EnableDepthClamping;
	}

	void GraphicsPipelineConfig::setDepthClampingEnabled(bool depthClamping) {
		m_EnableDepthClamping = depthClamping;
	}

	CullMode GraphicsPipelineConfig::getCulling() const {
		return m_Culling;
	}

	void GraphicsPipelineConfig::setCulling(CullMode cullMode) {
		m_Culling = cullMode;
	}

	DepthTest GraphicsPipelineConfig::getDepthTest() const {
		return m_DepthTest;
	}

	void GraphicsPipelineConfig::setDepthTest(DepthTest depthTest) {
		m_DepthTest = depthTest;
	}

	bool GraphicsPipelineConfig::isWritingDepth() const {
		return m_DepthWrite;
	}

	void GraphicsPipelineConfig::setWritingDepth(bool writingDepth) {
		m_DepthWrite = writingDepth;
	}

	bool GraphicsPipelineConfig::isWritingAlphaToCoverage() const {
		return m_AlphaToCoverage;
	}

	void GraphicsPipelineConfig::setWritingAlphaToCoverage(bool alphaToCoverage) {
		m_AlphaToCoverage = alphaToCoverage;
	}

	uint32_t GraphicsPipelineConfig::getTesselationControlPoints() const {
		return m_TessellationControlPoints;
	}

	void GraphicsPipelineConfig::setTesselationControlPoints(uint32_t tessellationControlPoints) {
		m_TessellationControlPoints = tessellationControlPoints;
	}

} // namespace vkcv
