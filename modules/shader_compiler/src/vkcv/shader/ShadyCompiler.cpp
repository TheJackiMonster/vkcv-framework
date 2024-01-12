
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

	static bool shadyCompileModule(Module* module,
								   ShaderStage shaderStage,
								   const std::filesystem::path &shaderPath,
								   const ShaderCompiledFunction &compiled,
								   const std::filesystem::path &includePath) {
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
				return false;
		}

		const std::filesystem::path tmp_path = generateTemporaryFilePath();

		DriverConfig config = default_driver_config();

		config.output_filename = tmp_path.c_str();

		// TODO

		codes = driver_compile(&config, module);
		destroy_driver_config(&config);

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
				return false;
		}

		if (compiled) {
			compiled(shaderStage, tmp_path);
		}

		std::filesystem::remove(tmp_path);
		return true;
	}

	static bool shadyCompileArena(IrArena* arena,
								  ShaderStage shaderStage,
								  const std::filesystem::path &shaderPath,
								  const ShaderCompiledFunction &compiled,
								  const std::filesystem::path &includePath) {
		Module* module = new_module(arena, "main");

		if (nullptr == module) {
			vkcv_log(LogLevel::ERROR, "Module could not be created");
			return false;
		}

		bool result = shadyCompileModule(module, shaderStage, shaderPath, compiled, includePath);

		destroy_module(module);
		return result;
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

		bool result = shadyCompileArena(arena, shaderStage, shaderPath, compiled, includePath);

		destroy_ir_arena(arena);

		if (!result) {
			vkcv_log(LogLevel::ERROR, "Shader compilation failed: (%s)", shaderPath.string().c_str());
		}

		if (update) {
			// TODO: Shader hot compilation during runtime
		}
	}
	
}
