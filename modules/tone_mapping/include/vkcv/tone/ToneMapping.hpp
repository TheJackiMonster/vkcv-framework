#pragma once

#include <vkcv/Core.hpp>

namespace vkcv::tone {
	
	class ToneMapping {
	private:
		/**
         * Reference to the current Core instance.
         */
		Core& m_core;
		
		std::string m_name;
		
		ComputePipelineHandle m_pipeline;
		
		/**
         * The descriptor set layout of the tone mapping pipeline.
         */
		DescriptorSetLayoutHandle m_descriptorSetLayout;
		
		/**
		 * The descriptor set for the tone mapping pipeline.
		 */
		DescriptorSetHandle m_descriptorSet;
		
	protected:
		ShaderProgram compileShaderProgram(const std::string& functionName,
										   const std::string& functionSource);
		
		void buildComputePipeline(const std::string& functionName,
								  const std::string& functionSource);
		
		virtual void initToneMapping() = 0;
		
	public:
		/**
         * Constructor to create an tone mapping instance.
         *
         * @param[in,out] core Reference to a Core instance
         * @param[in] name Name of the tone mapping function
         */
		explicit ToneMapping(Core& core, const std::string& name);
	
		~ToneMapping() = default;
		
		[[nodiscard]]
		const std::string& getName() const;
		
		/**
		 * Record the commands of the given tone mapping instance to
		 * process the image of the input handle mapping its colors into
		 * the regarding output image handle.
		 *
		 * @param[in] cmdStream Command stream handle to record commands
		 * @param[in] input Input image handle
		 * @param[in] output Output image handle
		 */
		void recordToneMapping(const CommandStreamHandle& cmdStream,
							   const ImageHandle& input,
							   const ImageHandle& output);
		
	};
	
}
