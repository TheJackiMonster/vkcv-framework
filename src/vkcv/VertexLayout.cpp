//
// Created by Charlotte on 28.05.2021.
//

#include "vkcv/VertexLayout.hpp"

namespace vkcv {
    uint32_t getFormatSize(VertexFormat format) {
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

    VertexInputAttachment::VertexInputAttachment(uint32_t location, uint32_t binding, std::string name, VertexFormat format, uint32_t offset) noexcept:
            location{location},
            binding{binding},
            name{name},
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

}