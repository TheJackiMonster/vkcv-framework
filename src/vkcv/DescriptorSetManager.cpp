#include "DescriptorSetManager.hpp"

#include "vkcv/Core.hpp"

namespace vkcv {

	bool DescriptorSetManager::init(Core &core) {
		return HandleManager<DescriptorSetEntry, DescriptorSetHandle>::init(core);
	}

	bool DescriptorSetManager::init(Core &core,
									DescriptorSetLayoutManager &descriptorSetLayoutManager) {
		if (!init(core)) {
			return false;
		}

		m_DescriptorSetLayoutManager = &descriptorSetLayoutManager;

		/**
		 * Allocate the set size for the descriptor pools, namely 1000 units of each descriptor type
		 * below. Finally, create an initial pool.
		 */
		m_PoolSizes.clear();
		m_PoolSizes.emplace_back(vk::DescriptorType::eSampler, 1000);
		m_PoolSizes.emplace_back(vk::DescriptorType::eSampledImage, 1000);
		m_PoolSizes.emplace_back(vk::DescriptorType::eUniformBuffer, 1000);
		m_PoolSizes.emplace_back(vk::DescriptorType::eStorageBuffer, 1000);
		m_PoolSizes.emplace_back(vk::DescriptorType::eUniformBufferDynamic, 1000);
		m_PoolSizes.emplace_back(vk::DescriptorType::eStorageBufferDynamic, 1000);

		if (core.getContext().getFeatureManager().isExtensionActive(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME)) {
			m_PoolSizes.emplace_back(vk::DescriptorType::eAccelerationStructureKHR, 1000);
		}

		m_PoolInfo = vk::DescriptorPoolCreateInfo(
			vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1000,
			static_cast<uint32_t>(m_PoolSizes.size()), m_PoolSizes.data()
		);

		return allocateDescriptorPool();
	}

	uint64_t DescriptorSetManager::getIdFrom(const DescriptorSetHandle &handle) const {
		return handle.getId();
	}

	DescriptorSetHandle DescriptorSetManager::createById(uint64_t id,
														 const HandleDestroyFunction &destroy) {
		return DescriptorSetHandle(id, destroy);
	}

	void DescriptorSetManager::destroyById(uint64_t id) {
		auto &set = getById(id);

		if (set.vulkanHandle) {
			getCore().getContext().getDevice().freeDescriptorSets(m_Pools [set.poolIndex], 1,
																  &(set.vulkanHandle));
			set.vulkanHandle = nullptr;
		}

		set.setLayoutHandle = DescriptorSetLayoutHandle();
	}

	bool DescriptorSetManager::allocateDescriptorPool() {
		vk::DescriptorPool pool;
		if (getCore().getContext().getDevice().createDescriptorPool(&m_PoolInfo, nullptr, &pool)
			!= vk::Result::eSuccess) {
			vkcv_log(LogLevel::WARNING, "Failed to allocate descriptor pool");
			return false;
		} else {
			m_Pools.push_back(pool);
			return true;
		}
	}

	DescriptorSetManager::DescriptorSetManager() noexcept :
		HandleManager<DescriptorSetEntry, DescriptorSetHandle>() {}

	DescriptorSetManager::~DescriptorSetManager() noexcept {
		clear();

		for (const auto &pool : m_Pools) {
			if (pool) {
				getCore().getContext().getDevice().destroy(pool);
			}
		}
	}

	DescriptorSetHandle
	DescriptorSetManager::createDescriptorSet(const DescriptorSetLayoutHandle &layout) {
		// create and allocate the set based on the layout provided
		const auto &setLayout = m_DescriptorSetLayoutManager->getDescriptorSetLayout(layout);

		vk::DescriptorSet vulkanHandle;
		vk::DescriptorSetAllocateInfo allocInfo(m_Pools.back(), 1, &setLayout.vulkanHandle);

		uint32_t sumVariableDescriptorCounts = 0;
		for (auto bindingElem : setLayout.descriptorBindings) {
			auto binding = bindingElem.second;

			if (binding.variableCount)
				sumVariableDescriptorCounts += binding.descriptorCount;
		}

		vk::DescriptorSetVariableDescriptorCountAllocateInfo variableAllocInfo(
			1, &sumVariableDescriptorCounts);

		if (sumVariableDescriptorCounts > 0) {
			allocInfo.setPNext(&variableAllocInfo);
		}

		auto result =
			getCore().getContext().getDevice().allocateDescriptorSets(&allocInfo, &vulkanHandle);
		if (result != vk::Result::eSuccess) {
			// create a new descriptor pool if the previous one ran out of memory
			if (result == vk::Result::eErrorOutOfPoolMemory) {
				allocateDescriptorPool();
				allocInfo.setDescriptorPool(m_Pools.back());
				result = getCore().getContext().getDevice().allocateDescriptorSets(&allocInfo,
																				   &vulkanHandle);
			}

			if (result != vk::Result::eSuccess) {
				vkcv_log(LogLevel::ERROR, "Failed to create descriptor set (%s)",
						 vk::to_string(result).c_str());
				return {};
			}
		};

		size_t poolIndex = (m_Pools.size() - 1);
		return add({ vulkanHandle, layout, poolIndex });
	}

	/**
	 * @brief Structure to store details to write to a descriptor set.
	 */
	struct WriteDescriptorSetInfo {
		size_t imageInfoIndex;
		size_t bufferInfoIndex;
		size_t structureIndex;
		uint32_t binding;
		uint32_t arrayElementIndex;
		uint32_t descriptorCount;
		vk::DescriptorType type;
	};

	void DescriptorSetManager::writeDescriptorSet(const DescriptorSetHandle &handle,
												  const DescriptorWrites &writes,
												  const ImageManager &imageManager,
												  const BufferManager &bufferManager,
												  const SamplerManager &samplerManager) {
		auto &set = (*this) [handle];

		Vector<vk::DescriptorImageInfo> imageInfos;
		Vector<vk::DescriptorBufferInfo> bufferInfos;
		
		bufferInfos.reserve(
				writes.getUniformBufferWrites().size() +
				writes.getStorageBufferWrites().size()
		);
		
		Vector<vk::AccelerationStructureKHR> accelerationStructures;
		Vector<size_t> accelerationStructureOffsets;
		
		accelerationStructureOffsets.reserve(writes.getAccelerationWrites().size());

		Vector<vk::WriteDescriptorSetAccelerationStructureKHR> writeStructures;

		Vector<WriteDescriptorSetInfo> writeInfos;
		writeInfos.reserve(
				writes.getSampledImageWrites().size() +
				writes.getStorageImageWrites().size() +
				writes.getUniformBufferWrites().size() +
				writes.getStorageBufferWrites().size() +
				writes.getSamplerWrites().size() +
				writes.getAccelerationWrites().size()
		);

		for (const auto &write : writes.getSampledImageWrites()) {
			const vk::ImageLayout layout =
				(write.useGeneralLayout ? vk::ImageLayout::eGeneral :
										  vk::ImageLayout::eShaderReadOnlyOptimal);

			for (uint32_t i = 0; i < write.mipCount; i++) {
				const vk::DescriptorImageInfo imageInfo(
					nullptr,
					imageManager.getVulkanImageView(write.image, write.mipLevel + i,
													write.arrayView),
					layout);

				imageInfos.push_back(imageInfo);
			}

			WriteDescriptorSetInfo vulkanWrite = {
				imageInfos.size() + 1 - write.mipCount,
				0,
				0,
				write.binding,
				write.arrayIndex,
				write.mipCount,
				vk::DescriptorType::eSampledImage,
			};

			writeInfos.push_back(vulkanWrite);
		}

		for (const auto &write : writes.getStorageImageWrites()) {
			for (uint32_t i = 0; i < write.mipCount; i++) {
				const vk::DescriptorImageInfo imageInfo(
					nullptr,
					imageManager.getVulkanImageView(write.image, write.mipLevel + i,
													write.arrayView),
					vk::ImageLayout::eGeneral);

				imageInfos.push_back(imageInfo);
			}

			WriteDescriptorSetInfo vulkanWrite = {
				imageInfos.size() + 1 - write.mipCount, 0, 0, write.binding, 0, write.mipCount,
				vk::DescriptorType::eStorageImage
			};

			writeInfos.push_back(vulkanWrite);
		}

		for (const auto &write : writes.getUniformBufferWrites()) {
			const size_t size = bufferManager.getBufferSize(write.buffer);
			const uint32_t offset = std::clamp<uint32_t>(write.offset, 0, size);

			const vk::DescriptorBufferInfo bufferInfo(
				bufferManager.getBuffer(write.buffer), offset,
				write.size == 0 ? size : std::min<uint32_t>(write.size, size - offset));

			bufferInfos.push_back(bufferInfo);

			WriteDescriptorSetInfo vulkanWrite = {
					0,
					bufferInfos.size(),
					0,
					write.binding,
					0,
					1,
					write.dynamic ?
						vk::DescriptorType::eUniformBufferDynamic :
						vk::DescriptorType::eUniformBuffer
			};

			writeInfos.push_back(vulkanWrite);
		}

		for (const auto &write : writes.getStorageBufferWrites()) {
			const size_t size = bufferManager.getBufferSize(write.buffer);
			const uint32_t offset = std::clamp<uint32_t>(write.offset, 0, size);

			const vk::DescriptorBufferInfo bufferInfo(
				bufferManager.getBuffer(write.buffer), offset,
				write.size == 0 ? size : std::min<uint32_t>(write.size, size - offset));

			bufferInfos.push_back(bufferInfo);

			WriteDescriptorSetInfo vulkanWrite = {
					0,
					bufferInfos.size(),
					0,
					write.binding,
					0,
					1,
					write.dynamic ?
						vk::DescriptorType::eStorageBufferDynamic :
						vk::DescriptorType::eStorageBuffer
			};

			writeInfos.push_back(vulkanWrite);
		}

		for (const auto &write : writes.getSamplerWrites()) {
			const vk::Sampler &sampler = samplerManager.getVulkanSampler(write.sampler);

			const vk::DescriptorImageInfo imageInfo(sampler, nullptr, vk::ImageLayout::eGeneral);

			imageInfos.push_back(imageInfo);

			WriteDescriptorSetInfo vulkanWrite = {
				imageInfos.size(), 0, 0, write.binding, 0, 1, vk::DescriptorType::eSampler
			};

			writeInfos.push_back(vulkanWrite);
		}
		
		for (const auto &write : writes.getAccelerationWrites()) {
			accelerationStructureOffsets.push_back(accelerationStructures.size());
			
			for (const auto &handle : write.structures) {
				accelerationStructures.push_back(getCore().getVulkanAccelerationStructure(handle));
			}
		}

		for (size_t i = 0; i < writes.getAccelerationWrites().size(); i++) {
			const auto &write = writes.getAccelerationWrites()[i];
			
			const vk::WriteDescriptorSetAccelerationStructureKHR structureWrite(
					write.structures.size(),
					&(accelerationStructures[ accelerationStructureOffsets[i] ])
			);

			writeStructures.push_back(structureWrite);

			WriteDescriptorSetInfo vulkanWrite = {
					0,
					0,
					writeStructures.size(),
					write.binding,
					0,
					1,
					vk::DescriptorType::eAccelerationStructureKHR
			};

			writeInfos.push_back(vulkanWrite);
		}

		Vector<vk::WriteDescriptorSet> vulkanWrites;
		vulkanWrites.reserve(writeInfos.size());

		for (const auto &write : writeInfos) {
			vk::WriteDescriptorSet vulkanWrite(
				set.vulkanHandle, write.binding, write.arrayElementIndex, write.descriptorCount,
				write.type,
				(write.imageInfoIndex > 0 ? &(imageInfos [write.imageInfoIndex - 1]) : nullptr),
				(write.bufferInfoIndex > 0 ? &(bufferInfos [write.bufferInfoIndex - 1]) : nullptr));

			if (write.structureIndex > 0) {
				vulkanWrite.setPNext(&(writeStructures [write.structureIndex - 1]));
			}

			vulkanWrites.push_back(vulkanWrite);
		}

		getCore().getContext().getDevice().updateDescriptorSets(vulkanWrites, nullptr);
	}

	const DescriptorSetEntry &
	DescriptorSetManager::getDescriptorSet(const DescriptorSetHandle &handle) const {
		return (*this) [handle];
	}

} // namespace vkcv
