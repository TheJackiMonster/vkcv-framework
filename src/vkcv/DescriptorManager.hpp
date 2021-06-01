#include <vulkan/vulkan.hpp>

#include "vkcv/Handles.hpp"
#include "vkcv/DescriptorConfig.hpp"
#include "vkcv/DescriptorWrites.hpp"

#include "ImageManager.hpp"
#include "vkcv/BufferManager.hpp"
#include "SamplerManager.hpp"

namespace vkcv
{
	class DescriptorManager
	{
	public:
	    explicit DescriptorManager(vk::Device device) noexcept;
	    ~DescriptorManager() noexcept;

		/**
		* Creates all vk::DescriptorSets and allocates them from the pool. 
		* DescriptorSets are put inside a ResourceDescription struct. 
		* Structs are then put into m_ResourceDescriptions.
		* @param[in] vector of filled vkcv::DescriptorSet structs
		* @return index into that objects a resource handle
		*/
        ResourcesHandle createResourceDescription(const std::vector<DescriptorSetConfig> & descriptorSets);

		void writeResourceDescription(
			ResourcesHandle			handle,
			size_t					setIndex,
			const DescriptorWrites& writes,
			const ImageManager& imageManager,
			const BufferManager& bufferManager,
			const SamplerManager& samplerManager);

		vk::DescriptorSet		getDescriptorSet(ResourcesHandle handle, size_t index);
		vk::DescriptorSetLayout getDescriptorSetLayout(ResourcesHandle handle, size_t index);

	private:
		vk::Device			m_Device;
        vk::DescriptorPool	m_Pool;


		/**
		* Container for all resources requested by the user in one call of createResourceDescription.
		* Includes descriptor sets and the respective descriptor set layouts.
		*/
        struct ResourceDescription
        {
            ResourceDescription() = delete;
            ResourceDescription(std::vector<vk::DescriptorSet> sets, std::vector<vk::DescriptorSetLayout> layouts) noexcept;

            std::vector<vk::DescriptorSet> descriptorSets;
            std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
        };

		/**
		* Contains all the resource descriptions that were requested by the user in calls of createResourceDescription.
		*/
        std::vector<ResourceDescription> m_ResourceDescriptions;
		
		/**
		* Converts the flags of the descriptor types from VulkanCV (vkcv) to Vulkan (vk).
		* @param[in] vkcv flag of the DescriptorType (see DescriptorConfig.hpp)
		* @return vk flag of the DescriptorType
		*/
		static vk::DescriptorType convertDescriptorTypeFlag(DescriptorType type);
		/**
		* Converts the flags of the shader stages from VulkanCV (vkcv) to Vulkan (vk).
		* @param[in] vkcv flag of the ShaderStage (see ShaderProgram.hpp)
		* @return vk flag of the ShaderStage
		*/
		static vk::ShaderStageFlagBits convertShaderStageFlag(ShaderStage stage);
		
		void destroyResourceDescriptionById(uint64_t id);
		
	};
}