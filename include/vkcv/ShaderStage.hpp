#pragma once

#include <vulkan/vulkan.hpp>

namespace vkcv {
	
	enum class ShaderStage : VkShaderStageFlags {
		VERTEX = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eVertex),
		TESS_CONTROL = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eTessellationControl),
		TESS_EVAL = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eTessellationEvaluation),
		GEOMETRY = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eGeometry),
		FRAGMENT = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eFragment),
		COMPUTE = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eCompute)
	};
	
	using ShaderStages = vk::Flags<ShaderStage>;
	
	constexpr vk::ShaderStageFlags getShaderStageFlags(ShaderStages shaderStages) noexcept {
		return vk::ShaderStageFlags(static_cast<VkShaderStageFlags>(shaderStages));
	}
	
	constexpr ShaderStages operator|(ShaderStage stage0, ShaderStage stage1) noexcept {
		return ShaderStages(stage0) | stage1;
	}
	
	constexpr ShaderStages operator&(ShaderStage stage0, ShaderStage stage1) noexcept {
		return ShaderStages(stage0) & stage1;
	}
	
	constexpr ShaderStages operator^(ShaderStage stage0, ShaderStage stage1) noexcept {
		return ShaderStages(stage0) ^ stage1;
	}
	
	constexpr ShaderStages operator~(ShaderStage stage) noexcept {
		return ~(ShaderStages(stage));
	}
	
}
