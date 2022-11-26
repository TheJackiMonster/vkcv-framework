#pragma once

/**
 * @authors Tobias Frisch
 * @file src/vkcv/PipelineManager.hpp
 * @brief Creation and handling of ray tracing pipelines
 */

#include "BufferManager.hpp"
#include "DescriptorSetLayoutManager.hpp"
#include "HandleManager.hpp"
#include "PassManager.hpp"
#include "vkcv/RayTracingPipelineConfig.hpp"
#include <vector>
#include <vulkan/vulkan.hpp>

namespace vkcv {
	
	struct RayTracingPipelineEntry {
		vk::Pipeline m_handle;
		vk::PipelineLayout m_layout;
		RayTracingPipelineConfig m_config;
		BufferHandle m_shaderBindingTable;
		vk::StridedDeviceAddressRegionKHR m_rayGenAddress;
		vk::StridedDeviceAddressRegionKHR m_rayMissAddress;
		vk::StridedDeviceAddressRegionKHR m_rayHitAddress;
		vk::StridedDeviceAddressRegionKHR m_rayCallAddress;
	};
	
	/**
	 * @brief Class to manage ray tracing pipelines.
	 */
	class RayTracingPipelineManager :
			public HandleManager<RayTracingPipelineEntry, RayTracingPipelineHandle> {
	private:
		[[nodiscard]] uint64_t getIdFrom(const RayTracingPipelineHandle &handle) const override;
		
		[[nodiscard]] RayTracingPipelineHandle
		createById(uint64_t id, const HandleDestroyFunction &destroy) override;
		
		/**
		 * Destroys and deallocates ray tracing pipeline represented by a given
		 * ray tracing pipeline handle id.
		 *
		 * @param id Ray tracing pipeline handle id
		 */
		void destroyById(uint64_t id) override;
	
	public:
		RayTracingPipelineManager() noexcept;
		
		~RayTracingPipelineManager() noexcept override; // dtor
		
		RayTracingPipelineManager(const RayTracingPipelineManager &other) = delete; // copy-ctor
		RayTracingPipelineManager(RayTracingPipelineManager &&other) = delete;      // move-ctor;
		
		RayTracingPipelineManager &
		operator=(const RayTracingPipelineManager &other) = delete; // copy-assign op
		RayTracingPipelineManager &
		operator=(RayTracingPipelineManager &&other) = delete; // move-assign op
		
		/**
		 * Creates a ray tracing pipeline based on the set shader stages in the config struct.
		 * This function is wrapped in /src/vkcv/Core.cpp by Core::createRayTracingPipeline(const
		 * PipelineConfig &config). Therefore the passManager is filled already by the overall
		 * context of an application. On application level it is necessary first to fill a
		 * pipeline config struct.
		 *
		 * @param config Hands over all needed information for pipeline creation.
		 * @param descriptorManager Hands over the corresponding descriptor set layouts
		 * @param bufferManager Allows managing the shader binding table
		 * @return A handler to the created ray tracing pipeline object.
		 */
		RayTracingPipelineHandle createPipeline(const RayTracingPipelineConfig &config,
												const DescriptorSetLayoutManager &descriptorManager,
												BufferManager &bufferManager);
		
		/**
		 * Returns a vk::Pipeline object by handle.
		 *
		 * @param handle Directing to the requested pipeline.
		 * @return vk::Pipeline.
		 */
		[[nodiscard]] vk::Pipeline getVkPipeline(const RayTracingPipelineHandle &handle) const;
		
		/**
		 * Returns a vk::PipelineLayout object by handle.
		 *
		 * @param handle Directing to the requested pipeline.
		 * @return vk::PipelineLayout.
		 */
		[[nodiscard]] vk::PipelineLayout
		getVkPipelineLayout(const RayTracingPipelineHandle &handle) const;
		
		/**
		 * Returns the corresponding pipeline config struct of a pipeline object directed by the
		 * given handler.
		 *
		 * @param handle Directing to the requested pipeline.
		 * @return Pipeline config struct
		 */
		[[nodiscard]] const RayTracingPipelineConfig &
		getPipelineConfig(const RayTracingPipelineHandle &handle) const;
		
		[[nodiscard]] const vk::StridedDeviceAddressRegionKHR*
		getRayGenShaderBindingTableAddress(const RayTracingPipelineHandle &handle) const;
		
		[[nodiscard]] const vk::StridedDeviceAddressRegionKHR*
		getMissShaderBindingTableAddress(const RayTracingPipelineHandle &handle) const;
		
		[[nodiscard]] const vk::StridedDeviceAddressRegionKHR*
		getHitShaderBindingTableAddress(const RayTracingPipelineHandle &handle) const;
		
		[[nodiscard]] const vk::StridedDeviceAddressRegionKHR*
		getCallShaderBindingTableAddress(const RayTracingPipelineHandle &handle) const;
	};
	
} // namespace vkcv
