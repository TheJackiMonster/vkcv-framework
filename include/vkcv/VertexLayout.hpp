#pragma once
/**
 * @authors Alexander Gauggel, Artur Wasmut, Mara Vogt, Susanne Dötsch,
 *          Trevor Hollmann, Leonie Franken, Simeon Hermann, Tobias Frisch
 * @file vkcv/VertexLayout.hpp
 * @brief Structures to handle vertex layout, bindings and attachments.
 */

#include <vector>
#include <iostream>
#include <string>

namespace vkcv {
	
	/**
	 * @brief Enum class to specify the format of vertex attributes.
	 */
    enum class VertexAttachmentFormat {
        FLOAT,
        FLOAT2,
        FLOAT3,
        FLOAT4,
        INT,
        INT2,
        INT3,
        INT4
    };

	/**
	 * @brief Returns the size in bytes of a vertex with a
	 * given vertex format.
	 *
	 * @param[in] format Vertex format
	 * @return Size in bytes
	 */
	uint32_t getFormatSize(VertexAttachmentFormat format);

	/**
	 * @brief Structure to store the details of a vertex input attachment.
	 *
	 * Describes an individual vertex input attribute/attachment. The offset
	 * is calculated when a collection of attachments forms a binding.
	 */
    struct VertexAttachment {
        uint32_t inputLocation;
        std::string name;
        VertexAttachmentFormat format;
        uint32_t offset;
    };
	
	typedef std::vector<VertexAttachment> VertexAttachments;
	
	/**
	 * @brief Structure to store the details of a vertex buffer binding.
	 *
	 * Describes all vertex input attachments _one_ buffer contains to create
	 * a vertex buffer binding. NOTE: multiple vertex layouts may contain
	 * various (mutually exclusive) vertex input attachments to form one
	 * complete vertex buffer binding!
	 */
    struct VertexBinding {
        uint32_t bindingLocation;
        uint32_t stride;
		VertexAttachments vertexAttachments;
    };
	
	/**
	 * Creates a vertex binding with given parameters and calculates its strides
	 * depending on its attachments.
	 *
	 * @param[in] bindingLocation Its entry in the buffers that make up the whole vertex buffer.
	 * @param[in] attachments The vertex input attachments this specific buffer layout contains.
	 * @return Vertex binding with calculated stride
	 */
	VertexBinding createVertexBinding(uint32_t bindingLocation, const VertexAttachments &attachments);
	
	typedef std::vector<VertexBinding> VertexBindings;
	
	/**
	 * Creates vertex bindings in a very simplified way with one vertex binding for
	 * each attachment.
	 *
	 * @param[in] attachments The vertex input attachments.
	 * @return Vertex bindings with calculated stride
	 */
	VertexBindings createVertexBindings(const VertexAttachments &attachments);

	/**
	 * @brief Structure to store the details of a vertex layout.
	 *
	 * Describes the complete layout of one vertex, e.g. all of the vertex input
	 * attachments used, and all of the buffer bindings that refer to the attachments
	 * (for when multiple buffers are used).
	 */
    struct VertexLayout {
		VertexBindings vertexBindings;
    };
	
}
