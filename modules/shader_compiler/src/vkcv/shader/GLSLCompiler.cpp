
#include "vkcv/shader/GLSLCompiler.hpp"

#include <fstream>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <vkcv/Logger.hpp>

namespace vkcv::shader {
	
	static uint32_t s_CompilerCount = 0;
	
	GLSLCompiler::GLSLCompiler() {
		if (s_CompilerCount == 0) {
			glslang::InitializeProcess();
		}
		
		s_CompilerCount++;
	}
	
	GLSLCompiler::GLSLCompiler(const GLSLCompiler &other) {
		s_CompilerCount++;
	}
	
	GLSLCompiler::~GLSLCompiler() {
		s_CompilerCount--;
		
		if (s_CompilerCount == 0) {
			glslang::FinalizeProcess();
		}
	}
	
	GLSLCompiler &GLSLCompiler::operator=(const GLSLCompiler &other) {
		s_CompilerCount++;
		return *this;
	}
	
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
		resources.maxMeshViewCountNV = 4;
		resources.limits.nonInductiveForLoops = 1;
		resources.limits.whileLoops = 1;
		resources.limits.doWhileLoops = 1;
		resources.limits.generalUniformIndexing = 1;
		resources.limits.generalAttributeMatrixVectorIndexing = 1;
		resources.limits.generalVaryingIndexing = 1;
		resources.limits.generalSamplerIndexing = 1;
		resources.limits.generalVariableIndexing = 1;
		resources.limits.generalConstantMatrixVectorIndexing = 1;
	}
	
	static std::vector<char> readShaderCode(const std::filesystem::path &shaderPath) {
		std::ifstream file (shaderPath.string(), std::ios::ate);
		
		if (!file.is_open()) {
			vkcv_log(LogLevel::ERROR, "The file could not be opened (%s)", shaderPath.string().c_str());
			return std::vector<char>{};
		}
		
		std::streamsize fileSize = file.tellg();
		std::vector<char> buffer (fileSize + 1);
		
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		
		buffer[fileSize] = '\0';
		return buffer;
	}
	
	static bool writeSpirvCode(const std::filesystem::path &shaderPath, const std::vector<uint32_t>& spirv) {
		std::ofstream file (shaderPath.string(), std::ios::out | std::ios::binary);
		
		if (!file.is_open()) {
			vkcv_log(LogLevel::ERROR, "The file could not be opened (%s)", shaderPath.string().c_str());
			return false;
		}
		
		const auto fileSize = static_cast<std::streamsize>(
				sizeof(uint32_t) * spirv.size()
		);
		
		file.seekp(0);
		file.write(reinterpret_cast<const char*>(spirv.data()), fileSize);
		file.close();
		
		return true;
	}
	
	void GLSLCompiler::compile(ShaderStage shaderStage, const std::filesystem::path &shaderPath,
							   const ShaderCompiledFunction& compiled, bool update) {
		const EShLanguage language = findShaderLanguage(shaderStage);
		
		if (language == EShLangCount) {
			vkcv_log(LogLevel::ERROR, "Shader stage not supported (%s)", shaderPath.string().c_str());
			return;
		}
		
		const std::vector<char> code = readShaderCode(shaderPath);
		
		glslang::TShader shader (language);
		glslang::TProgram program;
		
		const char *shaderStrings [1];
		shaderStrings[0] = code.data();
		
		shader.setStrings(shaderStrings, 1);
		
		TBuiltInResource resources = {};
		initResources(resources);
		
		const auto messages = (EShMessages) (
				EShMsgSpvRules |
				EShMsgVulkanRules
		);
		
		if (!shader.parse(&resources, 100, false, messages)) {
			vkcv_log(LogLevel::ERROR, "Shader parsing failed {\n%s\n%s\n} (%s)",
					 shader.getInfoLog(), shader.getInfoDebugLog(), shaderPath.string().c_str());
			return;
		}
		
		program.addShader(&shader);
		
		if (!program.link(messages)) {
			vkcv_log(LogLevel::ERROR, "Shader linking failed {\n%s\n%s\n} (%s)",
					 shader.getInfoLog(), shader.getInfoDebugLog(), shaderPath.string().c_str());
			return;
		}
		
		const glslang::TIntermediate* intermediate = program.getIntermediate(language);
		
		if (!intermediate) {
			vkcv_log(LogLevel::ERROR, "No valid intermediate representation (%s)", shaderPath.string().c_str());
			return;
		}
		
		std::vector<uint32_t> spirv;
		glslang::GlslangToSpv(*intermediate, spirv);
		
		const std::filesystem::path tmp_path (std::tmpnam(nullptr));
		
		if (!writeSpirvCode(tmp_path, spirv)) {
			vkcv_log(LogLevel::ERROR, "Spir-V could not be written to disk (%s)", shaderPath.string().c_str());
			return;
		}
		
		if (compiled) {
			compiled(shaderStage, tmp_path);
		}
		
		std::filesystem::remove(tmp_path);
		
		if (update) {
			// TODO: Shader hot compilation during runtime
		}
	}
	
}
