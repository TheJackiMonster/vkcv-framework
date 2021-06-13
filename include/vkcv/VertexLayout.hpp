#pragma once

#include <vector>
#include <iostream>
#include <string>

namespace vkcv{
    enum class VertexAttachmentFormat{
        FLOAT,
        FLOAT2,
        FLOAT3,
        FLOAT4,
        INT,
        INT2,
        INT3,
        INT4
    };

	uint32_t getFormatSize(VertexAttachmentFormat format);

    struct VertexAttachment{
        /**
         * Describes an individual vertex input attribute/attachment.
         * @param inputLocation its location in the vertex shader.
         * @param name the name referred to in the shader.
         * @param format the format (and therefore, the size) this attachment is in.
         * @param offset the attachment's byte offset within a vertex.
         */
        VertexAttachment(uint32_t inputLocation, const std::string &name, VertexAttachmentFormat format, uint32_t offset) noexcept;
        VertexAttachment() = delete;

        uint32_t                inputLocation;
        std::string             name;
        VertexAttachmentFormat  format;
        uint32_t                offset;
    };

    struct VertexBinding{
        /**
         * Describes all vertex input attachments _one_ buffer contains to create a vertex buffer binding.
         * NOTE: multiple vertex layouts may contain various (mutually exclusive) vertex input attachments
         * to form one complete vertex buffer binding!
         * @param bindingLocation its entry in the buffers that make up the whole vertex buffer.
         * @param attachments the vertex input attachments this specific buffer layout contains.
         */
        VertexBinding(uint32_t bindingLocation, const std::vector<VertexAttachment> &attachments) noexcept;
        VertexBinding() = delete;

        uint32_t                        bindingLocation;
        uint32_t                        stride;
        std::vector<VertexAttachment>   vertexAttachments;
    };

    struct VertexLayout{
        /**
         * Describes the complete layout of one vertex, e.g. all of the vertex input attachments used,
         * and all of the buffer bindings that refer to the attachments (for when multiple buffers are used).
         * @param bindings bindings the complete vertex buffer is comprised of.
         */
        VertexLayout() noexcept;
        VertexLayout(const std::vector<VertexBinding> &bindings) noexcept;

        std::vector<VertexBinding> vertexBindings;
    };
}