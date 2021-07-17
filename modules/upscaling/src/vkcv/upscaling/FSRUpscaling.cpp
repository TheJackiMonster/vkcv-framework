
#include "vkcv/upscaling/FSRUpscaling.hpp"

#include <stdint.h>
#include <math.h>

#define A_CPU 1
#include <ffx_a.h>
#include <ffx_fsr1.h>

#include "ffx_a.h.hxx"
#include "ffx_fsr1.h.hxx"

#include <vkcv/shader/GLSLCompiler.hpp>

namespace vkcv::upscaling {
	
	static void setupTemporaryShaderDirectory() {
	
	}
	
	FSRUpscaling::FSRUpscaling() {
		vkcv::shader::GLSLCompiler compiler;
		compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("resources/shaders/shader.vert"),
						 [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
			m_program.addShader(shaderStage, path);
		});
	}

}
