#pragma once

#include <vulkan/vulkan.hpp>

namespace vkcv {

	enum class ShaderStage : VkShaderStageFlags {
		VERTEX          = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eVertex),
		TESS_CONTROL    = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eTessellationControl),
		TESS_EVAL       = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eTessellationEvaluation),
		GEOMETRY        = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eGeometry),
		FRAGMENT        = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eFragment),
		COMPUTE         = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eCompute),
		TASK            = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eTaskNV),
		MESH            = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eMeshNV),
		RAY_GEN          = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eRaygenKHR), // RTX
		RAY_ANY_HIT         = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eAnyHitKHR), // RTX
		RAY_CLOSEST_HIT     = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eClosestHitKHR), // RTX
		RAY_MISS            = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eMissKHR), // RTX
		RAY_INTERSECTION    = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eIntersectionKHR), // RTX
		RAY_CALLABLE        = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eCallableKHR) // RTX
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
