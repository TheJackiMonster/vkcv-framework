#include "DescriptorManager.hpp"

namespace vkcv
{
    DescriptorManager::DescriptorManager(vk::Device device) noexcept:
        m_Device{ device }
    {
        /**
         * Allocate the set size for the descriptor pools, namely 1000 units of each descriptor type below.
		 * Finally, create an initial pool.
         */
		m_PoolSizes = {
				vk::DescriptorPoolSize(vk::DescriptorType::eSampler, 1000),
				vk::DescriptorPoolSize(vk::DescriptorType::eSampledImage, 1000),
				vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1000),
				vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 1000),
				vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1000),
				vk::DescriptorPoolSize(vk::DescriptorType::eStorageBufferDynamic, 1000),    // for RTX
				vk::DescriptorPoolSize(vk::DescriptorType::eAccelerationStructureKHR, 1000) // for RTX
		};

		m_PoolInfo = vk::DescriptorPoolCreateInfo(
				vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			1000,
			static_cast<uint32_t>(m_PoolSizes.size()),
			m_PoolSizes.data());

		allocateDescriptorPool();
    }

    DescriptorManager::~DescriptorManager() noexcept
    {
        for (uint64_t id = 0; id < m_DescriptorSets.size(); id++) {
			destroyDescriptorSetById(id);
        }
		
		for (uint64_t id = 0; id < m_DescriptorSetLayouts.size(); id++) {
			// Resets the usage count to zero for destruction.
			m_DescriptorSetLayouts[id].layoutUsageCount = 0;
			destroyDescriptorSetLayoutById(id);
		}
        
		m_DescriptorSets.clear();
		m_DescriptorSetLayouts.clear();
  
		for (const auto &pool : m_Pools) {
			if (pool) {
				m_Device.destroy(pool);
			}
		}
    }

    DescriptorSetLayoutHandle DescriptorManager::createDescriptorSetLayout(const DescriptorBindings &bindings)
    {
        for (size_t i = 0; i < m_DescriptorSetLayouts.size(); i++)
        {
            if(m_DescriptorSetLayouts[i].descriptorBindings.size() != bindings.size())
                continue;

            if (m_DescriptorSetLayouts[i].descriptorBindings == bindings)
            {
				m_DescriptorSetLayouts[i].layoutUsageCount++;
                return DescriptorSetLayoutHandle(i, [&](uint64_t id) { destroyDescriptorSetLayoutById(id); });
            }
        }
        
        //create the descriptor set's layout and binding flags by iterating over its bindings
        std::vector<vk::DescriptorSetLayoutBinding> bindingsVector = {};
		std::vector<vk::DescriptorBindingFlags> bindingsFlags = {};
		
        for (auto bindingElem : bindings)
        {
            DescriptorBinding binding = bindingElem.second;
            uint32_t bindingID = bindingElem.first;
	
			bindingsVector.emplace_back(
					bindingID,
					getVkDescriptorType(binding.descriptorType),
					binding.descriptorCount,
					getShaderStageFlags(binding.shaderStages),
					nullptr
			);
			
			vk::DescriptorBindingFlags flags;
			
			if (binding.variableCount)
				flags |= vk::DescriptorBindingFlagBits::eVariableDescriptorCount;
			
			if (binding.partialBinding)
				flags |= vk::DescriptorBindingFlagBits::ePartiallyBound;
	
			bindingsFlags.push_back(flags);
        }
		
        vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo (
				bindingsFlags.size(), bindingsFlags.data()
		);

        //create the descriptor set's layout from the binding data gathered above
        vk::DescriptorSetLayout vulkanHandle;
        vk::DescriptorSetLayoutCreateInfo layoutInfo(vk::DescriptorSetLayoutCreateFlags(), bindingsVector);
		layoutInfo.setPNext(&bindingFlagsInfo);
		
        auto result = m_Device.createDescriptorSetLayout(&layoutInfo, nullptr, &vulkanHandle);
        if (result != vk::Result::eSuccess) {
            vkcv_log(LogLevel::ERROR, "Failed to create descriptor set layout");
            return DescriptorSetLayoutHandle();
        };

        const uint64_t id = m_DescriptorSetLayouts.size();
        m_DescriptorSetLayouts.push_back({vulkanHandle, bindings, 1});
        return DescriptorSetLayoutHandle(id, [&](uint64_t id) { destroyDescriptorSetLayoutById(id); });
    }

    DescriptorSetHandle DescriptorManager::createDescriptorSet(const DescriptorSetLayoutHandle &layout)
    {
        //create and allocate the set based on the layout provided
        DescriptorSetLayout setLayout = m_DescriptorSetLayouts[layout.getId()];
        vk::DescriptorSet vulkanHandle;
        vk::DescriptorSetAllocateInfo allocInfo(m_Pools.back(), 1, &setLayout.vulkanHandle);

        uint32_t sumVariableDescriptorCounts = 0;
        for (auto bindingElem : setLayout.descriptorBindings)
        {
            auto binding = bindingElem.second;
            if(binding.variableCount)
                sumVariableDescriptorCounts += binding.descriptorCount;
        }

        vk::DescriptorSetVariableDescriptorCountAllocateInfo variableAllocInfo(1, &sumVariableDescriptorCounts);

        if (sumVariableDescriptorCounts > 0) {
            allocInfo.setPNext(&variableAllocInfo);
        }

        auto result = m_Device.allocateDescriptorSets(&allocInfo, &vulkanHandle);
        if(result != vk::Result::eSuccess)
        {
			//create a new descriptor pool if the previous one ran out of memory
			if (result == vk::Result::eErrorOutOfPoolMemory) {
				allocateDescriptorPool();
				allocInfo.setDescriptorPool(m_Pools.back());
				result = m_Device.allocateDescriptorSets(&allocInfo, &vulkanHandle);
			}
			
			if (result != vk::Result::eSuccess) {
				vkcv_log(LogLevel::ERROR, "Failed to create descriptor set (%s)",
						 vk::to_string(result).c_str());
				return DescriptorSetHandle();
			}
        };
	
		size_t poolIndex = (m_Pools.size() - 1);
        const uint64_t id = m_DescriptorSets.size();
        m_DescriptorSets.push_back({ vulkanHandle, layout, poolIndex });
        return DescriptorSetHandle(id, [&](uint64_t id) { destroyDescriptorSetById(id); });
    }
    
	/**
	 * @brief Structure to store details to write to a descriptor set.
	 */
    struct WriteDescriptorSetInfo {
		size_t imageInfoIndex;
		size_t bufferInfoIndex;
		uint32_t binding;
		uint32_t arrayElementIndex;
		uint32_t descriptorCount;
		vk::DescriptorType type;
    };

	void DescriptorManager::writeDescriptorSet(
		const DescriptorSetHandle	&handle,
		const DescriptorWrites	&writes,
		const ImageManager		&imageManager, 
		const BufferManager		&bufferManager,
		const SamplerManager	&samplerManager) {
		vk::DescriptorSet set = m_DescriptorSets[handle.getId()].vulkanHandle;

		std::vector<vk::DescriptorImageInfo> imageInfos;
		std::vector<vk::DescriptorBufferInfo> bufferInfos;
		
		std::vector<WriteDescriptorSetInfo> writeInfos;

		for (const auto& write : writes.getSampledImageWrites())
		{
		    const vk::ImageLayout layout = (write.useGeneralLayout?
					vk::ImageLayout::eGeneral :
					vk::ImageLayout::eShaderReadOnlyOptimal
			);
			
			for (uint32_t i = 0; i < write.mipCount; i++) {
				const vk::DescriptorImageInfo imageInfo(
						nullptr,
						imageManager.getVulkanImageView(
								write.image,
								write.mipLevel + i,
								write.arrayView
						),
						layout
				);
				
				imageInfos.push_back(imageInfo);
			}
			
			WriteDescriptorSetInfo vulkanWrite = {
					imageInfos.size() + 1 - write.mipCount,
					0,
					write.binding,
					write.arrayIndex,
					write.mipCount,
					vk::DescriptorType::eSampledImage,
			};
			
			writeInfos.push_back(vulkanWrite);
		}

		for (const auto& write : writes.getStorageImageWrites()) {
			for (uint32_t i = 0; i < write.mipCount; i++) {
				const vk::DescriptorImageInfo imageInfo(
						nullptr,
						imageManager.getVulkanImageView(
								write.image,
								write.mipLevel + i,
								write.arrayView
						),
						vk::ImageLayout::eGeneral
				);
				
				imageInfos.push_back(imageInfo);
			}
			
			WriteDescriptorSetInfo vulkanWrite = {
					imageInfos.size() + 1 - write.mipCount,
					0,
					write.binding,
					0,
					write.mipCount,
					vk::DescriptorType::eStorageImage
			};
			
			writeInfos.push_back(vulkanWrite);
		}

		for (const auto& write : writes.getUniformBufferWrites()) {
			const size_t size = bufferManager.getBufferSize(write.buffer);
			const uint32_t offset = std::clamp<uint32_t>(write.offset, 0, size);
			
			const vk::DescriptorBufferInfo bufferInfo(
				bufferManager.getBuffer(write.buffer),
				offset,
				write.size == 0? size : std::min<uint32_t>(
						write.size, size - offset
				)
			);
			
			bufferInfos.push_back(bufferInfo);

			WriteDescriptorSetInfo vulkanWrite = {
					0,
					bufferInfos.size(),
					write.binding,
					0,
					1,
					write.dynamic?
					vk::DescriptorType::eUniformBufferDynamic :
					vk::DescriptorType::eUniformBuffer
			};
			
			writeInfos.push_back(vulkanWrite);
		}

		for (const auto& write : writes.getStorageBufferWrites()) {
			const size_t size = bufferManager.getBufferSize(write.buffer);
			const uint32_t offset = std::clamp<uint32_t>(write.offset, 0, size);
			
			const vk::DescriptorBufferInfo bufferInfo(
				bufferManager.getBuffer(write.buffer),
				offset,
				write.size == 0? size : std::min<uint32_t>(
						write.size, size - offset
				)
			);
			
			bufferInfos.push_back(bufferInfo);
			
			WriteDescriptorSetInfo vulkanWrite = {
					0,
					bufferInfos.size(),
					write.binding,
					0,
					1,
					write.dynamic?
					vk::DescriptorType::eStorageBufferDynamic :
					vk::DescriptorType::eStorageBuffer
			};
			
			writeInfos.push_back(vulkanWrite);
		}

		for (const auto& write : writes.getSamplerWrites()) {
			const vk::Sampler& sampler = samplerManager.getVulkanSampler(write.sampler);
			
			const vk::DescriptorImageInfo imageInfo(
				sampler,
				nullptr,
				vk::ImageLayout::eGeneral
			);
			
			imageInfos.push_back(imageInfo);

			WriteDescriptorSetInfo vulkanWrite = {
					imageInfos.size(),
					0,
					write.binding,
					0,
					1,
					vk::DescriptorType::eSampler
			};
			
			writeInfos.push_back(vulkanWrite);
		}
		
		std::vector<vk::WriteDescriptorSet> vulkanWrites;
		
		for (const auto& write : writeInfos) {
			vk::WriteDescriptorSet vulkanWrite(
					set,
					write.binding,
					write.arrayElementIndex,
					write.descriptorCount,
					write.type,
					(write.imageInfoIndex > 0? &(imageInfos[write.imageInfoIndex - 1]) : nullptr),
					(write.bufferInfoIndex > 0? &(bufferInfos[write.bufferInfoIndex - 1]) : nullptr)
			);
			
			vulkanWrites.push_back(vulkanWrite);
		}
		
		m_Device.updateDescriptorSets(vulkanWrites, nullptr);
	}

	DescriptorSetLayout DescriptorManager::getDescriptorSetLayout(const DescriptorSetLayoutHandle handle) const
	{
	    return m_DescriptorSetLayouts[handle.getId()];
	}

	DescriptorSet DescriptorManager::getDescriptorSet(const DescriptorSetHandle handle) const {
		return m_DescriptorSets[handle.getId()];
	}

    void DescriptorManager::destroyDescriptorSetById(uint64_t id) {
		if (id >= m_DescriptorSets.size()) {
			vkcv_log(LogLevel::ERROR, "Invalid id");
			return;
		}
		
		auto& set = m_DescriptorSets[id];
		
		if (set.vulkanHandle) {
			m_Device.freeDescriptorSets(m_Pools[set.poolIndex], 1, &(set.vulkanHandle));
			set.setLayoutHandle = DescriptorSetLayoutHandle();
			set.vulkanHandle = nullptr;
		}
	}

	void DescriptorManager::destroyDescriptorSetLayoutById(uint64_t id) {
	    if (id >= m_DescriptorSetLayouts.size()) {
	        vkcv_log(LogLevel::ERROR, "Invalid id");
	        return;
	    }

	    auto& layout = m_DescriptorSetLayouts[id];

		if (layout.layoutUsageCount > 1) {
			layout.layoutUsageCount--;
			return;
		} else {
			layout.layoutUsageCount = 0;
		}
		
	    if (layout.vulkanHandle){
	        m_Device.destroy(layout.vulkanHandle);
	        layout.vulkanHandle = nullptr;
	    }
	}

	vk::DescriptorPool DescriptorManager::allocateDescriptorPool() {
		vk::DescriptorPool pool;
		if (m_Device.createDescriptorPool(&m_PoolInfo, nullptr, &pool) != vk::Result::eSuccess) {
			vkcv_log(LogLevel::WARNING, "Failed to allocate descriptor pool");
			pool = nullptr;
		} else {
			m_Pools.push_back(pool);
		}
		
		return pool;
	}

}
