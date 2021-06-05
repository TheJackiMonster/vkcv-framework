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
	std::vector<Texture> textures;
	std::vector<Sampler> samplers;

	std::vector<int> vertexGroupsIndex;

	for(int i = 0; i < numVertexGroups; i++){
        vertexGroupsIndex.push_back(i);
	}


	mesh = {
		object.meshes[0].name,
		vertexGroupsIndex,
	};

	// FIXME HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK
	// fail quietly if there is no texture
	textures.reserve(1);
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
		
		std::string img_uri = dir + "/" + img.uri;
		int w, h, c;
		uint8_t *data = stbi_load(img_uri.c_str(), &w, &h, &c, 4);
		c = 4;	// FIXME hardcoded to always have RGBA channel layout
		if (!data) {
			std::cerr << "ERROR loading texture image data.\n";
			return 0;
		}
		const size_t byteLen = w * h * c;

		std::vector<uint8_t> imgdata;
		imgdata.resize(byteLen);
		if (!memcpy(imgdata.data(), data, byteLen)) {
			std::cerr << "ERROR copying texture image data.\n";
			free(data);
			return 0;
		}
		free(data);

		textures.push_back({
			0, static_cast<uint8_t>(c),
			static_cast<uint16_t>(w), static_cast<uint16_t>(h),
			imgdata
		});
	}
	// FIXME HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK HACK
	return 1;
}

int loadScene(const std::string &path, Scene &scene){
    fx::gltf::Document sceneObjects;

    try {
        if (path.rfind(".glb", (path.length()-4)) != std::string::npos) {
            sceneObjects = fx::gltf::LoadFromBinary(path);
        } else {
            sceneObjects = fx::gltf::LoadFromText(path);
        }
    } catch (const std::system_error &err) {
        print_what(err, path);
        return 0;
    } catch (const std::exception &e) {
        print_what(e, path);
        return 0;
    }
    size_t pos = path.find_last_of("/");
    auto dir = path.substr(0, pos);

    // file has to contain at least one mesh
    if (sceneObjects.meshes.size() == 0) return 0;


    fx::gltf::Accessor posAccessor;
    std::vector<VertexAttribute> vertexAttributes;
    std::vector<Material> materials;
    std::vector<Texture> textures;
    std::vector<Sampler> samplers;
    std::vector<Mesh> meshes;
    std::vector<VertexGroup> vertexGroups;
    std::vector<int> vertexGroupsIndex;
    int groupCount = 0;

    Mesh mesh = {};


    for(int i = 0; i < sceneObjects.meshes.size(); i++){
        fx::gltf::Mesh const &objectMesh = sceneObjects.meshes[i];

        for(int j = 0; j < objectMesh.primitives.size(); j++){
            fx::gltf::Primitive const &objectPrimitive = objectMesh.primitives[j];
            vertexAttributes.reserve(objectPrimitive.attributes.size());

            for (auto const & attrib : objectPrimitive.attributes) {

                fx::gltf::Accessor accessor =  sceneObjects.accessors[attrib.second];
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

                attribute.offset = sceneObjects.bufferViews[accessor.bufferView].byteOffset;
                attribute.length = sceneObjects.bufferViews[accessor.bufferView].byteLength;
                attribute.stride = sceneObjects.bufferViews[accessor.bufferView].byteStride;
                // TODO use enums instead of only integer representation for types (defined in VertexLayout.hpp)
                attribute.componentType = static_cast<uint16_t>(accessor.componentType);

                if (convertTypeToInt(accessor.type) != 10) {
                    attribute.componentCount = convertTypeToInt(accessor.type);
                } else {
                    return 0;
                }

                vertexAttributes.push_back(attribute);
            }

            IndexType indexType;
            std::vector<uint8_t> indexBufferData = {};
            if (objectPrimitive.indices >= 0){ // if there is no index buffer, -1 is returned from fx-gltf
                const fx::gltf::Accessor &indexAccessor = sceneObjects.accessors[objectPrimitive.indices];
                const fx::gltf::BufferView &indexBufferView = sceneObjects.bufferViews[indexAccessor.bufferView];
                const fx::gltf::Buffer &indexBuffer = sceneObjects.buffers[indexBufferView.buffer];

                indexBufferData.resize(indexBufferView.byteLength);
                {
                    const size_t off = indexBufferView.byteOffset;
                    const void *const ptr = ((char*)indexBuffer.data.data()) + off;
                    if (!memcpy(indexBufferData.data(), ptr, indexBufferView.byteLength)) {
                        std::cerr << "ERROR copying index buffer data.\n";
                        return 0;
                    }
                }

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
            }

            const fx::gltf::BufferView&	vertexBufferView	= sceneObjects.bufferViews[posAccessor.bufferView];
            const fx::gltf::Buffer&		vertexBuffer		= sceneObjects.buffers[vertexBufferView.buffer];

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

            const size_t numVertexGroups = objectMesh.primitives.size();
            vertexGroups.reserve(numVertexGroups);

            vertexGroups.push_back({
                static_cast<PrimitiveMode>(objectPrimitive.mode),
                sceneObjects.accessors[objectPrimitive.indices].count,
                posAccessor.count,
                {indexType, indexBufferData},
                {vertexBufferData, vertexAttributes},
                {posAccessor.min[0], posAccessor.min[1], posAccessor.min[2]},
                {posAccessor.max[0], posAccessor.max[1], posAccessor.max[2]},
                static_cast<uint8_t>(objectPrimitive.material)
            });

            groupCount++;
            vertexGroupsIndex.push_back(groupCount);
        }

        mesh = {
                sceneObjects.meshes[i].name,
                vertexGroupsIndex,
        };

        meshes.push_back(mesh);
    }

    if (sceneObjects.textures.size() > 0){
        textures.reserve(sceneObjects.textures.size());

        for(int k = 0; k < sceneObjects.textures.size(); k++){
            const fx::gltf::Texture &tex = sceneObjects.textures[k];
            const fx::gltf::Image &img = sceneObjects.images[tex.source];
#ifndef NDEBUG
            printf("texture name=%s sampler=%u source=%u\n",
                   tex.name.c_str(), tex.sampler, tex.source);
            printf("image   name=%s uri=%s mime=%s\n", img.name.c_str(),
                   img.uri.c_str(), img.mimeType.c_str());
#endif
            std::string img_uri = dir + "/" + img.uri;
            int w, h, c;
            uint8_t *data = stbi_load(img_uri.c_str(), &w, &h, &c, 4);
            c = 4;	// FIXME hardcoded to always have RGBA channel layout
            if (!data) {
                std::cerr << "ERROR loading texture image data.\n";
                return 0;
            }
            const size_t byteLen = w * h * c;

            std::vector<uint8_t> imgdata;
            imgdata.resize(byteLen);
            if (!memcpy(imgdata.data(), data, byteLen)) {
                std::cerr << "ERROR copying texture image data.\n";
                free(data);
                return 0;
            }
            free(data);

            textures.push_back({
                0,
                static_cast<uint8_t>(c),
                static_cast<uint16_t>(w),
                static_cast<uint16_t>(h),
                imgdata
            });

        }
    }

    // TODO fill materials struct and vector

    scene = {
            meshes,
            vertexGroups,
            materials,
            textures,
            samplers
    };

    return 1;
}

}
