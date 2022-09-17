
#include "vkcv/DescriptorWrites.hpp"

namespace vkcv {

	DescriptorWrites &DescriptorWrites::writeSampledImage(uint32_t binding, ImageHandle image,
														  uint32_t mipLevel, bool useGeneralLayout,
														  uint32_t arrayIndex, uint32_t mipCount,
														  bool arrayView) {
		m_sampledImageWrites.emplace_back(binding, image, mipLevel, useGeneralLayout, arrayIndex,
										  mipCount, arrayView);

		return *this;
	}

	DescriptorWrites &DescriptorWrites::writeStorageImage(uint32_t binding, ImageHandle image,
														  uint32_t mipLevel, uint32_t mipCount,
														  bool arrayView) {
		m_storageImageWrites.emplace_back(binding, image, mipLevel, mipCount, arrayView);

		return *this;
	}

	DescriptorWrites &DescriptorWrites::writeUniformBuffer(uint32_t binding, BufferHandle buffer,
														   bool dynamic, uint32_t offset,
														   uint32_t size) {
		m_uniformBufferWrites.emplace_back(binding, buffer, dynamic, offset, size);
		return *this;
	}

	DescriptorWrites &DescriptorWrites::writeStorageBuffer(uint32_t binding, BufferHandle buffer,
														   bool dynamic, uint32_t offset,
														   uint32_t size) {
		m_storageBufferWrites.emplace_back(binding, buffer, dynamic, offset, size);
		return *this;
	}

	DescriptorWrites &DescriptorWrites::writeSampler(uint32_t binding, SamplerHandle sampler) {
		m_samplerWrites.emplace_back(binding, sampler);
		return *this;
	}

	DescriptorWrites &DescriptorWrites::writeAcceleration(
		uint32_t binding, const std::vector<vk::AccelerationStructureKHR> &structures) {
		m_accelerationWrites.emplace_back(binding, structures);
		return *this;
	}

	const std::vector<SampledImageDescriptorWrite> &
	DescriptorWrites::getSampledImageWrites() const {
		return m_sampledImageWrites;
	}

	const std::vector<StorageImageDescriptorWrite> &
	DescriptorWrites::getStorageImageWrites() const {
		return m_storageImageWrites;
	}

	const std::vector<BufferDescriptorWrite> &DescriptorWrites::getUniformBufferWrites() const {
		return m_uniformBufferWrites;
	}

	const std::vector<BufferDescriptorWrite> &DescriptorWrites::getStorageBufferWrites() const {
		return m_storageBufferWrites;
	}

	const std::vector<SamplerDescriptorWrite> &DescriptorWrites::getSamplerWrites() const {
		return m_samplerWrites;
	}

	const std::vector<AccelerationDescriptorWrite> &
	DescriptorWrites::getAccelerationWrites() const {
		return m_accelerationWrites;
	}

} // namespace vkcv
