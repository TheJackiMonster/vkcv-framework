#pragma once

#include <vulkan/vulkan.hpp>

#include "HandleManager.hpp"

#include "vkcv/Container.hpp"
#include "vkcv/Sampler.hpp"

namespace vkcv {

	/**
	 * @brief Class to manage the creation and destruction of samplers.
	 */
	class SamplerManager : public HandleManager<vk::Sampler, SamplerHandle> {
		friend class Core;

	private:
		[[nodiscard]] uint64_t getIdFrom(const SamplerHandle &handle) const override;

		[[nodiscard]] SamplerHandle createById(uint64_t id,
											   const HandleDestroyFunction &destroy) override;

		void destroyById(uint64_t id) override;

	public:
		SamplerManager() noexcept;

		~SamplerManager() noexcept override;

		SamplerHandle createSampler(SamplerFilterType magFilter, SamplerFilterType minFilter,
									SamplerMipmapMode mipmapMode, SamplerAddressMode addressMode,
									float mipLodBias, SamplerBorderColor borderColor);

		[[nodiscard]] vk::Sampler getVulkanSampler(const SamplerHandle &handle) const;
	};

} // namespace vkcv
