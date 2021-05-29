//
// Created by Charlotte on 28.05.2021.
//

#include "vkcv/VertexLayout.hpp"

namespace vkcv {
    uint32_t static getFormatSize(VertexFormat format) {
        switch (format) {
            case VertexFormat::FLOAT:
                return 4;
            case VertexFormat::FLOAT2:
                return 8;
            case VertexFormat::FLOAT3:
                return 12;
            case VertexFormat::FLOAT4:
                return 16;
            case VertexFormat::INT:
                return 4;
            case VertexFormat::INT2:
                return 8;
            case VertexFormat::INT3:
                return 12;
            case VertexFormat::INT4:
                return 16;
            default:
                break;
        }
        std::cout << "VertexLayout: No format given" << std::endl;
        return 0;
    }

    VertexInputAttachment::VertexInputAttachment(uint32_t location, uint32_t binding, VertexFormat format, uint32_t offset) noexcept:
            location{location},
            binding{binding},
            format{format},
            offset{offset}
            {}

    VertexLayout::VertexLayout() noexcept :
    stride{0},
    attachmentMap()
    {}

    VertexLayout::VertexLayout(const std::vector<VertexInputAttachment> &inputs) noexcept {
        stride = 0;
        for (const auto &input : inputs) {
            attachmentMap.insert(std::make_pair(input.location, input));
            stride += getFormatSize(input.format);
        }
    }

	vk::Format vertexFormatToVulkanFormat(const VertexFormat format) {
		switch (format) {
			case VertexFormat::FLOAT	: return vk::Format::eR32Sfloat;
			case VertexFormat::FLOAT2	: return vk::Format::eR32G32Sfloat;
			case VertexFormat::FLOAT3	: return vk::Format::eR32G32B32Sfloat;
			case VertexFormat::FLOAT4	: return vk::Format::eR32G32B32A32Sfloat;
			case VertexFormat::INT		: return vk::Format::eR32Sint;
			case VertexFormat::INT2		: return vk::Format::eR32G32Sint;
			case VertexFormat::INT3		: return vk::Format::eR32G32B32Sint;
			case VertexFormat::INT4		: return vk::Format::eR32G32B32A32Sint;
			default: std::cerr << "Warning: Unknown vertex format" << std::endl; return vk::Format::eUndefined;
		}
	}

}