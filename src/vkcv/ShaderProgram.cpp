/**
 * @authors Simeon Hermann, Leonie Franken, Tobias Frisch
 * @file src/vkcv/ShaderProgram.cpp
 * @brief ShaderProgram class to handle and prepare the shader stages for a graphics pipeline
 */

#include "vkcv/ShaderProgram.hpp"

#include "vkcv/File.hpp"
#include "vkcv/Logger.hpp"
#include <cstddef>
#include <cstdint>
#include <limits>

namespace vkcv {

	VertexAttachmentFormat convertFormat(spirv_cross::SPIRType::BaseType basetype,
										 uint32_t vecsize) {
		switch (basetype) {
		case spirv_cross::SPIRType::Int:
			switch (vecsize) {
			case 1:
				return VertexAttachmentFormat::INT;
			case 2:
				return VertexAttachmentFormat::INT2;
			case 3:
				return VertexAttachmentFormat::INT3;
			case 4:
				return VertexAttachmentFormat::INT4;
			default:
				break;
			}
			break;
		case spirv_cross::SPIRType::Float:
			switch (vecsize) {
			case 1:
				return VertexAttachmentFormat::FLOAT;
			case 2:
				return VertexAttachmentFormat::FLOAT2;
			case 3:
				return VertexAttachmentFormat::FLOAT3;
			case 4:
				return VertexAttachmentFormat::FLOAT4;
			default:
				break;
			}
			break;
		default:
			break;
		}

		vkcv_log(LogLevel::WARNING, "Unknown vertex format");
		return VertexAttachmentFormat::FLOAT;
	}

	ShaderProgram::ShaderProgram() noexcept :
		m_Shaders {}, m_VertexAttachments {}, m_DescriptorSets {} {}

	bool ShaderProgram::addShader(ShaderStage stage, const std::filesystem::path &path) {
		if (m_Shaders.find(stage) != m_Shaders.end()) {
			vkcv_log(LogLevel::WARNING, "Overwriting existing shader stage");
		}

		Vector<uint32_t> shaderCode;
		if ((!readBinaryFromFile(path, shaderCode)) || (shaderCode.empty())) {
			return false;
		}
		
		m_Shaders.insert(std::make_pair(stage, shaderCode));
		reflectShader(stage);
		return true;
	}

	const Vector<uint32_t> &ShaderProgram::getShaderBinary(ShaderStage stage) const {
		return m_Shaders.at(stage);
	}

	bool ShaderProgram::existsShader(ShaderStage stage) const {
		if (m_Shaders.find(stage) == m_Shaders.end())
			return false;
		else
			return true;
	}

	static void reflectShaderDescriptorSets(Dictionary<uint32_t, DescriptorBindings> &descriptorSets,
	                                        ShaderStage shaderStage,
	                                        DescriptorType descriptorType,
	                                        const spirv_cross::Compiler &comp,
	                                        const spirv_cross::ShaderResources &resources) {
		const spirv_cross::SmallVector<spirv_cross::Resource> *res = nullptr;

		switch (descriptorType) {
			case DescriptorType::UNIFORM_BUFFER:
			  res = &(resources.uniform_buffers);
			  break;
			case DescriptorType::STORAGE_BUFFER:
			  res = &(resources.storage_buffers);
			  break;
			case DescriptorType::SAMPLER:
			  res = &(resources.separate_samplers);
			  break;
			case DescriptorType::IMAGE_SAMPLED:
			  res = &(resources.separate_images);
			  break;
			case DescriptorType::IMAGE_STORAGE:
			  res = &(resources.storage_images);
			  break;
			case DescriptorType::UNIFORM_BUFFER_DYNAMIC:
			  res = &(resources.uniform_buffers);
			  break;
			case DescriptorType::STORAGE_BUFFER_DYNAMIC:
			  res = &(resources.storage_buffers);
			  break;
			case DescriptorType::ACCELERATION_STRUCTURE_KHR:
			  res = &(resources.acceleration_structures);
			  break;
			default:
			  break;
		}

		if (nullptr == res) {
			return;
		}

		for (uint32_t i = 0; i < res->size(); i++) {
			const spirv_cross::Resource &u = (*res)[i];
			const spirv_cross::SPIRType &base_type = comp.get_type(u.base_type_id);
			const spirv_cross::SPIRType &type = comp.get_type(u.type_id);

			uint32_t setID = comp.get_decoration(u.id, spv::DecorationDescriptorSet);
			uint32_t bindingID = comp.get_decoration(u.id, spv::DecorationBinding);

			uint32_t descriptorCount = base_type.vecsize;

			bool variableCount = false;
			// query whether reflected resources are qualified as one-dimensional array
			if (descriptorCount == 0) {
				variableCount = true;
			}

			DescriptorBinding binding {
				bindingID,
				descriptorType,
				descriptorCount,
				shaderStage,
				variableCount,
				variableCount // partialBinding == variableCount
			};

			auto insertionResult = descriptorSets[setID].insert(std::make_pair(bindingID, binding));
			if (!insertionResult.second) {
				insertionResult.first->second.shaderStages |= shaderStage;
				
				vkcv_log(LogLevel::WARNING,
						 "Attempting to overwrite already existing binding %u at set ID %u.",
						 bindingID, setID);
			}
		}
	}

	void ShaderProgram::reflectShader(ShaderStage shaderStage) {
		auto shaderCode = m_Shaders.at(shaderStage);

		spirv_cross::Compiler comp(shaderCode);
		spirv_cross::ShaderResources resources = comp.get_shader_resources();

		// reflect vertex input
		if (shaderStage == ShaderStage::VERTEX) {
			// spirv-cross API (hopefully) returns the stage_inputs in order
			for (uint32_t i = 0; i < resources.stage_inputs.size(); i++) {
				// spirv-cross specific objects
				auto &stage_input = resources.stage_inputs [i];
				const spirv_cross::SPIRType &base_type = comp.get_type(stage_input.base_type_id);

				// vertex input location
				const uint32_t attachment_loc =
					comp.get_decoration(stage_input.id, spv::DecorationLocation);
				// vertex input name
				const std::string attachment_name = stage_input.name;
				// vertex input format (implies its size)
				const VertexAttachmentFormat attachment_format =
					convertFormat(base_type.basetype, base_type.vecsize);

				m_VertexAttachments.push_back(
					{ attachment_loc, attachment_name, attachment_format, 0 });
			}
		}

		reflectShaderDescriptorSets(
			m_DescriptorSets,
			shaderStage,
			DescriptorType::UNIFORM_BUFFER,
			comp,
			resources
		);

		reflectShaderDescriptorSets(
			m_DescriptorSets,
			shaderStage,
			DescriptorType::STORAGE_BUFFER,
			comp,
			resources
		);

		reflectShaderDescriptorSets(
			m_DescriptorSets,
			shaderStage,
			DescriptorType::SAMPLER,
			comp,
			resources
		);

		reflectShaderDescriptorSets(
			m_DescriptorSets,
			shaderStage,
			DescriptorType::IMAGE_SAMPLED,
			comp,
			resources
		);

		reflectShaderDescriptorSets(
			m_DescriptorSets,
			shaderStage,
			DescriptorType::IMAGE_STORAGE,
			comp,
			resources
		);

		reflectShaderDescriptorSets(
			m_DescriptorSets,
			shaderStage,
			DescriptorType::ACCELERATION_STRUCTURE_KHR,
			comp,
			resources
		);

		for (auto &descriptorSet : m_DescriptorSets) {
			uint32_t maxVariableBindingID = 0;

			for (const auto &binding : descriptorSet.second) {
				maxVariableBindingID = std::max(maxVariableBindingID, binding.first);
			}

			for (auto &binding : descriptorSet.second) {
				if (binding.first < maxVariableBindingID) {
					binding.second.variableCount &= false;
					binding.second.partialBinding &= false;
				}
			}
		}

		// reflect push constants
		for (const auto &pushConstantBuffer : resources.push_constant_buffers) {
			for (const auto &range : comp.get_active_buffer_ranges(pushConstantBuffer.id)) {
				const size_t size = range.range + range.offset;
				m_pushConstantsSize = std::max(m_pushConstantsSize, size);
			}
		}
	}

	const VertexAttachments &ShaderProgram::getVertexAttachments() const {
		return m_VertexAttachments;
	}

	const Dictionary<uint32_t, DescriptorBindings> &
	ShaderProgram::getReflectedDescriptors() const {
		return m_DescriptorSets;
	}

	size_t ShaderProgram::getPushConstantsSize() const {
		return m_pushConstantsSize;
	}
} // namespace vkcv

