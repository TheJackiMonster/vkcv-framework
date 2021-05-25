#include <vulkan/vulkan.hpp>

#include "vkcv/Handles.hpp"
#include "vkcv/DescriptorConfig.hpp"

namespace vkcv
{
	class DescriptorManager
	{
	public:
	    explicit DescriptorManager(vk::Device device) noexcept;
	    ~DescriptorManager() = default;

		/**
		* Creates all vk::DescriptorSets and allocates them from the pool. 
		* DescriptorSets are put inside a ResourceDescription struct. 
		* Structs are then put into m_ResourceDescriptions.
		* @param[in] vector of filled vkcv::DescriptorSet structs
		* @return index into that objects a resource handle
		*/
        ResourcesHandle createResourceDescription(const std::vector<DescriptorSet> & p_descriptorSets);

	private:
		vk::Device m_Device;
        vk::DescriptorPool m_Pool;
		uint64_t m_NextDescriptorSetID;

		/**
		* Container for all resources requested by the user in one call of createResourceDescription.
		* Includes descriptor sets and the respective descriptor set layouts.
		*/
        struct ResourceDescription
        {
            std::vector<vk::DescriptorSet> descriptorSets;
            std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
        };

		/**
		* Contains all the resource descriptions that were requested by the user in calls of createResourceDescription.
		*/
        std::vector<ResourceDescription> m_ResourceDescriptions;

		/**
		* Contains all the allowed vkcv::DescriptorTypes that were also pre-defined in the DescriptorConfig class.
		* Allowed types are defined in the constructor of DescriptorManager.
		*/
		std::vector<DescriptorType> m_DescriptorTypes;
		
		/**
		* Converts the flags of the descriptor types from VulkanCV (vkcv) to Vulkan (vk).
		* @param[in] vkcv flag of the DescriptorType (see DescriptorConfig.hpp)
		* @return vk flag of the DescriptorType
		*/
		vk::DescriptorType convertDescriptorTypeFlag(DescriptorType type);
		/**
		* Converts the flags of the shader stages from VulkanCV (vkcv) to Vulkan (vk).
		* @param[in] vkcv flag of the ShaderStage (see ShaderProgram.hpp)
		* @return vk flag of the ShaderStage
		*/
		vk::ShaderStageFlagBits vkcv::DescriptorManager::convertShaderStageFlag(ShaderStage stage);
	};
}