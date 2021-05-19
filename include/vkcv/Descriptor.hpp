#include <vulkan/vulkan.hpp>

namespace vkcv {

	class Descriptor
	{

	
		friend class Core;
	public:

		//Die verschiedenen Pool Sizes für die verschiedenen Arten von Buffern
		struct PoolSizes {
			std::vector<std::pair<VkDescriptorType, float>> sizes =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f }
			};
		};

		void createDescriptors(VkDevice device/*, int buffer_size*/);
		
		const void destroyDescriptor(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);

		const VkDescriptorPool createDescPool(VkDescriptorPool descriptorPool, VkDevice device, const PoolSizes& poolSizes, int count);

		const VkDescriptorSetLayout createDescSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSetLayoutCreateInfo* info);

		struct DescriptorLayoutInfo;

		struct PoolSizes;

	private:

		//Layout Info
		struct DescriptorLayoutInfo {
			std::vector<VkDescriptorSetLayoutBinding> bindings;

			bool operator==(const DescriptorLayoutInfo& other) const;

			size_t hash() const;
		};


		//Informationen zum Set Layout
		typedef struct VkDescriptorSetLayoutCreateInfo {
			VkStructureType sType;
			const void* pNext;
			VkDescriptorSetLayoutCreateFlags flags;
			uint32_t bindingCount;
			const VkDescriptorSetLayoutBinding* pBindings;
		} VkDescriptorSetLayoutCreateInfo;
	};
}