#pragma once

#include <vulkan/vulkan.hpp>

namespace vkcv {

	class DescriptorSet
	{

	public:
		DescriptorSet::DescriptorSet();
		void createDescriptorSets(vk::Device device);
	private:
		//std::vector<vk::DescriptorPoolSize> descriptorPoolSizes;
		//std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBinding;
		//vk::DescriptorSetLayout descriptorSetLayout;
		//vk::DescriptorPool descriptorPool;
	};
}