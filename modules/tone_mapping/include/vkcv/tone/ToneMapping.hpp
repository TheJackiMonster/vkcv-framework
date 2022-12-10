#pragma once

#include <vkcv/Core.hpp>

namespace vkcv::tone {
	
	/**
     * @defgroup vkcv_tone Tone Mapping Module
     * A module to apply tone mapping to an image in realtime.
     * @{
     */
	
	class ToneMapping {
	private:
		/**
         * Reference to the current Core instance.
         */
		Core& m_core;
		
		/**
		 * The name of the tone mapping instance.
		 */
		std::string m_name;
		
		/**
		 * Flag whether tone mapping should normalize before mapping.
		 */
		bool m_normalize;
		
		/**
		 * The compute pipeline of the tone mapping instance.
		 */
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
         * @param[in] normalize (Optional) Flag to normalize color values
         */
		explicit ToneMapping(Core& core,
							 const std::string& name,
							 bool normalize = false);
	
		~ToneMapping() = default;
		
		/**
		 * Return name of the tone mapping instance.
		 *
		 * @return Name of the tone mapping
		 */
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
	
	/** @} */
	
}
