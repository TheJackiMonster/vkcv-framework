#include <vulkan/vulkan.hpp>

#include "vkcv/Handles.hpp"
#include "vkcv/ResourcesConfig.hpp"

namespace vkcv
{
	class DescriptorManager
	{
	public:
	    explicit DescriptorManager(vk::Device) noexcept;
	    ~DescriptorManager();


	    // TODO: Interface
	    //  user wishes to create descriptor sets {X,Y,Z} each with different descriptions {A,B,C}
	    //  returns a resource
        ResourcesHandle createResourceDescription(const std::vector<SetDescription> &setDescriptions);

	private:
        vk::DescriptorPool m_Pool;

        // TODO: container for all resources requested by the user
        struct ResourceDescription
        {
            std::vector<vk::DescriptorSet> descriptorSets;
            std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
        };

        std::vector<ResourceDescription> m_Resources;
	};
}