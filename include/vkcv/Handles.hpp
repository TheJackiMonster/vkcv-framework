#pragma once
/**
 * @authors Artur Wasmut
 * @file src/vkcv/Handles.cpp
 * @brief Central header file for all possible handles that the framework will hand out.
 */

#include <cstdint>

namespace vkcv
{
    // Handle returned for any buffer created with the core/context objects
    struct BufferHandle     {uint64_t id;};
    struct PassHandle       {uint64_t id;};
    struct PipelineHandle   {uint64_t id;};
    struct ResourcesHandle  {uint64_t id;};
    struct SamplerHandle	{uint64_t id;};
}
