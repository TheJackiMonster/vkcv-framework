#pragma once

/**
 * @authors Mark Mints, Tobias Frisch
 * @file src/vkcv/ComputePipelineManager.hpp
 * @brief Creation and handling of Compute Pipelines
 */

#include <vector>
#include <vulkan/vulkan.hpp>

#include "HandleManager.hpp"

#include "vkcv/ComputePipelineConfig.hpp"
#include "vkcv/ShaderProgram.hpp"

namespace vkcv {

	struct ComputePipelineEntry {
		vk::Pipeline m_handle;
		vk::PipelineLayout m_layout;
	};

	/**
	 * @brief Class to manage compute pipelines.
	 */
	class ComputePipelineManager :
		public HandleManager<ComputePipelineEntry, ComputePipelineHandle> {
	private:
		[[nodiscard]] uint64_t getIdFrom(const ComputePipelineHandle &handle) const override;

		[[nodiscard]] ComputePipelineHandle
		createById(uint64_t id, const HandleDestroyFunction &destroy) override;

		/**
		 * Destroys and deallocates compute pipeline represented by a given
		 * compute pipeline handle id.
		 *
		 * @param id Compute pipeline handle id
		 */
		void destroyById(uint64_t id) override;

		vk::Result createShaderModule(vk::ShaderModule &module, const ShaderProgram &shaderProgram,
									  ShaderStage stage);

	public:
		ComputePipelineManager() noexcept;

		~ComputePipelineManager() noexcept override; // dtor

		/**
		 * Returns a vk::Pipeline object by handle.
		 * @param handle Directing to the requested pipeline.
		 * @return vk::Pipeline.
		 */
		[[nodiscard]] vk::Pipeline getVkPipeline(const ComputePipelineHandle &handle) const;

		/**
		 * Returns a vk::PipelineLayout object by handle.
		 * @param handle Directing to the requested pipeline.
		 * @return vk::PipelineLayout.
		 */
		[[nodiscard]] vk::PipelineLayout
		getVkPipelineLayout(const ComputePipelineHandle &handle) const;

		/**
		 * Creates a Compute Pipeline based on the set shader stages in the Config Struct.
		 * This function is wrapped in /src/vkcv/Core.cpp by Core::createComputePipeline(const
		 * ComputePipelineConfig &config). On application level it is necessary first to fill a
		 * ComputePipelineConfig Struct.
		 * @param shaderProgram Hands over all needed information for pipeline creation.
		 * @param descriptorSetLayouts Hands over all needed information for pipeline creation.
		 * @return A Handler to the created Compute Pipeline Object.
		 */
		ComputePipelineHandle
		createComputePipeline(const ShaderProgram &shaderProgram,
							  const std::vector<vk::DescriptorSetLayout> &descriptorSetLayouts);
	};

} // namespace vkcv
