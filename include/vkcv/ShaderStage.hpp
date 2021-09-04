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
		RAYGEN          = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eRaygenKHR),
		ANY_HIT         = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eAnyHitKHR),
		CLOSEST_HIT     = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eClosestHitKHR),
		MISS            = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eMissKHR),
		INTERSECTION    = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eIntersectionKHR),
		CALLABLE        = static_cast<VkShaderStageFlags>(vk::ShaderStageFlagBits::eCallableKHR)
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
