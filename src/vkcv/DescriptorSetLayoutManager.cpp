#include "DescriptorSetLayoutManager.hpp"

#include "vkcv/Core.hpp"

namespace vkcv {

	uint64_t DescriptorSetLayoutManager::getIdFrom(const DescriptorSetLayoutHandle &handle) const {
		return handle.getId();
	}

	DescriptorSetLayoutHandle
	DescriptorSetLayoutManager::createById(uint64_t id, const HandleDestroyFunction &destroy) {
		return DescriptorSetLayoutHandle(id, destroy);
	}

	void DescriptorSetLayoutManager::destroyById(uint64_t id) {
		auto &layout = getById(id);

		if (layout.layoutUsageCount > 1) {
			layout.layoutUsageCount--;
			return;
		} else {
			layout.layoutUsageCount = 0;
		}

		if (layout.vulkanHandle) {
			getCore().getContext().getDevice().destroy(layout.vulkanHandle);
			layout.vulkanHandle = nullptr;
		}
	}

	DescriptorSetLayoutManager::DescriptorSetLayoutManager() noexcept :
		HandleManager<DescriptorSetLayoutEntry, DescriptorSetLayoutHandle>() {}

	DescriptorSetLayoutManager::~DescriptorSetLayoutManager() noexcept {
		for (uint64_t id = 0; id < getCount(); id++) {
			// Resets the usage count to one for destruction.
			getById(id).layoutUsageCount = 1;
		}

		clear();
	}

	DescriptorSetLayoutHandle
	DescriptorSetLayoutManager::createDescriptorSetLayout(const DescriptorBindings &bindings) {
		for (uint64_t id = 0; id < getCount(); id++) {
			auto &layout = getById(id);

			if (layout.descriptorBindings.size() != bindings.size())
				continue;

			if (layout.descriptorBindings == bindings) {
				layout.layoutUsageCount++;
				return createById(id, [&](uint64_t id) {
					destroyById(id);
				});
			}
		}

		// create the descriptor set's layout and binding flags by iterating over its bindings
		Vector<vk::DescriptorSetLayoutBinding> bindingsVector = {};
		Vector<vk::DescriptorBindingFlags> bindingsFlags = {};

		for (auto bindingElem : bindings) {
			DescriptorBinding binding = bindingElem.second;
			uint32_t bindingID = bindingElem.first;

			bindingsVector.emplace_back(bindingID, getVkDescriptorType(binding.descriptorType),
										binding.descriptorCount,
										getShaderStageFlags(binding.shaderStages), nullptr);

			vk::DescriptorBindingFlags flags;

			if (binding.variableCount)
				flags |= vk::DescriptorBindingFlagBits::eVariableDescriptorCount;

			if (binding.partialBinding)
				flags |= vk::DescriptorBindingFlagBits::ePartiallyBound;

			bindingsFlags.push_back(flags);
		}

		vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo(bindingsFlags.size(),
																	   bindingsFlags.data());

		// create the descriptor set's layout from the binding data gathered above
		vk::DescriptorSetLayout vulkanHandle;
		vk::DescriptorSetLayoutCreateInfo layoutInfo(vk::DescriptorSetLayoutCreateFlags(),
													 bindingsVector);
		layoutInfo.setPNext(&bindingFlagsInfo);

		auto result = getCore().getContext().getDevice().createDescriptorSetLayout(
			&layoutInfo, nullptr, &vulkanHandle);
		if (result != vk::Result::eSuccess) {
			vkcv_log(LogLevel::ERROR, "Failed to create descriptor set layout");
			return DescriptorSetLayoutHandle();
		};

		return add({ vulkanHandle, bindings, 1 });
	}

	const DescriptorSetLayoutEntry &DescriptorSetLayoutManager::getDescriptorSetLayout(
		const DescriptorSetLayoutHandle &handle) const {
		return (*this) [handle];
	}

} // namespace vkcv
