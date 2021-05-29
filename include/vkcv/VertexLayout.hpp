#pragma once

#include <unordered_map>
#include <vector>
#include <iostream>
#include <vulkan/vulkan.hpp>

namespace vkcv{
    enum class VertexFormat{
        FLOAT,
        FLOAT2,
        FLOAT3,
        FLOAT4,
        INT,
        INT2,
        INT3,
        INT4
    };

    struct VertexInputAttachment{
        VertexInputAttachment() = delete;
        VertexInputAttachment(uint32_t location, uint32_t binding, VertexFormat format, uint32_t offset) noexcept;

        uint32_t location;
        uint32_t binding;
        VertexFormat format;
        uint32_t offset;
    };

    struct VertexLayout{
        VertexLayout() noexcept;
        VertexLayout(const std::vector<VertexInputAttachment> &inputs) noexcept;
        std::unordered_map<uint32_t, VertexInputAttachment> attachmentMap;
        uint32_t stride;
    };

	// currently assuming default 32 bit formats, no lower precision or normalized variants supported
	vk::Format vertexFormatToVulkanFormat(const VertexFormat format);
}