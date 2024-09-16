
#include "vkcv/shader/GLSLCompiler.hpp"

#include <sstream>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/StandAlone/DirStackFileIncluder.h>

#include <vkcv/File.hpp>
#include <vkcv/Logger.hpp>

namespace vkcv::shader {
	
	GLSLCompiler::GLSLCompiler(GLSLCompileTarget target)
	: GlslangCompiler(), m_target(target) {}
	
	constexpr EShLanguage findShaderLanguage(ShaderStage shaderStage) {
		switch (shaderStage) {
			case ShaderStage::VERTEX:
				return EShLangVertex;
			case ShaderStage::TESS_CONTROL:
				return EShLangTessControl;
			case ShaderStage::TESS_EVAL:
				return EShLangTessEvaluation;
			case ShaderStage::GEOMETRY:
				return EShLangGeometry;
			case ShaderStage::FRAGMENT:
				return EShLangFragment;
			case ShaderStage::COMPUTE:
				return EShLangCompute;
			case ShaderStage::TASK:
				return EShLangTask;
			case ShaderStage::MESH:
				return EShLangMesh;
			case ShaderStage::RAY_GEN:
			    return EShLangRayGen;
			case ShaderStage::RAY_CLOSEST_HIT:
			    return EShLangClosestHit;
			case ShaderStage::RAY_MISS:
			    return EShLangMiss;
			case ShaderStage::RAY_INTERSECTION:
				return EShLangIntersect;
			case ShaderStage::RAY_ANY_HIT:
				return EShLangAnyHit;
			case ShaderStage::RAY_CALLABLE:
				return EShLangCallable;
			default:
				return EShLangCount;
		}
	}
	
	static void initResources(TBuiltInResource& resources) {
		resources.maxLights = 32;
		resources.maxClipPlanes = 6;
		resources.maxTextureUnits = 32;
		resources.maxTextureCoords = 32;
		resources.maxVertexAttribs = 64;
		resources.maxVertexUniformComponents = 4096;
		resources.maxVaryingFloats = 64;
		resources.maxVertexTextureImageUnits = 32;
		resources.maxCombinedTextureImageUnits = 80;
		resources.maxTextureImageUnits = 32;
		resources.maxFragmentUniformComponents = 4096;
		resources.maxDrawBuffers = 32;
		resources.maxVertexUniformVectors = 128;
		resources.maxVaryingVectors = 8;
		resources.maxFragmentUniformVectors = 16;
		resources.maxVertexOutputVectors = 16;
		resources.maxFragmentInputVectors = 15;
		resources.minProgramTexelOffset = -8;
		resources.maxProgramTexelOffset = 7;
		resources.maxClipDistances = 8;
		resources.maxComputeWorkGroupCountX = 65535;
		resources.maxComputeWorkGroupCountY = 65535;
		resources.maxComputeWorkGroupCountZ = 65535;
		resources.maxComputeWorkGroupSizeX = 1024;
		resources.maxComputeWorkGroupSizeY = 1024;
		resources.maxComputeWorkGroupSizeZ = 64;
		resources.maxComputeUniformComponents = 1024;
		resources.maxComputeTextureImageUnits = 16;
		resources.maxComputeImageUniforms = 8;
		resources.maxComputeAtomicCounters = 8;
		resources.maxComputeAtomicCounterBuffers = 1;
		resources.maxVaryingComponents = 60;
		resources.maxVertexOutputComponents = 64;
		resources.maxGeometryInputComponents = 64;
		resources.maxGeometryOutputComponents = 128;
		resources.maxFragmentInputComponents = 128;
		resources.maxImageUnits = 8;
		resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
		resources.maxCombinedShaderOutputResources = 8;
		resources.maxImageSamples = 0;
		resources.maxVertexImageUniforms = 0;
		resources.maxTessControlImageUniforms = 0;
		resources.maxTessEvaluationImageUniforms = 0;
		resources.maxGeometryImageUniforms = 0;
		resources.maxFragmentImageUniforms = 8;
		resources.maxCombinedImageUniforms = 8;
		resources.maxGeometryTextureImageUnits = 16;
		resources.maxGeometryOutputVertices = 256;
		resources.maxGeometryTotalOutputComponents = 1024;
		resources.maxGeometryUniformComponents = 1024;
		resources.maxGeometryVaryingComponents = 64;
		resources.maxTessControlInputComponents = 128;
		resources.maxTessControlOutputComponents = 128;
		resources.maxTessControlTextureImageUnits = 16;
		resources.maxTessControlUniformComponents = 1024;
		resources.maxTessControlTotalOutputComponents = 4096;
		resources.maxTessEvaluationInputComponents = 128;
		resources.maxTessEvaluationOutputComponents = 128;
		resources.maxTessEvaluationTextureImageUnits = 16;
		resources.maxTessEvaluationUniformComponents = 1024;
		resources.maxTessPatchComponents = 120;
		resources.maxPatchVertices = 32;
		resources.maxTessGenLevel = 64;
		resources.maxViewports = 16;
		resources.maxVertexAtomicCounters = 0;
		resources.maxTessControlAtomicCounters = 0;
		resources.maxTessEvaluationAtomicCounters = 0;
		resources.maxGeometryAtomicCounters = 0;
		resources.maxFragmentAtomicCounters = 8;
		resources.maxCombinedAtomicCounters = 8;
		resources.maxAtomicCounterBindings = 1;
		resources.maxVertexAtomicCounterBuffers = 0;
		resources.maxTessControlAtomicCounterBuffers = 0;
		resources.maxTessEvaluationAtomicCounterBuffers = 0;
		resources.maxGeometryAtomicCounterBuffers = 0;
		resources.maxFragmentAtomicCounterBuffers = 1;
		resources.maxCombinedAtomicCounterBuffers = 1;
		resources.maxAtomicCounterBufferSize = 16384;
		resources.maxTransformFeedbackBuffers = 4;
		resources.maxTransformFeedbackInterleavedComponents = 64;
		resources.maxCullDistances = 8;
		resources.maxCombinedClipAndCullDistances = 8;
		resources.maxSamples = 4;
		resources.maxMeshOutputVerticesNV = 256;
		resources.maxMeshOutputPrimitivesNV = 512;
		resources.maxMeshWorkGroupSizeX_NV = 32;
		resources.maxMeshWorkGroupSizeY_NV = 1;
		resources.maxMeshWorkGroupSizeZ_NV = 1;
		resources.maxTaskWorkGroupSizeX_NV = 32;
		resources.maxTaskWorkGroupSizeY_NV = 1;
		resources.maxTaskWorkGroupSizeZ_NV = 1;
		resources.maxMeshOutputVerticesEXT = 256;
		resources.maxMeshOutputPrimitivesEXT = 512;
		resources.maxMeshWorkGroupSizeX_EXT = 32;
		resources.maxMeshWorkGroupSizeY_EXT = 1;
		resources.maxMeshWorkGroupSizeZ_EXT = 1;
		resources.maxTaskWorkGroupSizeX_EXT = 32;
		resources.maxTaskWorkGroupSizeY_EXT = 1;
		resources.maxTaskWorkGroupSizeZ_EXT = 1;
		resources.maxMeshViewCountEXT = 4;
		resources.limits.nonInductiveForLoops = true;
		resources.limits.whileLoops = true;
		resources.limits.doWhileLoops = true;
		resources.limits.generalUniformIndexing = true;
		resources.limits.generalAttributeMatrixVectorIndexing = true;
		resources.limits.generalVaryingIndexing = true;
		resources.limits.generalSamplerIndexing = true;
		resources.limits.generalVariableIndexing = true;
		resources.limits.generalConstantMatrixVectorIndexing = true;
	}
	
	bool GLSLCompiler::compileSource(ShaderStage shaderStage,
									 const std::string& shaderSource,
									 const ShaderCompiledFunction &compiled,
									 const std::filesystem::path& includePath) {
		const EShLanguage language = findShaderLanguage(shaderStage);
		
		if (language == EShLangCount) {
			vkcv_log(LogLevel::ERROR, "Shader stage not supported");
			return false;
		}
		
		glslang::TShader shader (language);
		switch (m_target) {
			case GLSLCompileTarget::SUBGROUP_OP:
				shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
				shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
				break;
			case GLSLCompileTarget::RAY_TRACING:
				shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
				shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_4);
				break;
			case GLSLCompileTarget::MESH_SHADING:
				shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
				shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_4);
				break;
			default:
				break;
		}

		glslang::TProgram program;
		std::string source = processShaderSource(shaderSource);
		
		if (!m_defines.empty()) {
			std::ostringstream defines;
			for (const auto& define : m_defines) {
				defines << "#define " << define.first << " " << define.second << std::endl;
			}

			size_t pos = source.find("#version") + 8;
			if (pos >= source.length()) {
				pos = 0;
			}
			
			const size_t epos = source.find_last_of("#extension", pos) + 10;
			if (epos < source.length()) {
				pos = epos;
			}
			
			const auto defines_str = defines.str();
			
			pos = source.find('\n', pos) + 1;
			source = source.insert(pos, defines_str);
		}
		
		const char *shaderStrings [1];
		shaderStrings[0] = source.c_str();
		
		shader.setStrings(shaderStrings, 1);
		
		TBuiltInResource resources = {};
		initResources(resources);

		const auto messages = (EShMessages)(
			EShMsgSpvRules |
			EShMsgVulkanRules
		);

		std::string preprocessedGLSL;

		DirStackFileIncluder includer;
		includer.pushExternalLocalDirectory(includePath.string());

		if (!shader.preprocess(&resources, 100, ENoProfile,
							   false, false,
							   messages, &preprocessedGLSL, includer)) {
			vkcv_log(LogLevel::ERROR, "Shader preprocessing failed {\n%s\n%s\n}",
							 shader.getInfoLog(), shader.getInfoDebugLog());
			return false;
		}
		
		const char* preprocessedCString = preprocessedGLSL.c_str();
		shader.setStrings(&preprocessedCString, 1);

		if (!shader.parse(&resources, 100, false, messages)) {
			vkcv_log(LogLevel::ERROR, "Shader parsing failed {\n%s\n%s\n}",
							 shader.getInfoLog(), shader.getInfoDebugLog());
			return false;
		}
		
		program.addShader(&shader);
		
		if (!program.link(messages)) {
			vkcv_log(LogLevel::ERROR, "Shader linking failed {\n%s\n%s\n}",
							 shader.getInfoLog(), shader.getInfoDebugLog());
			return false;
		}
		
		const glslang::TIntermediate* intermediate = program.getIntermediate(language);
		
		if (!intermediate) {
			vkcv_log(LogLevel::ERROR, "No valid intermediate representation");
			return false;
		}
		
		std::vector<uint32_t> spirv;
		glslang::GlslangToSpv(*intermediate, spirv);
		
		const std::filesystem::path tmp_path = generateTemporaryFilePath();
		
		if (!writeBinaryToFile(tmp_path, spirv)) {
			vkcv_log(LogLevel::ERROR, "Spir-V could not be written to disk");
			return false;
		}
		
		if (compiled) {
			compiled(shaderStage, tmp_path);
		}
		
		std::filesystem::remove(tmp_path);
		return true;
	}
	
}
