
#include "SamplerManager.hpp"
#include "vkcv/Core.hpp"

namespace vkcv {

	uint64_t SamplerManager::getIdFrom(const SamplerHandle &handle) const {
		return handle.getId();
	}

	SamplerHandle SamplerManager::createById(uint64_t id, const HandleDestroyFunction &destroy) {
		return SamplerHandle(id, destroy);
	}

	void SamplerManager::destroyById(uint64_t id) {
		auto &sampler = getById(id);

		if (sampler) {
			getCore().getContext().getDevice().destroySampler(sampler);
			sampler = nullptr;
		}
	}

	SamplerManager::SamplerManager() noexcept : HandleManager<vk::Sampler, SamplerHandle>() {}

	SamplerManager::~SamplerManager() noexcept {
		clear();
	}

	SamplerHandle SamplerManager::createSampler(SamplerFilterType magFilter,
												SamplerFilterType minFilter,
												SamplerMipmapMode mipmapMode,
												SamplerAddressMode addressMode, float mipLodBias,
												SamplerBorderColor borderColor) {
		vk::Filter vkMagFilter;
		vk::Filter vkMinFilter;
		vk::SamplerMipmapMode vkMipmapMode;
		vk::SamplerAddressMode vkAddressMode;
		vk::BorderColor vkBorderColor;

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
		case SamplerAddressMode::CLAMP_TO_BORDER:
			vkAddressMode = vk::SamplerAddressMode::eClampToBorder;
			break;
		default:
			return SamplerHandle();
		}

		switch (borderColor) {
		case SamplerBorderColor::INT_ZERO_OPAQUE:
			vkBorderColor = vk::BorderColor::eIntOpaqueBlack;
			break;
		case SamplerBorderColor::INT_ZERO_TRANSPARENT:
			vkBorderColor = vk::BorderColor::eIntTransparentBlack;
			break;
		case SamplerBorderColor::FLOAT_ZERO_OPAQUE:
			vkBorderColor = vk::BorderColor::eFloatOpaqueBlack;
			break;
		case SamplerBorderColor::FLOAT_ZERO_TRANSPARENT:
			vkBorderColor = vk::BorderColor::eFloatTransparentBlack;
			break;
		case SamplerBorderColor::INT_ONE_OPAQUE:
			vkBorderColor = vk::BorderColor::eIntOpaqueWhite;
			break;
		case SamplerBorderColor::FLOAT_ONE_OPAQUE:
			vkBorderColor = vk::BorderColor::eFloatOpaqueWhite;
			break;
		default:
			return SamplerHandle();
		}

		const vk::SamplerCreateInfo samplerCreateInfo(
			vk::SamplerCreateFlags(), vkMagFilter, vkMinFilter, vkMipmapMode, vkAddressMode,
			vkAddressMode, vkAddressMode, mipLodBias, false, 16.0f, false, vk::CompareOp::eAlways,
			-1000.0f, 1000.0f, vkBorderColor, false);

		const vk::Sampler sampler =
			getCore().getContext().getDevice().createSampler(samplerCreateInfo);

		return add(sampler);
	}

	vk::Sampler SamplerManager::getVulkanSampler(const SamplerHandle &handle) const {
		return (*this) [handle];
	}

} // namespace vkcv
