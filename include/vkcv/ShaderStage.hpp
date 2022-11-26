#pragma once
/**
 * @authors Artur Wasmut, Simeon Hermann, Tobias Frisch, Vanessa Karolek, Alexander Gauggel, Lars
 * Hoerttrich
 * @file vkcv/ShaderStage.hpp
 * @brief Enum and struct to operate with multiple shader stages.
 */

#include <vulkan/vulkan.hpp>

namespace vkcv {

	/**
	 * @brief Enum class to specify the stage of a shader.
	 */
	enum class ShaderStage : VkShaderStageFlags {
		VERTEX = static_cast<VkShaderStageFlags>(VULKAN_HPP_NAMESPACE::ShaderStageFlagBits::eVertex),
		TESS_CONTROL =
			static_cast<VkShaderStageFlags>(VULKAN_HPP_NAMESPACE::ShaderStageFlagBits::eTessellationControl),
		TESS_EVAL =
			static_cast<VkShaderStageFlags>(VULKAN_HPP_NAMESPACE::ShaderStageFlagBits::eTessellationEvaluation),
		GEOMETRY = static_cast<VkShaderStageFlags>(VULKAN_HPP_NAMESPACE::ShaderStageFlagBits::eGeometry),
		FRAGMENT = static_cast<VkShaderStageFlags>(VULKAN_HPP_NAMESPACE::ShaderStageFlagBits::eFragment),
		COMPUTE = static_cast<VkShaderStageFlags>(VULKAN_HPP_NAMESPACE::ShaderStageFlagBits::eCompute),
		TASK = static_cast<VkShaderStageFlags>(VULKAN_HPP_NAMESPACE::ShaderStageFlagBits::eTaskNV),
		MESH = static_cast<VkShaderStageFlags>(VULKAN_HPP_NAMESPACE::ShaderStageFlagBits::eMeshNV),
		RAY_GEN = static_cast<VkShaderStageFlags>(VULKAN_HPP_NAMESPACE::ShaderStageFlagBits::eRaygenKHR),
		RAY_ANY_HIT = static_cast<VkShaderStageFlags>(VULKAN_HPP_NAMESPACE::ShaderStageFlagBits::eAnyHitKHR),
		RAY_CLOSEST_HIT = static_cast<VkShaderStageFlags>(VULKAN_HPP_NAMESPACE::ShaderStageFlagBits::eClosestHitKHR),
		RAY_MISS = static_cast<VkShaderStageFlags>(VULKAN_HPP_NAMESPACE::ShaderStageFlagBits::eMissKHR),
		RAY_INTERSECTION =
			static_cast<VkShaderStageFlags>(VULKAN_HPP_NAMESPACE::ShaderStageFlagBits::eIntersectionKHR),
		RAY_CALLABLE = static_cast<VkShaderStageFlags>(VULKAN_HPP_NAMESPACE::ShaderStageFlagBits::eCallableKHR)
	};

	using ShaderStages = VULKAN_HPP_NAMESPACE::Flags<ShaderStage>;

} // namespace vkcv

/**
 * @cond VULKAN_TYPES
 */
namespace VULKAN_HPP_NAMESPACE {

	template <>
	struct [[maybe_unused]] FlagTraits<vkcv::ShaderStage> {
		static VULKAN_HPP_CONST_OR_CONSTEXPR bool isBitmask = true;
		static VULKAN_HPP_CONST_OR_CONSTEXPR Flags<vkcv::ShaderStage> allFlags =
				vkcv::ShaderStage::VERTEX
				| vkcv::ShaderStage::TESS_CONTROL
				| vkcv::ShaderStage::TESS_EVAL
				| vkcv::ShaderStage::GEOMETRY
				| vkcv::ShaderStage::FRAGMENT
				| vkcv::ShaderStage::TASK
				| vkcv::ShaderStage::MESH
				| vkcv::ShaderStage::RAY_GEN
				| vkcv::ShaderStage::RAY_ANY_HIT
				| vkcv::ShaderStage::RAY_CLOSEST_HIT
				| vkcv::ShaderStage::RAY_MISS
				| vkcv::ShaderStage::RAY_INTERSECTION
				| vkcv::ShaderStage::RAY_CALLABLE;
	};

} // namespace vk
/**
 * @endcond
 */

namespace vkcv {

	constexpr VULKAN_HPP_NAMESPACE::ShaderStageFlags getShaderStageFlags(ShaderStages shaderStages) noexcept {
		return VULKAN_HPP_NAMESPACE::ShaderStageFlags(static_cast<VkShaderStageFlags>(shaderStages));
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

} // namespace vkcv
