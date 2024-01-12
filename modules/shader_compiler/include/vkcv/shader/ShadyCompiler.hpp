#pragma once

#include <filesystem>

#include <vkcv/ShaderStage.hpp>
#include "Compiler.hpp"

namespace vkcv::shader {

    class ShadyCompiler : public Compiler {
    public:
        ShadyCompiler();

        ShadyCompiler(const ShadyCompiler& other);

        ShadyCompiler(ShadyCompiler&& other) = default;

        ~ShadyCompiler();

        ShadyCompiler& operator=(const ShadyCompiler& other);

        ShadyCompiler& operator=(ShadyCompiler&& other) = default;

        void compile(ShaderStage shaderStage,
					 const std::filesystem::path& shaderPath,
					 const ShaderCompiledFunction& compiled,
					 const std::filesystem::path& includePath = "",
					 bool update = false) override;

    };

}
