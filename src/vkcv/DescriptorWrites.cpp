
#include "vkcv/DescriptorWrites.hpp"

namespace vkcv {

	DescriptorWrites &DescriptorWrites::writeSampledImage(uint32_t binding,
														  const ImageHandle& image,
														  uint32_t mipLevel,
														  bool useGeneralLayout,
														  uint32_t arrayIndex,
														  uint32_t mipCount,
														  bool arrayView) {
		SampledImageDescriptorWrite write;
		write.binding = binding;
		write.image = image;
		write.mipLevel = mipLevel;
		write.useGeneralLayout = useGeneralLayout;
		write.arrayIndex = arrayIndex;
		write.mipCount = mipCount;
		write.arrayView = arrayView;
		m_sampledImageWrites.push_back(write);
		return *this;
	}

	DescriptorWrites &DescriptorWrites::writeStorageImage(uint32_t binding,
														  const ImageHandle& image,
														  uint32_t mipLevel,
														  uint32_t mipCount,
														  bool arrayView) {
		StorageImageDescriptorWrite write;
		write.binding = binding;
		write.image = image;
		write.mipLevel = mipLevel;
		write.mipCount = mipCount;
		write.arrayView = arrayView;
		m_storageImageWrites.push_back(write);
		return *this;
	}

	DescriptorWrites &DescriptorWrites::writeUniformBuffer(uint32_t binding,
														   const BufferHandle& buffer,
														   bool dynamic,
														   uint32_t offset,
														   uint32_t size) {
		BufferDescriptorWrite write;
		write.binding = binding;
		write.buffer = buffer;
		write.dynamic = dynamic;
		write.offset = offset;
		write.size = size;
		m_uniformBufferWrites.push_back(write);
		return *this;
	}

	DescriptorWrites &DescriptorWrites::writeStorageBuffer(uint32_t binding,
														   const BufferHandle& buffer,
														   bool dynamic,
														   uint32_t offset,
														   uint32_t size) {
		BufferDescriptorWrite write;
		write.binding = binding;
		write.buffer = buffer;
		write.dynamic = dynamic;
		write.offset = offset;
		write.size = size;
		m_storageBufferWrites.push_back(write);
		return *this;
	}

	DescriptorWrites &DescriptorWrites::writeSampler(uint32_t binding,
													 const SamplerHandle& sampler) {
		SamplerDescriptorWrite write;
		write.binding = binding;
		write.sampler = sampler;
		m_samplerWrites.push_back(write);
		return *this;
	}

	DescriptorWrites &DescriptorWrites::writeAcceleration(
		uint32_t binding, const std::vector<vk::AccelerationStructureKHR> &structures) {
		AccelerationDescriptorWrite write;
		write.binding = binding;
		write.structures = structures;
		m_accelerationWrites.push_back(write);
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
