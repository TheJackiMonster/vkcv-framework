
#include "vkcv/tone/ToneMapping.hpp"

#include <vkcv/shader/GLSLCompiler.hpp>

#include <sstream>

namespace vkcv::tone {
	
	static DescriptorBindings getDescriptorBindings() {
		DescriptorBindings descriptorBindings = {};
		
		auto binding_0 = DescriptorBinding {
				0,
				DescriptorType::IMAGE_STORAGE,
				1,
				ShaderStage::COMPUTE,
				false,
				false
		};
		
		auto binding_1 = DescriptorBinding {
				1,
				DescriptorType::IMAGE_STORAGE,
				1,
				ShaderStage::COMPUTE,
				false,
				false
		};
		
		descriptorBindings.insert(std::make_pair(0, binding_0));
		descriptorBindings.insert(std::make_pair(1, binding_1));
		
		return descriptorBindings;
	}
	
	ShaderProgram ToneMapping::compileShaderProgram(const std::string &functionName,
													const std::string &functionSource) {
		vkcv::shader::GLSLCompiler compiler;
		ShaderProgram program;
		
		std::ostringstream stream;
		stream << "#version 450" << std::endl;
		stream << "layout(set=0, binding=0, rgba16f) restrict readonly uniform image2D inImage;" << std::endl;
		stream << "layout(set=0, binding=1, rgba8) restrict writeonly uniform image2D outImage;" << std::endl;
		stream << "layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;" << std::endl;
		stream << functionSource << std::endl;
		stream << "void main() {" << std::endl;
		stream << "  if (any(greaterThanEqual(gl_GlobalInvocationID.xy, imageSize(inImage)))) {" << std::endl;
		stream << "    return;" << std::endl;
		stream << "  }" << std::endl;
		stream << "  if (any(greaterThanEqual(gl_GlobalInvocationID.xy, imageSize(outImage)))) {" << std::endl;
		stream << "    return;" << std::endl;
		stream << "  }" << std::endl;
		stream << "  ivec2 uv = ivec2(gl_GlobalInvocationID.xy);" << std::endl;
		stream << "  vec4 color = imageLoad(inImage, uv);" << std::endl;
		
		if (m_normalize) {
			stream << "  color /= color.w;" << std::endl;
		}
		
		stream << "  color = vec4(" << functionName << "(color.xyz), color.w);" << std::endl;
		stream << "  imageStore(outImage, uv, color);" << std::endl;
		stream << "}" << std::endl;
		
		compiler.compileSource(
				ShaderStage::COMPUTE,
				stream.str().c_str(),
				[&](ShaderStage stage, const std::filesystem::path &path) {
					program.addShader(stage, path);
				}
		);
		
		return program;
	}
	
	void ToneMapping::buildComputePipeline(const std::string &functionName,
										   const std::string &functionSource) {
		const ShaderProgram program = compileShaderProgram(
				functionName,
				functionSource
		);
		
		m_descriptorSetLayout = m_core.createDescriptorSetLayout(
				getDescriptorBindings()
		);
		
		m_descriptorSet = m_core.createDescriptorSet(m_descriptorSetLayout);
		
		m_pipeline = m_core.createComputePipeline({
			program,
			{ m_descriptorSetLayout }
		});
	}
	
	ToneMapping::ToneMapping(Core &core,
							 const std::string &name,
							 bool normalize)
	: m_core(core),
	  m_name(name),
	  m_normalize(normalize),
	  m_pipeline(),
	  m_descriptorSetLayout(),
	  m_descriptorSet() {}
	
	const std::string &ToneMapping::getName() const {
		return m_name;
	}
	
	void ToneMapping::recordToneMapping(const CommandStreamHandle& cmdStream,
										const ImageHandle& input,
										const ImageHandle& output) {
		m_core.recordBeginDebugLabel(cmdStream, m_name, std::array<float, 4>{
			0.75f, 0.75f, 0.75f, 1.0f
		});
		
		m_core.prepareImageForStorage(cmdStream, input);
		m_core.prepareImageForStorage(cmdStream, output);
		
		vkcv::DescriptorWrites writes;
		
		writes.writeStorageImage(0, input);
		writes.writeStorageImage(1, output);
		
		m_core.writeDescriptorSet(m_descriptorSet, writes);
		
		const uint32_t width = m_core.getImageWidth(output);
		const uint32_t height = m_core.getImageHeight(output);
		
		m_core.recordComputeDispatchToCmdStream(
				cmdStream,
				m_pipeline,
				dispatchInvocations(
						DispatchSize(width, height),
						DispatchSize(8, 8)
				),
				{ useDescriptorSet(0, m_descriptorSet) },
				PushConstants(0)
		);
		
		m_core.recordEndDebugLabel(cmdStream);
	}
	
}
