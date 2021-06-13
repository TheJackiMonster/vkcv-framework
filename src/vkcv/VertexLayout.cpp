//
// Created by Charlotte on 28.05.2021.
//

#include "vkcv/VertexLayout.hpp"
#include "vkcv/Logger.hpp"

namespace vkcv {
    uint32_t getFormatSize(VertexAttachmentFormat format) {
        switch (format) {
            case VertexAttachmentFormat::FLOAT:
                return 4;
            case VertexAttachmentFormat::FLOAT2:
                return 8;
            case VertexAttachmentFormat::FLOAT3:
                return 12;
            case VertexAttachmentFormat::FLOAT4:
                return 16;
            case VertexAttachmentFormat::INT:
                return 4;
            case VertexAttachmentFormat::INT2:
                return 8;
            case VertexAttachmentFormat::INT3:
                return 12;
            case VertexAttachmentFormat::INT4:
                return 16;
            default:
				vkcv_log(LogLevel::WARNING, "No format given");
                return 0;
        }
    }

    VertexAttachment::VertexAttachment(uint32_t inputLocation, const std::string &name, VertexAttachmentFormat format, uint32_t offset) noexcept:
            inputLocation{inputLocation},
            name{name},
            format{format},
            offset{offset}
    {}


    VertexBinding::VertexBinding(uint32_t bindingLocation, const std::vector<VertexAttachment> &attachments) noexcept :
    bindingLocation{bindingLocation},
    stride{0},
    vertexAttachments{attachments}
    {
        for (const auto &attachment : attachments)
            stride += getFormatSize(attachment.format);
    }

    VertexLayout::VertexLayout() noexcept :
    vertexBindings{}
    {}

    VertexLayout::VertexLayout(const std::vector<VertexBinding> &bindings) noexcept :
    vertexBindings{bindings}
    {}
}