
#include "SamplerManager.hpp"
#include "vkcv/Core.hpp"

namespace vkcv {
	
	SamplerManager::SamplerManager(const vk::Device& device) noexcept :
		m_device(device), m_samplers()
	{}
	
	SamplerManager::~SamplerManager() {
		for (uint64_t id = 0; id < m_samplers.size(); id++) {
			destroySamplerById(id);
		}
	}
	
	SamplerHandle SamplerManager::createSampler(SamplerFilterType magFilter,
												SamplerFilterType minFilter,
												SamplerMipmapMode mipmapMode,
												SamplerAddressMode addressMode) {
		vk::Filter vkMagFilter;
		vk::Filter vkMinFilter;
		vk::SamplerMipmapMode vkMipmapMode;
		vk::SamplerAddressMode vkAddressMode;
		
		switch (magFilter) {
			case SamplerFilterType::NEAREST:
				vkMagFilter = vk::Filter::eNearest;
				break;
			case SamplerFilterType::LINEAR:
				vkMagFilter = vk::Filter::eLinear;
				break;
			default:
				return SamplerHandle();
		}
		
		switch (minFilter) {
			case SamplerFilterType::NEAREST:
				vkMinFilter = vk::Filter::eNearest;
				break;
			case SamplerFilterType::LINEAR:
				vkMinFilter = vk::Filter::eLinear;
				break;
			default:
				return SamplerHandle();
		}
		
		switch (mipmapMode) {
			case SamplerMipmapMode::NEAREST:
				vkMipmapMode = vk::SamplerMipmapMode::eNearest;
				break;
			case SamplerMipmapMode::LINEAR:
				vkMipmapMode = vk::SamplerMipmapMode::eLinear;
				break;
			default:
				return SamplerHandle();
		}
		
		switch (addressMode) {
			case SamplerAddressMode::REPEAT:
				vkAddressMode = vk::SamplerAddressMode::eRepeat;
				break;
			case SamplerAddressMode::MIRRORED_REPEAT:
				vkAddressMode = vk::SamplerAddressMode::eMirroredRepeat;
				break;
			case SamplerAddressMode::CLAMP_TO_EDGE:
				vkAddressMode = vk::SamplerAddressMode::eClampToEdge;
				break;
			case SamplerAddressMode::MIRROR_CLAMP_TO_EDGE:
				vkAddressMode = vk::SamplerAddressMode::eMirrorClampToEdge;
				break;
			default:
				return SamplerHandle();
		}
		
		const vk::SamplerCreateInfo samplerCreateInfo (
				vk::SamplerCreateFlags(),
				vkMagFilter,
				vkMinFilter,
				vkMipmapMode,
				vkAddressMode,
				vkAddressMode,
				vkAddressMode,
				0.0f,
				false,
				16.0f,
				false,
				vk::CompareOp::eAlways,
				0.0f,
				1.0f,
				vk::BorderColor::eIntOpaqueBlack,
				false
		);
		
		const vk::Sampler sampler = m_device.createSampler(samplerCreateInfo);
		
		const uint64_t id = m_samplers.size();
		m_samplers.push_back(sampler);
		return SamplerHandle(id, [&](uint64_t id) { destroySamplerById(id); });
	}
	
	vk::Sampler SamplerManager::getVulkanSampler(const SamplerHandle &handle) const {
		const uint64_t id = handle.getId();
		
		if (id >= m_samplers.size()) {
			return nullptr;
		}
		
		return m_samplers[id];
	}
	
	void SamplerManager::destroySamplerById(uint64_t id) {
		if (id >= m_samplers.size()) {
			return;
		}
		
		auto& sampler = m_samplers[id];
		
		if (sampler) {
			m_device.destroySampler(sampler);
			sampler = nullptr;
		}
	}

}
