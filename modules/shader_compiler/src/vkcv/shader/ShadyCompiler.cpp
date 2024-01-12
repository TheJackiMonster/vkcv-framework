
#include "vkcv/shader/ShadyCompiler.hpp"

#include <vkcv/File.hpp>
#include <vkcv/Logger.hpp>

#include <shady/driver.h>

namespace vkcv::shader {
	
	ShadyCompiler::ShadyCompiler() : Compiler() {
		// TODO
	}
	
	ShadyCompiler::ShadyCompiler(const ShadyCompiler &other) : Compiler(other) {
		// TODO
	}
	
	ShadyCompiler::~ShadyCompiler() {
		// TODO
	}
	
	ShadyCompiler &ShadyCompiler::operator=(const ShadyCompiler &other) {
		// TODO
		return *this;
	}

	static void shadyCompileModule(Module* module,
								   ShaderStage shaderStage,
								   const std::filesystem::path &shaderPath,
								   const ShaderCompiledFunction &compiled,
								   const std::filesystem::path &includePath,
								   bool update) {
		ShadyErrorCodes codes = driver_load_source_file_from_filename(
			shaderPath.c_str(), module
		);

		switch (codes) {
			case NoError:
				break;
			case MissingInputArg:
			case MissingOutputArg:
			case InputFileDoesNotExist:
			case InputFileIOError:
			case MissingDumpCfgArg:
			case MissingDumpIrArg:
			case IncorrectLogLevel:
			case InvalidTarget:
			case ClangInvocationFailed:
			default:
				vkcv_log(LogLevel::ERROR, "Unknown error while loading shader");
				return;
		}

		DriverConfig config = default_driver_config();

		// TODO

		codes = driver_compile(&config, module);

		switch (codes) {
			case NoError:
				break;
			case MissingInputArg:
			case MissingOutputArg:
			case InputFileDoesNotExist:
			case InputFileIOError:
			case MissingDumpCfgArg:
			case MissingDumpIrArg:
			case IncorrectLogLevel:
			case InvalidTarget:
			case ClangInvocationFailed:
			default:
				vkcv_log(LogLevel::ERROR, "Unknown error while compiling shader");
				return;
		}

		if (compiled) {
			compiled(shaderStage, shaderPath);
		}
	}

	static void shadyCompileArena(IrArena* arena,
								  ShaderStage shaderStage,
								  const std::filesystem::path &shaderPath,
								  const ShaderCompiledFunction &compiled,
								  const std::filesystem::path &includePath,
								  bool update) {
		Module* module = new_module(arena, "NAME");

		if (nullptr == module) {
			vkcv_log(LogLevel::ERROR, "Module could not be created");
			return;
		}

		shadyCompileModule(module, shaderStage, shaderPath, compiled, includePath, update);
	}
	
	void ShadyCompiler::compile(ShaderStage shaderStage,
								const std::filesystem::path &shaderPath,
								const ShaderCompiledFunction &compiled,
								const std::filesystem::path &includePath,
								bool update) {
		ArenaConfig config {};

		// TODO

		IrArena* arena = new_ir_arena(config);

		if (nullptr == arena) {
			vkcv_log(LogLevel::ERROR, "IR Arena could not be created");
			return;
		}

		shadyCompileArena(arena, shaderStage, shaderPath, compiled, includePath, update);
		destroy_ir_arena(arena);
	}
	
}
