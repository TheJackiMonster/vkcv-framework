#pragma once
/**
 * @authors Mara Vogt, Mark Mints, Tobias Frisch
 * @file vkcv/GraphicsPipelineConfig.hpp
 * @brief Graphics pipeline config struct to hand over required information to pipeline creation.
 */

#include <cstdint>
#include <vector>

#include "Multisampling.hpp"
#include "PipelineConfig.hpp"
#include "VertexLayout.hpp"

namespace vkcv {

	/**
	 * @brief Enum class to specify types of primitive topology.
	 */
	enum class PrimitiveTopology {
		PointList,
		LineList,
		TriangleList,
		PatchList
	};

	/**
	 * @brief Enum class to specify modes of culling.
	 */
	enum class CullMode {
		None,
		Front,
		Back,
		Both
	};

	/**
	 * @brief Enum class to specify depth-test modes.
	 */
	enum class DepthTest {
		None,
		Less,
		LessEqual,
		Greater,
		GreatherEqual,
		Equal
	};

	// add more as needed
	// alternatively we could expose the blend factors directly
	/**
	 * @brief Enum class to specify blending modes.
	 */
	enum class BlendMode {
		None,
		Additive
	};

	/**
	 * @brief Class to configure a graphics pipeline before its creation.
	 */
	class GraphicsPipelineConfig : public PipelineConfig {
	private:
		PassHandle m_PassHandle;
		VertexLayout m_VertexLayout;

		uint32_t m_Width;
		uint32_t m_Height;

		bool m_UseConservativeRasterization = false;
		PrimitiveTopology m_PrimitiveTopology = PrimitiveTopology::TriangleList;
		BlendMode m_blendMode = BlendMode::None;
		bool m_EnableDepthClamping = false;
		CullMode m_Culling = CullMode::None;
		DepthTest m_DepthTest = DepthTest::LessEqual;
		bool m_DepthWrite = true;
		bool m_AlphaToCoverage = false;
		uint32_t m_TessellationControlPoints = 0;

	public:
		GraphicsPipelineConfig();

		GraphicsPipelineConfig(const ShaderProgram &program,
							   const PassHandle &pass,
							   const VertexLayout &vertexLayout,
							   const std::vector<DescriptorSetLayoutHandle> &layouts);

		GraphicsPipelineConfig(const GraphicsPipelineConfig &other) = default;
		GraphicsPipelineConfig(GraphicsPipelineConfig &&other) = default;

		~GraphicsPipelineConfig() = default;

		GraphicsPipelineConfig &operator=(const GraphicsPipelineConfig &other) = default;
		GraphicsPipelineConfig &operator=(GraphicsPipelineConfig &&other) = default;

		[[nodiscard]] const PassHandle &getPass() const;

		[[nodiscard]] const VertexLayout &getVertexLayout() const;

		[[nodiscard]] uint32_t getWidth() const;

		[[nodiscard]] uint32_t getHeight() const;

		void setResolution(uint32_t width, uint32_t height);

		[[nodiscard]] bool isViewportDynamic() const;

		[[nodiscard]] bool isUsingConservativeRasterization() const;

		void setUsingConservativeRasterization(bool conservativeRasterization);

		[[nodiscard]] PrimitiveTopology getPrimitiveTopology() const;

		void setPrimitiveTopology(PrimitiveTopology primitiveTopology);

		[[nodiscard]] BlendMode getBlendMode() const;

		void setBlendMode(BlendMode blendMode);

		[[nodiscard]] bool isDepthClampingEnabled() const;

		void setDepthClampingEnabled(bool depthClamping);

		[[nodiscard]] CullMode getCulling() const;

		void setCulling(CullMode cullMode);

		[[nodiscard]] DepthTest getDepthTest() const;

		void setDepthTest(DepthTest depthTest);

		[[nodiscard]] bool isWritingDepth() const;

		void setWritingDepth(bool writingDepth);

		[[nodiscard]] bool isWritingAlphaToCoverage() const;

		void setWritingAlphaToCoverage(bool alphaToCoverage);

		[[nodiscard]] uint32_t getTesselationControlPoints() const;

		void setTesselationControlPoints(uint32_t tessellationControlPoints);
	};

} // namespace vkcv