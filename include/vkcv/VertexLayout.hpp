#pragma once

#include <unordered_map>
#include <vector>
#include <iostream>
#include <string>

namespace vkcv{

	/* With these enums, 0 is reserved to signal uninitialized or invalid data. */
	enum class PrimitiveType : uint32_t {
		UNDEFINED = 0,
		POSITION = 1,
		NORMAL = 2,
		TEXCOORD_0 = 3
	};
	/* This struct describes one vertex attribute of a vertex buffer. */
	typedef struct {
		PrimitiveType type;			// POSITION, NORMAL, ...
		uint32_t offset;			// offset in bytes
		uint32_t length;			// length of ... in bytes
		uint32_t stride;			// stride in bytes
		uint16_t componentType;		// eg. 5126 for float
		uint8_t  componentCount;	// eg. 3 for vec3
	} VertexAttribute;

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

	uint32_t getFormatSize(VertexFormat format);

    struct VertexInputAttachment{
        VertexInputAttachment() = delete;
        VertexInputAttachment(uint32_t location, uint32_t binding, std::string name, VertexFormat format, uint32_t offset) noexcept;

        uint32_t location;
        uint32_t binding;
        std::string name;
        VertexFormat format;
        uint32_t offset;
    };

    struct VertexLayout{
        VertexLayout() noexcept;
        VertexLayout(const std::vector<VertexInputAttachment> &inputs) noexcept;
        std::unordered_map<uint32_t, VertexInputAttachment> attachmentMap;
        uint32_t stride;
    };
}