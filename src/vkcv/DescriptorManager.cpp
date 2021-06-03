#include "DescriptorManager.hpp"

namespace vkcv
{
    DescriptorManager::ResourceDescription::ResourceDescription(std::vector<vk::DescriptorSet> sets,
                                                                std::vector<vk::DescriptorSetLayout> layouts) noexcept :
    descriptorSets{std::move(sets)},
    descriptorSetLayouts{std::move(layouts)}
    {}
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

		vk::DescriptorPool initPool = allocateDescriptorPool();
    }

    DescriptorManager::~DescriptorManager() noexcept
    {
        for (uint64_t id = 0; id < m_ResourceDescriptions.size(); id++) {
			destroyResourceDescriptionById(id);
        }
		for (uint64_t i = 0; i < m_Pools.size(); i++) {
			m_Device.destroy(m_Pools[i]);
		}
    }

    ResourcesHandle DescriptorManager::createResourceDescription(const std::vector<DescriptorSetConfig> &descriptorSets)
    {
        std::vector<vk::DescriptorSet> vk_sets;
        std::vector<vk::DescriptorSetLayout> vk_setLayouts;

        for (const auto &set : descriptorSets) {
            std::vector<vk::DescriptorSetLayoutBinding> setBindings = {};

            //create each set's binding
            for (uint32_t j = 0; j < set.bindings.size(); j++) {
                vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding(
                        j,
                        convertDescriptorTypeFlag(set.bindings[j].descriptorType),
                        set.bindings[j].descriptorCount,
                        convertShaderStageFlag(set.bindings[j].shaderStage));
                setBindings.push_back(descriptorSetLayoutBinding);
            }

            //create the descriptor set's layout from the bindings gathered above
            vk::DescriptorSetLayoutCreateInfo layoutInfo({}, setBindings);
            vk::DescriptorSetLayout layout = nullptr;
            if(m_Device.createDescriptorSetLayout(&layoutInfo, nullptr, &layout) != vk::Result::eSuccess)
            {
                std::cout << "FAILED TO CREATE DESCRIPTOR SET LAYOUT" << std::endl;
                return ResourcesHandle();
            };
            vk_setLayouts.push_back(layout);
        }
        //create and allocate the set(s) based on the layouts that have been gathered above
        vk_sets.resize(vk_setLayouts.size());
        vk::DescriptorSetAllocateInfo allocInfo(m_Pools.back(), vk_sets.size(), vk_setLayouts.data());
        auto result = m_Device.allocateDescriptorSets(&allocInfo, vk_sets.data());
        if(result != vk::Result::eSuccess)
        {
			//create a new descriptor pool if the previous one ran out of memory
			if (result == vk::Result::eErrorOutOfPoolMemory) {
				allocateDescriptorPool();
				allocInfo.setDescriptorPool(m_Pools.back());
				result = m_Device.allocateDescriptorSets(&allocInfo, vk_sets.data());
			}
			if (result != vk::Result::eSuccess) {
				std::cout << "FAILED TO ALLOCATE DESCRIPTOR SET" << std::endl;
				std::cout << vk::to_string(result) << std::endl;
				for (const auto& layout : vk_setLayouts)
					m_Device.destroy(layout);

				return ResourcesHandle();
			}
        };

        const uint64_t id = m_ResourceDescriptions.size();
        m_ResourceDescriptions.emplace_back(vk_sets, vk_setLayouts);
        return ResourcesHandle(id, [&](uint64_t id) { destroyResourceDescriptionById(id); });
    }
    
    struct WriteDescriptorSetInfo {
		size_t imageInfoIndex;
		size_t bufferInfoIndex;
		uint32_t binding;
		vk::DescriptorType type;
    };

	void DescriptorManager::writeResourceDescription(
		ResourcesHandle			handle, 
		size_t					setIndex,
		const DescriptorWrites	&writes,
		const ImageManager		&imageManager, 
		const BufferManager		&bufferManager,
		const SamplerManager	&samplerManager) {

		vk::DescriptorSet set = m_ResourceDescriptions[handle.getId()].descriptorSets[setIndex];

		std::vector<vk::DescriptorImageInfo> imageInfos;
		std::vector<vk::DescriptorBufferInfo> bufferInfos;
		
		std::vector<WriteDescriptorSetInfo> writeInfos;

		for (const auto& write : writes.sampledImageWrites) {
			const vk::DescriptorImageInfo imageInfo(
				nullptr,
				imageManager.getVulkanImageView(write.image),
				vk::ImageLayout::eShaderReadOnlyOptimal
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
				imageManager.getVulkanImageView(write.image),
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

	vk::DescriptorSet DescriptorManager::getDescriptorSet(ResourcesHandle handle, size_t index) {
		return m_ResourceDescriptions[handle.getId()].descriptorSets[index];
	}

	vk::DescriptorSetLayout DescriptorManager::getDescriptorSetLayout(ResourcesHandle handle, size_t index) {
		return m_ResourceDescriptions[handle.getId()].descriptorSetLayouts[index];
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
				std::cerr << "Error: DescriptorManager::convertDescriptorTypeFlag, unknown DescriptorType" << std::endl;
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
    
    void DescriptorManager::destroyResourceDescriptionById(uint64_t id) {
		if (id >= m_ResourceDescriptions.size()) {
			return;
		}
		
		auto& resourceDescription = m_ResourceDescriptions[id];
	
		for(const auto &layout : resourceDescription.descriptorSetLayouts) {
			m_Device.destroyDescriptorSetLayout(layout);
		}
	
		resourceDescription.descriptorSetLayouts.clear();
	}

	vk::DescriptorPool DescriptorManager::allocateDescriptorPool() {
		vk::DescriptorPool pool;
		if (m_Device.createDescriptorPool(&m_PoolInfo, nullptr, &pool) != vk::Result::eSuccess)
		{
			std::cout << "FAILED TO ALLOCATE DESCRIPTOR POOL." << std::endl;
			pool = nullptr;
		};
		m_Pools.push_back(pool);
		return pool;
	}

}