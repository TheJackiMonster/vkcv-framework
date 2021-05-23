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


	    // TODO: Interface
	    //  user wishes to create descriptor sets {X,Y,Z} each with different descriptions {A,B,C}
	    //  returns a resource
        ResourcesHandle createResourceDescription(const std::vector<DescriptorSet> & p_descriptorSets);

	private:
		vk::Device m_Device;
        vk::DescriptorPool m_Pool;
		uint64_t m_NextDescriptorSetID;

        // TODO: container for all resources requested by the user
        struct ResourceDescription
        {
            std::vector<vk::DescriptorSet> descriptorSets;
            std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
        };

        std::vector<ResourceDescription> m_Resources;
		std::vector<DescriptorType> m_DescriptorTypes;
		
		/**
		* converts the flags of the descriptor types from VulkanCV (vkcv) to Vulkan (vk)
		* @param[in] vkcv flag of the DescriptorType (see DescriptorConfig.hpp)
		* @return vk flag of the DescriptorType
		*/
		vk::DescriptorType convertDescriptorTypeFlag(DescriptorType type);
		/**
		* converts the flags of the shader stages from VulkanCV (vkcv) to Vulkan (vk)
		* @param[in] vkcv flag of the ShaderStage (see ShaderProgram.hpp)
		* @return vk flag of the ShaderStage
		*/
		vk::ShaderStageFlagBits vkcv::DescriptorManager::convertShaderStageFlag(ShaderStage stage);
	};
}