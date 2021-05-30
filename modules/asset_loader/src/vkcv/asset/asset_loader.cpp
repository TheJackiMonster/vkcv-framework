#include "vkcv/asset/asset_loader.hpp"
#include <iostream>
#include <string.h>	// memcpy(3)
#include <stdlib.h>	// calloc(3)
#include <fx/gltf.h>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#include <stb_image.h>

namespace vkcv::asset {

/**
* convert the accessor type from the fx-gltf library to an unsigned int
* @param type
* @return unsigned integer representation
*/
// TODO Return proper error code (we need to define those as macros or enums,
// will discuss during the next core meeting if that should happen on the scope
// of the vkcv framework or just this module)
uint8_t convertTypeToInt(const fx::gltf::Accessor::Type type) {
	switch (type) {
	case fx::gltf::Accessor::Type::None :
		return 0;
	case fx::gltf::Accessor::Type::Scalar :
		return 1;
	case fx::gltf::Accessor::Type::Vec2 :
		return 2;
	case fx::gltf::Accessor::Type::Vec3 :
		return 3;
	case fx::gltf::Accessor::Type::Vec4 :
		return 4;
	default: return 10; // TODO add cases for matrices (or maybe change the type in the struct itself)
	}
}

/**
 * This function unrolls nested exceptions via recursion and prints them
 * @param e error code
 * @param path path to file that is responsible for error
 */
void print_what (const std::exception& e, const std::string &path) {
	fprintf(stderr, "ERROR loading file %s: %s\n", path.c_str(), e.what());
	try {
		std::rethrow_if_nested(e);
	} catch (const std::exception& nested) {
		std::cerr << "nested: ";
		print_what(nested, path);
	}
}

int loadMesh(const std::string &path, Mesh &mesh) {
	fx::gltf::Document object;

	try {
		if (path.rfind(".glb", (path.length()-4)) != std::string::npos) {
			object = fx::gltf::LoadFromBinary(path);
		} else {
			object = fx::gltf::LoadFromText(path);
		}
	} catch (const std::system_error &err) {
		print_what(err, path);
		return 0;
	} catch (const std::exception &e) {
		print_what(e, path);
		return 0;
	}

	// TODO Temporary restriction: Only one mesh per glTF file allowed
	// currently. Later, we want to support whole scenes with more than
	// just meshes.
	if (object.meshes.size() != 1) return 0;

	fx::gltf::Mesh const &objectMesh = object.meshes[0];
	// TODO We want to support more than one vertex group per mesh
	// eventually... right now this is hard-coded to use only the first one
	// because we only care about the example triangle and cube
	fx::gltf::Primitive const &objectPrimitive = objectMesh.primitives[0];
	fx::gltf::Accessor posAccessor;
	
	std::vector<VertexAttribute> vertexAttributes;
	vertexAttributes.reserve(objectPrimitive.attributes.size());
	
	for (auto const & attrib : objectPrimitive.attributes) {
		fx::gltf::Accessor accessor =  object.accessors[attrib.second];
		VertexAttribute attribute;

		if (attrib.first == "POSITION") {
			attribute.type = PrimitiveType::POSITION;
			posAccessor = accessor;
		} else if (attrib.first == "NORMAL") {
			attribute.type = PrimitiveType::NORMAL;
		} else if (attrib.first == "TEXCOORD_0") {
			attribute.type = PrimitiveType::TEXCOORD_0;
		} else {
			return 0;
		}
		
		attribute.offset = object.bufferViews[accessor.bufferView].byteOffset;
		attribute.length = object.bufferViews[accessor.bufferView].byteLength;
		attribute.stride = object.bufferViews[accessor.bufferView].byteStride;
		attribute.componentType = static_cast<uint16_t>(accessor.componentType);
		
		if (convertTypeToInt(accessor.type) != 10) {
			attribute.componentCount = convertTypeToInt(accessor.type);
		} else {
			return 0;
		}
		
		vertexAttributes.push_back(attribute);
	}

	// TODO consider the case where there is no index buffer (not all
	// meshes have to use indexed rendering)
	const fx::gltf::Accessor &indexAccessor = object.accessors[objectPrimitive.indices];
	const fx::gltf::BufferView &indexBufferView = object.bufferViews[indexAccessor.bufferView];
	const fx::gltf::Buffer &indexBuffer = object.buffers[indexBufferView.buffer];
	
	std::vector<uint8_t> indexBufferData;
	indexBufferData.resize(indexBufferView.byteLength);
	{
		const size_t off = indexBufferView.byteOffset;
		const void *const ptr = ((char*)indexBuffer.data.data()) + off;
		if (!memcpy(indexBufferData.data(), ptr, indexBufferView.byteLength)) {
			std::cerr << "ERROR copying index buffer data.\n";
			return 0;
		}
	}

	const fx::gltf::BufferView&	vertexBufferView	= object.bufferViews[posAccessor.bufferView];
	const fx::gltf::Buffer&		vertexBuffer		= object.buffers[vertexBufferView.buffer];
	
	// FIXME: This only works when all vertex attributes are in one buffer
	std::vector<uint8_t> vertexBufferData;
	vertexBufferData.resize(vertexBuffer.byteLength);
	{
		const size_t off = 0;
		const void *const ptr = ((char*)vertexBuffer.data.data()) + off;
		if (!memcpy(vertexBufferData.data(), ptr, vertexBuffer.byteLength)) {
			std::cerr << "ERROR copying vertex buffer data.\n";
			return 0;
		}
	}

	IndexType indexType;
	switch(indexAccessor.componentType) {
	case fx::gltf::Accessor::ComponentType::UnsignedByte:
		indexType = UINT8; break;
	case fx::gltf::Accessor::ComponentType::UnsignedShort:
		indexType = UINT16; break;
	case fx::gltf::Accessor::ComponentType::UnsignedInt:
		indexType = UINT32; break;
	default:
		std::cerr << "ERROR: Index type not supported: " <<
			static_cast<uint16_t>(indexAccessor.componentType) <<
			std::endl;
		return 0;
	}

	const size_t numVertexGroups = objectMesh.primitives.size();
	
	std::vector<VertexGroup> vertexGroups;
	vertexGroups.reserve(numVertexGroups);
	
	vertexGroups.push_back({
		static_cast<PrimitiveMode>(objectPrimitive.mode),
		object.accessors[objectPrimitive.indices].count,
		posAccessor.count,
		{indexType, indexBufferData},
		{vertexBufferData, vertexAttributes},
		{posAccessor.min[0], posAccessor.min[1], posAccessor.min[2]},
		{posAccessor.max[0], posAccessor.max[1], posAccessor.max[2]},
		static_cast<uint8_t>(objectPrimitive.material)
	});
	
	std::vector<Material> materials;

	mesh = {
		object.meshes[0].name,
		vertexGroups,
		materials,
		0, 0, 0, NULL
	};

	// FIXME HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK
	// fail quietly if there is no texture
	if (object.textures.size()) {
		const std::string mime_type("image/jpeg");
		const fx::gltf::Texture &tex = object.textures[0];
		const fx::gltf::Image &img = object.images[tex.source];
#ifndef NDEBUG
		printf("texture name=%s sampler=%u source=%u\n",
				tex.name.c_str(), tex.sampler, tex.source);
		printf("image   name=%s uri=%s mime=%s\n", img.name.c_str(),
				img.uri.c_str(), img.mimeType.c_str());
#endif
		
		size_t pos = path.find_last_of("/");
		auto dir = path.substr(0, pos);
		
		mesh.texture_hack.img = stbi_load((dir + "/" + img.uri).c_str(),
				&mesh.texture_hack.w, &mesh.texture_hack.h,
				&mesh.texture_hack.ch, 4);
	}
	// FIXME HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK
	return 1;
}

}
