#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

#include "vkcv/Handles.hpp"
#include "vkcv/Sampler.hpp"

namespace vkcv {
	
	class Core;
	
	/**
	 * @brief Class to manage the creation and destruction of samplers.
	 */
	class SamplerManager {
		friend class Core;
	private:
		vk::Device m_device;
		std::vector<vk::Sampler> m_samplers;
		
		explicit SamplerManager(const vk::Device& device) noexcept;
		
		void destroySamplerById(uint64_t id);
		
	public:
		~SamplerManager();
		
		SamplerManager(const SamplerManager& other) = delete;
		SamplerManager(SamplerManager&& other) = delete;
		
		SamplerManager& operator=(const SamplerManager& other) = delete;
		SamplerManager& operator=(SamplerManager&& other) = delete;
		
		SamplerHandle createSampler(SamplerFilterType magFilter,
									SamplerFilterType minFilter,
									SamplerMipmapMode mipmapMode,
									SamplerAddressMode addressMode,
									float mipLodBias,
									SamplerBorderColor borderColor);
		
		[[nodiscard]]
		vk::Sampler getVulkanSampler(const SamplerHandle& handle) const;
	
	};
	
}
