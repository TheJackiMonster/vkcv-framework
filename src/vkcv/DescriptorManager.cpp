#include "DescriptorManager.hpp"

#include "vkcv/Logger.hpp"

namespace vkcv
{
    DescriptorManager::DescriptorManager(vk::Device device) noexcept:
        m_Device{ device }
    {
        /**
         * Allocate the set size for the descriptor pools, namely 1000 units of each descriptor type below.
		 * Finally, create an initial pool.
         */
		m_PoolSizes = { vk::DescriptorPoolSize(vk::DescriptorType::eSampler, 1000),
													vk::DescriptorPoolSize(vk::DescriptorType::eSampledImage, 1000),
													vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1000),
													vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 1000) };

		m_PoolInfo = vk::DescriptorPoolCreateInfo({},
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
		m_DescriptorSets.clear();
		for (const auto &pool : m_Pools) {
			m_Device.destroy(pool);
		}
    }

    DescriptorSetHandle DescriptorManager::createDescriptorSet(const std::vector<DescriptorBinding>& bindings)
    {
        std::vector<vk::DescriptorSetLayoutBinding> setBindings = {};

        //create each set's binding
        for (uint32_t i = 0; i < bindings.size(); i++) {
            vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding(
                bindings[i].bindingID,
                convertDescriptorTypeFlag(bindings[i].descriptorType),
                bindings[i].descriptorCount,
                convertShaderStageFlag(bindings[i].shaderStage));
            setBindings.push_back(descriptorSetLayoutBinding);
        }

        DescriptorSet set;

        //create the descriptor set's layout from the bindings gathered above
        vk::DescriptorSetLayoutCreateInfo layoutInfo({}, setBindings);
        if(m_Device.createDescriptorSetLayout(&layoutInfo, nullptr, &set.layout) != vk::Result::eSuccess)
        {
			vkcv_log(LogLevel::ERROR, "Failed to create descriptor set layout");
            return DescriptorSetHandle();
        };
        
        //create and allocate the set based on the layout that have been gathered above
        vk::DescriptorSetAllocateInfo allocInfo(m_Pools.back(), 1, &set.layout);
        auto result = m_Device.allocateDescriptorSets(&allocInfo, &set.vulkanHandle);
        if(result != vk::Result::eSuccess)
        {
			//create a new descriptor pool if the previous one ran out of memory
			if (result == vk::Result::eErrorOutOfPoolMemory) {
				allocateDescriptorPool();
				allocInfo.setDescriptorPool(m_Pools.back());
				result = m_Device.allocateDescriptorSets(&allocInfo, &set.vulkanHandle);
			}
			if (result != vk::Result::eSuccess) {
				vkcv_log(LogLevel::ERROR, "Failed to create descriptor set (%s)",
						 vk::to_string(result).c_str());
				
				m_Device.destroy(set.layout);
				return DescriptorSetHandle();
			}
        };

        const uint64_t id = m_DescriptorSets.size();

        m_DescriptorSets.push_back(set);
        return DescriptorSetHandle(id, [&](uint64_t id) { destroyDescriptorSetById(id); });
    }
    
    struct WriteDescriptorSetInfo {
		size_t imageInfoIndex;
		size_t bufferInfoIndex;
		uint32_t binding;
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

		for (const auto& write : writes.sampledImageWrites) {
		    vk::ImageLayout layout = write.useGeneralLayout ? vk::ImageLayout::eGeneral : vk::ImageLayout::eShaderReadOnlyOptimal;
			const vk::DescriptorImageInfo imageInfo(
				nullptr,
				imageManager.getVulkanImageView(write.image, write.mipLevel),
                layout
			);
			
			imageInfos.push_back(imageInfo);
			
			WriteDescriptorSetInfo vulkanWrite = {
					imageInfos.size(),
					0,
					write.binding,
					vk::DescriptorType::eSampledImage,
			};
			
			writeInfos.push_back(vulkanWrite);
		}

		for (const auto& write : writes.storageImageWrites) {
			const vk::DescriptorImageInfo imageInfo(
				nullptr,
				imageManager.getVulkanImageView(write.image, write.mipLevel),
				vk::ImageLayout::eGeneral
			);
			
			imageInfos.push_back(imageInfo);
			
			WriteDescriptorSetInfo vulkanWrite = {
					imageInfos.size(),
					0,
					write.binding,
					vk::DescriptorType::eStorageImage
			};
			
			writeInfos.push_back(vulkanWrite);
		}

		for (const auto& write : writes.uniformBufferWrites) {
			const vk::DescriptorBufferInfo bufferInfo(
				bufferManager.getBuffer(write.buffer),
				static_cast<uint32_t>(0),
				bufferManager.getBufferSize(write.buffer)
			);
			
			bufferInfos.push_back(bufferInfo);

			WriteDescriptorSetInfo vulkanWrite = {
					0,
					bufferInfos.size(),
					write.binding,
					vk::DescriptorType::eUniformBuffer
			};
			
			writeInfos.push_back(vulkanWrite);
		}

		for (const auto& write : writes.storageBufferWrites) {
			const vk::DescriptorBufferInfo bufferInfo(
				bufferManager.getBuffer(write.buffer),
				static_cast<uint32_t>(0),
				bufferManager.getBufferSize(write.buffer)
			);
			
			bufferInfos.push_back(bufferInfo);
			
			WriteDescriptorSetInfo vulkanWrite = {
					0,
					bufferInfos.size(),
					write.binding,
					vk::DescriptorType::eStorageBuffer
			};
			
			writeInfos.push_back(vulkanWrite);
		}

		for (const auto& write : writes.samplerWrites) {
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
					vk::DescriptorType::eSampler
			};
			
			writeInfos.push_back(vulkanWrite);
		}
		
		std::vector<vk::WriteDescriptorSet> vulkanWrites;
		
		for (const auto& write : writeInfos) {
			vk::WriteDescriptorSet vulkanWrite(
					set,
					write.binding,
					static_cast<uint32_t>(0),
					1,
					write.type,
					(write.imageInfoIndex > 0? &(imageInfos[write.imageInfoIndex - 1]) : nullptr),
					(write.bufferInfoIndex > 0? &(bufferInfos[write.bufferInfoIndex - 1]) : nullptr)
			);
			
			vulkanWrites.push_back(vulkanWrite);
		}
		
		m_Device.updateDescriptorSets(vulkanWrites, nullptr);
	}

	DescriptorSet DescriptorManager::getDescriptorSet(const DescriptorSetHandle handle) const {
		return m_DescriptorSets[handle.getId()];
	}

    vk::DescriptorType DescriptorManager::convertDescriptorTypeFlag(DescriptorType type) {
        switch (type)
        {
            case DescriptorType::UNIFORM_BUFFER:
                return vk::DescriptorType::eUniformBuffer;
            case DescriptorType::STORAGE_BUFFER:
                return vk::DescriptorType::eStorageBuffer;
            case DescriptorType::SAMPLER:
                return vk::DescriptorType::eSampler;
            case DescriptorType::IMAGE_SAMPLED:
                return vk::DescriptorType::eSampledImage;
			case DescriptorType::IMAGE_STORAGE:
				return vk::DescriptorType::eStorageImage;
            default:
				vkcv_log(LogLevel::ERROR, "Unknown DescriptorType");
                return vk::DescriptorType::eUniformBuffer;
        }
    }

    vk::ShaderStageFlagBits DescriptorManager::convertShaderStageFlag(ShaderStage stage) {
        switch (stage) 
        {
            case ShaderStage::VERTEX:
                return vk::ShaderStageFlagBits::eVertex;
            case ShaderStage::FRAGMENT:
                return vk::ShaderStageFlagBits::eFragment;
            case ShaderStage::TESS_CONTROL:
                return vk::ShaderStageFlagBits::eTessellationControl;
            case ShaderStage::TESS_EVAL:
                return vk::ShaderStageFlagBits::eTessellationEvaluation;
            case ShaderStage::GEOMETRY:
                return vk::ShaderStageFlagBits::eGeometry;
            case ShaderStage::COMPUTE:
                return vk::ShaderStageFlagBits::eCompute;
            default:
                return vk::ShaderStageFlagBits::eAll;
        }
    }
    
    void DescriptorManager::destroyDescriptorSetById(uint64_t id) {
		if (id >= m_DescriptorSets.size()) {
			vkcv_log(LogLevel::ERROR, "Invalid id");
			return;
		}
		
		auto& set = m_DescriptorSets[id];
		if (set.layout) {
			m_Device.destroyDescriptorSetLayout(set.layout);
			set.layout = nullptr;
		}
		// FIXME: descriptor set itself not destroyed
	}

	vk::DescriptorPool DescriptorManager::allocateDescriptorPool() {
		vk::DescriptorPool pool;
		if (m_Device.createDescriptorPool(&m_PoolInfo, nullptr, &pool) != vk::Result::eSuccess)
		{
			vkcv_log(LogLevel::WARNING, "Failed to allocate descriptor pool");
			pool = nullptr;
		};
		m_Pools.push_back(pool);
		return pool;
	}

}