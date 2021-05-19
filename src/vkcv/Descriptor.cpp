#include "vkcv/Descriptor.hpp"

namespace vkcv
{
	const VkDescriptorSetLayout createDescSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSetLayoutCreateInfo* info)
	{

		/* Code, falls nur ein Set mit nur einem Binding erstellt wird
		VkDescriptorSetLayoutBinding layoutBinding = {};
		layoutBinding.binding = 0;
		layoutBinding.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &layoutBinding;*/

		//Erstellen von der Layout Info
		DescriptorLayoutInfo layoutinfo;
		layoutinfo.bindings.reserve(info->bindingCount);
		bool isSorted = true;
		int lastBinding = -1;

		//Sortieren und pushen von Bindings für Ordnung
		for (int i = 0; i < info->bindingCount; i++) {
			layoutinfo.bindings.push_back(info->pBindings[i]);

			//Bindings müssen aufsteigend sortiert sein
			if (info->pBindings[i].binding > lastBinding) {
				lastBinding = info->pBindings[i].binding;
			}
			else {
				isSorted = false;
			}
		}
		//Sortieren, falls Bindings noch nicht sortiert sind
		if (!isSorted) {
			std::sort(layoutinfo.bindings.begin(), layoutinfo.bindings.end(), [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b) {
				return a.binding < b.binding;
			});
		}

		//Test, ob es funktioniert
		VkResult result = vkCreateDescriptorSetLayout(
			device,
			&layoutInfo,
			nullptr,
			&descriptorSetLayout
		);
		assert(result == VK_SUCCESS);

	}

	const VkDescriptorPool createDescPool(VkDescriptorPool descriptorPool, VkDevice device, const PoolSizes& poolSizes, int count)
	{
		//Größe des Pools bestimmen 
		std::vector<VkDescriptorPoolSize> poolSize;
		poolSize.reserve(poolSizes.sizes.size());
		for (auto sz : poolSizes.sizes) {
			poolSize.push_back({ sz.first, uint32_t(sz.second * count) });
		}

		//Erstelle das createInfo zu den Pools
		VkDescriptorPoolCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createInfo.poolSizeCount = (uint32_t)poolSizes.sizes.size();
		createInfo.pPoolSizes = poolSize.data();
		createInfo.maxSets = count;

		//Teste, ob der Pool erstellt werden kann
		VkResult createDescriptorPool = vkCreateDescriptorPool(
			device,
			&createInfo,
			nullptr,
			&descriptorPool
		);
		assert(createDescriptorPool == VK_SUCCESS);

	}

	const void destroyDescriptor(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool)  
	{
		//Destruktor für Layout und Pool
		vkDestroyDescriptorSetLayout(
			device,
			descriptorSetLayout,
			nullptr
		);

		vkDestroyDescriptorPool(
			device,
			descriptorPool,
			nullptr
		);
	}

	void Descriptor::createDescriptors(VkDevice device/*, int buffer_size*/)
	{
		//Benötigte Variablen
		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorSet descriptorSet;
		VkWriteDescriptorSet descriptorWriteSet;
		PoolSizes descriptorSizes;
		int count = 1000;
		
		//Erstellung des Pools
		descriptorPool = createDescPool(descriptorPool, device, descriptorSizes, count);

		//Erstellung der Set Layouts
		descriptorSetLayout = createDescSetLayout(device, descriptorSetLayout);

		{	
			//Sets erstellen
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = count;
			allocInfo.pSetLayouts = &descriptorSetLayout;

			//Teste, ob Sets erstellt wurden
			VkResult result = vkAllocateDescriptorSets(
				device,
				&allocInfo,
				&descriptorSet
			);
			assert (result == VK_SUCCESS);

		}

		
		{	/*Buffer Date in Descriptor schreiben
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = buffer_size;

			descriptorWriteSet = {};
			descriptorWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWriteSet.dstSet = descriptorSet;
			descriptorWriteSet.dstBinding = 0;
			descriptorWriteSet.dstArrayElement = 0;
			descriptorWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWriteSet.descriptorCount = 1;
			descriptorWriteSet.pBufferInfo = &bufferInfo;


			vkUpdateDescriptorSets(
				device,
				1,
				&descriptorWriteSet,
				0,
				NULL
			);*/
		}

	}


}