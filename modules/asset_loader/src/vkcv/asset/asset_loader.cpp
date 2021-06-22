#include "vkcv/asset/asset_loader.hpp"
#include <iostream>
#include <string.h>	// memcpy(3)
#include <stdlib.h>	// calloc(3)
#include <fx/gltf.h>
#include <vulkan/vulkan.hpp>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#include <stb_image.h>
#include <vkcv/Logger.hpp>
#include <algorithm>

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
	vkcv_log(LogLevel::ERROR, "Loading file %s: %s",
			 path.c_str(), e.what());
	
	try {
		std::rethrow_if_nested(e);
	} catch (const std::exception& nested) {
		print_what(nested, path);
	}
}

/** Translate the component type used in the index accessor of fx-gltf to our
 * enum for index type. The reason we have defined an incompatible enum that
 * needs translation is that only a subset of component types is valid for
 * indices and we want to catch these incompatibilities here. */
enum IndexType getIndexType(const enum fx::gltf::Accessor::ComponentType &t)
{
	switch (t) {
	case fx::gltf::Accessor::ComponentType::UnsignedByte:
		return IndexType::UINT8;
	case fx::gltf::Accessor::ComponentType::UnsignedShort:
		return IndexType::UINT16;
	case fx::gltf::Accessor::ComponentType::UnsignedInt:
		return IndexType::UINT32;
	default:
        std::cerr << "ERROR: Index type not supported: " <<
			static_cast<uint16_t>(t) << std::endl;
		return IndexType::UNDEFINED;
	}
}

/**
 * This function computes the modelMatrix out of the data given in the gltf file. It also checks, whether a modelMatrix was given.
 * @param translation possible translation vector (default 0,0,0)
 * @param scale possible scale vector (default 1,1,1)
 * @param rotation possible rotation, given in quaternion (default 0,0,0,1)
 * @param matrix possible modelmatrix (default identity)
 * @return model Matrix as an array of floats
 */
std::array<float, 16> computeModelMatrix(std::array<float, 3> translation, std::array<float, 3> scale, std::array<float, 4> rotation, std::array<float, 16> matrix){
    std::array<float, 16> modelMatrix = {1,0,0,0,
                                         0,1,0,0,
                                         0,0,1,0,
                                         0,0,0,1};
    if (matrix != modelMatrix){
        return matrix;
    } else {
        // translation
        modelMatrix[3] = translation[0];
        modelMatrix[7] = translation[1];
        modelMatrix[11] = translation[2];
        // rotation and scale
        auto a = rotation[0];
        auto q1 = rotation[1];
        auto q2 = rotation[2];
        auto q3 = rotation[3];

        modelMatrix[0] = (2 * (a * a + q1 * q1) - 1) * scale[0];
        modelMatrix[1] = (2 * (q1 * q2 - a * q3)) * scale[1];
        modelMatrix[2] = (2 * (q1 * q3 + a * q2)) * scale[2];

        modelMatrix[4] = (2 * (q1 * q2 + a * q3)) * scale[0];
        modelMatrix[5] = (2 * (a * a + q2 * q2) - 1) * scale[1];
        modelMatrix[6] = (2 * (q2 * q3 - a * q1)) * scale[2];

        modelMatrix[8] = (2 * (q1 * q3 - a * q2)) * scale[0];
        modelMatrix[9] = (2 * (q2 * q3 + a * q1)) * scale[1];
        modelMatrix[10] = (2 * (a * a + q3 * q3) - 1) * scale[2];

        // flip y, because GLTF uses y up, but vulkan -y up
        modelMatrix[5] *= -1;

        return modelMatrix;
    }

}

bool materialHasTexture(const Material *const m, const PBRTextureTarget t)
{
	return m->textureMask & bitflag(t);
}

int translateSamplerMode(const fx::gltf::Sampler::WrappingMode mode)
{
	switch (mode) {
	case fx::gltf::Sampler::WrappingMode::ClampToEdge:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case fx::gltf::Sampler::WrappingMode::MirroredRepeat:
		return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	case fx::gltf::Sampler::WrappingMode::Repeat:
	default:
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}
}

int translateSampler(const fx::gltf::Sampler &src, vkcv::asset::Sampler &dst)
{
	dst.minLOD = 0;
	dst.maxLOD = VK_LOD_CLAMP_NONE;

	switch (src.minFilter) {
	case fx::gltf::Sampler::MinFilter::None:
	case fx::gltf::Sampler::MinFilter::Nearest:
		dst.minFilter = VK_FILTER_NEAREST;
		dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		dst.maxLOD = 0.25;
	case fx::gltf::Sampler::MinFilter::Linear:
		dst.minFilter = VK_FILTER_LINEAR;
		dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		dst.maxLOD = 0.25;
	case fx::gltf::Sampler::MinFilter::NearestMipMapNearest:
		dst.minFilter = VK_FILTER_NEAREST;
		dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	case fx::gltf::Sampler::MinFilter::LinearMipMapNearest:
		dst.minFilter = VK_FILTER_LINEAR;
		dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	case fx::gltf::Sampler::MinFilter::NearestMipMapLinear:
		dst.minFilter = VK_FILTER_NEAREST;
		dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	case fx::gltf::Sampler::MinFilter::LinearMipMapLinear:
		dst.minFilter = VK_FILTER_LINEAR;
		dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}

	switch (src.magFilter) {
	case fx::gltf::Sampler::MagFilter::None:
	case fx::gltf::Sampler::MagFilter::Nearest:
		dst.magFilter = VK_FILTER_NEAREST;
	case fx::gltf::Sampler::MagFilter::Linear:
		dst.magFilter = VK_FILTER_LINEAR;
	}

	dst.addressModeU = translateSamplerMode(src.wrapS);
	dst.addressModeV = translateSamplerMode(src.wrapT);
	// XXX What's a good heuristic for W?
	if (src.wrapS == src.wrapT)
		dst.addressModeW = dst.addressModeU;
	else
		dst.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	return ASSET_SUCCESS;
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
        return ASSET_ERROR;
    } catch (const std::exception &e) {
        print_what(e, path);
        return ASSET_ERROR;
    }
    size_t pos = path.find_last_of("/");
    auto dir = path.substr(0, pos);

    // file has to contain at least one mesh
    if (sceneObjects.meshes.size() == 0) return ASSET_ERROR;

    fx::gltf::Accessor posAccessor;
    std::vector<VertexAttribute> vertexAttributes;
    std::vector<Material> materials;
    std::vector<Texture> textures;
    std::vector<Sampler> samplers;
    std::vector<Mesh> meshes;
    std::vector<VertexGroup> vertexGroups;
    int groupCount = 0;

    Mesh mesh = {};

    for(int i = 0; i < sceneObjects.meshes.size(); i++){
        std::vector<int> vertexGroupsIndices;
        fx::gltf::Mesh const &objectMesh = sceneObjects.meshes[i];

        for(int j = 0; j < objectMesh.primitives.size(); j++){
            fx::gltf::Primitive const &objectPrimitive = objectMesh.primitives[j];
            vertexAttributes.clear();
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
                } else if (attrib.first == "TEXCOORD_1") {
                    attribute.type = PrimitiveType::TEXCOORD_1;
                } else {
                    return ASSET_ERROR;
                }

                attribute.offset = sceneObjects.bufferViews[accessor.bufferView].byteOffset;
                attribute.length = sceneObjects.bufferViews[accessor.bufferView].byteLength;
                attribute.stride = sceneObjects.bufferViews[accessor.bufferView].byteStride;
		        attribute.componentType = static_cast<ComponentType>(accessor.componentType);

                if (convertTypeToInt(accessor.type) != 10) {
                    attribute.componentCount = convertTypeToInt(accessor.type);
                } else {
                    return ASSET_ERROR;
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
                        vkcv_log(LogLevel::ERROR, "Copying index buffer data");
                        return ASSET_ERROR;
                    }
                }

                indexType = getIndexType(indexAccessor.componentType);
                if (indexType == IndexType::UNDEFINED){
                    vkcv_log(LogLevel::ERROR, "Index Type undefined.");
                    return ASSET_ERROR;
                }
            }

            const fx::gltf::BufferView&	vertexBufferView = sceneObjects.bufferViews[posAccessor.bufferView];
            const fx::gltf::Buffer& vertexBuffer = sceneObjects.buffers[vertexBufferView.buffer];

            // only copy relevant part of vertex data
            uint32_t relevantBufferOffset = std::numeric_limits<uint32_t>::max();
            uint32_t relevantBufferEnd = 0;
            for (const auto &attribute : vertexAttributes) {
                relevantBufferOffset = std::min(attribute.offset, relevantBufferOffset);
                const uint32_t attributeEnd = attribute.offset + attribute.length;
                relevantBufferEnd = std::max(relevantBufferEnd, attributeEnd);    // TODO: need to incorporate stride?
            }
            const uint32_t relevantBufferSize = relevantBufferEnd - relevantBufferOffset;

            // FIXME: This only works when all vertex attributes are in one buffer
            std::vector<uint8_t> vertexBufferData;
            vertexBufferData.resize(relevantBufferSize);
            {
                const void *const ptr = ((char*)vertexBuffer.data.data()) + relevantBufferOffset;
                if (!memcpy(vertexBufferData.data(), ptr, relevantBufferSize)) {
                    vkcv_log(LogLevel::ERROR, "Copying vertex buffer data");
                    return ASSET_ERROR;
                }
            }

            // make vertex attributes relative to copied section
            for (auto &attribute : vertexAttributes) {
                attribute.offset -= relevantBufferOffset;
            }

            const size_t numVertexGroups = objectMesh.primitives.size();
            vertexGroups.reserve(numVertexGroups);	// FIXME this is a bug

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

            vertexGroupsIndices.push_back(groupCount);
            groupCount++;
        }

        mesh.name = sceneObjects.meshes[i].name;
        mesh.vertexGroups = vertexGroupsIndices;

        meshes.push_back(mesh);
    }

    for(int m = 0; m < sceneObjects.nodes.size(); m++) {
        meshes[sceneObjects.nodes[m].mesh].modelMatrix = computeModelMatrix(sceneObjects.nodes[m].translation,
                                                                            sceneObjects.nodes[m].scale,
                                                                            sceneObjects.nodes[m].rotation,
                                                                            sceneObjects.nodes[m].matrix);
    }

    if (sceneObjects.textures.size() > 0){
        textures.reserve(sceneObjects.textures.size());

        for(int k = 0; k < sceneObjects.textures.size(); k++){
            const fx::gltf::Texture &tex = sceneObjects.textures[k];
            const fx::gltf::Image &img = sceneObjects.images[tex.source];
	    // TODO Image objects in glTF can have a URI _or_ a bufferView and
	    // a mimeType; but here we are assuming to always find a URI.
            std::string img_uri = dir + "/" + img.uri;
            int w, h, c;
            uint8_t *data = stbi_load(img_uri.c_str(), &w, &h, &c, 4);
            c = 4;	// FIXME hardcoded to always have RGBA channel layout
            if (!data) {
                vkcv_log(LogLevel::ERROR, "Loading texture image data.")
                return ASSET_ERROR;
            }
            const size_t byteLen = w * h * c;

            std::vector<uint8_t> imgdata;
            imgdata.resize(byteLen);
            if (!memcpy(imgdata.data(), data, byteLen)) {
                vkcv_log(LogLevel::ERROR, "Copying texture image data")
                free(data);
                return ASSET_ERROR;
            }
            free(data);

            textures.push_back({
                tex.sampler,
                static_cast<uint8_t>(c),
                static_cast<uint16_t>(w),
                static_cast<uint16_t>(h),
                imgdata
            });

        }
    }

    if (sceneObjects.materials.size() > 0){
        materials.reserve(sceneObjects.materials.size());

        for (int l = 0; l < sceneObjects.materials.size(); l++){
            fx::gltf::Material material = sceneObjects.materials[l];
	    // TODO I think we shouldn't set the index for a texture target if
	    // it isn't defined. So we need to test first if there is a normal
	    // texture before assigning material.normalTexture.index.
	    // About the bitmask: If a normal texture is there, modify the
	    // materials textureMask like this:
	    // 		mat.textureMask |= bitflag(asset::normal);
            materials.push_back({
               0,	// TODO
               material.pbrMetallicRoughness.baseColorTexture.index,
               material.pbrMetallicRoughness.metallicRoughnessTexture.index,
               material.normalTexture.index,
               material.occlusionTexture.index,
               material.emissiveTexture.index,
               {
                   material.pbrMetallicRoughness.baseColorFactor[0],
                   material.pbrMetallicRoughness.baseColorFactor[1],
                   material.pbrMetallicRoughness.baseColorFactor[2],
                   material.pbrMetallicRoughness.baseColorFactor[3]
               },
               material.pbrMetallicRoughness.metallicFactor,
               material.pbrMetallicRoughness.roughnessFactor,
               material.normalTexture.scale,
               material.occlusionTexture.strength,
               {
                   material.emissiveFactor[0],
                   material.emissiveFactor[1],
                   material.emissiveFactor[2]
               }
            });
        }
    }

    samplers.reserve(sceneObjects.samplers.size());
    for (const auto &it : sceneObjects.samplers) {
	    samplers.push_back({});
	    auto &sampler = samplers.back();
	    if (translateSampler(it, sampler) != ASSET_SUCCESS)
		    return ASSET_ERROR;
    }

    scene = {
            meshes,
            vertexGroups,
            materials,
            textures,
            samplers
    };

    return ASSET_SUCCESS;
}

}
