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
 * This function unrolls nested exceptions via recursion and prints them
 * @param e	The exception being thrown
 * @param path	The path to the file that was responsible for the exception
 */
void recurseExceptionPrint(const std::exception& e, const std::string &path)
{
	vkcv_log(LogLevel::ERROR, "Loading file %s: %s", path.c_str(), e.what());
	try {
		std::rethrow_if_nested(e);
	} catch (const std::exception& nested) {
		recurseExceptionPrint(nested, path);
	}
}

/**
 * Computes the component count for an accessor type of the fx-gltf library.
 * @param type	The accessor type
 * @return	An unsigned integer count
 */
// TODO add cases for matrices (or maybe change the type in the struct itself)
uint8_t getCompCount(const fx::gltf::Accessor::Type type) {
	switch (type) {
	case fx::gltf::Accessor::Type::Scalar :
		return 1;
	case fx::gltf::Accessor::Type::Vec2 :
		return 2;
	case fx::gltf::Accessor::Type::Vec3 :
		return 3;
	case fx::gltf::Accessor::Type::Vec4 :
		return 4;
	case fx::gltf::Accessor::Type::None :
	default: return ASSET_ERROR;
	}
}

/**
 * Translate the component type used in the index accessor of fx-gltf to our
 * enum for index type. The reason we have defined an incompatible enum that
 * needs translation is that only a subset of component types is valid for
 * indices and we want to catch these incompatibilities here.
 * @param t	The component type
 * @return 	The vkcv::IndexType enum representation
 */
enum IndexType getIndexType(const enum fx::gltf::Accessor::ComponentType &type)
{
	switch (type) {
	case fx::gltf::Accessor::ComponentType::UnsignedByte:
		return IndexType::UINT8;
	case fx::gltf::Accessor::ComponentType::UnsignedShort:
		return IndexType::UINT16;
	case fx::gltf::Accessor::ComponentType::UnsignedInt:
		return IndexType::UINT32;
	default:
		return IndexType::UNDEFINED;
	}
}

/**
 * This function loads all the textures of a Scene described by the texture-
 * and image- array of an fx::gltf::Document.
 * @param tex_src The array of textures from a fx::gltf::Document
 * @param img_src The array of images from a fx::gltf::Document
 * @param dir	  The path of directory in which the glTF file is located
 * @param dst	  The array from the vkcv::Scene to write the textures to
 * @return	  ASSET_ERROR if at least one texture could not be constructed
 * 		  properly, otherwise ASSET_SUCCESS
 */
int createTextures(const std::vector<fx::gltf::Texture> &tex_src,
		const std::vector<fx::gltf::Image> &img_src,
		const std::string &dir, std::vector<Texture> &dst)
{
	dst.clear();
	dst.reserve(tex_src.size());
	for (int i = 0; i < tex_src.size(); i++) {
		// TODO Image objects in glTF can have
		// 1) a URI
		// 2) a bufferView and a mimeType
		// to describe where/how to load the data, but here we are
		// always assuming a URI. In order to load files where images
		// have no URI, we need to handle the second case as well here.
		std::string uri = dir + "/" + img_src[tex_src[i].source].uri;

		int w, h, c;
		uint8_t *data = stbi_load(uri.c_str(), &w, &h, &c, 4);
		c = 4;	// FIXME hardcoded to always have RGBA channel layout
		if (!data) {
			vkcv_log(LogLevel::ERROR, "Failed to load image data from %s",
					uri.c_str());
			return ASSET_ERROR;
		}

		const size_t nbytes = w * h * c;
		std::vector<uint8_t> imgdata;
		imgdata.resize(nbytes);
		if (!memcpy(imgdata.data(), data, nbytes)) {
			vkcv_log(LogLevel::ERROR, "Failed to copy texture data");
			free(data);
			return ASSET_ERROR;
		}
		free(data);

		dst.push_back({
			tex_src[i].sampler,
			static_cast<uint8_t>(c),
			static_cast<uint16_t>(w), static_cast<uint16_t>(h),
			imgdata
		});
	}
	return ASSET_SUCCESS;
}

/**
 * This function fills the array of vertex attributes of a VertexGroup (usually
 * part of a vkcv::asset::Mesh) object based on the description of attributes
 * for a fx::gltf::Primitive.
 * @param src	The description of attribute objects from the fx-gltf library
 * @param gltf	The main glTF document
 * @param dst	The array of vertex attributes stored in an asset::Mesh object
 * @return	ASSET_ERROR when at least one VertexAttribute could not be
 * 		constructed properly, otherwise ASSET_SUCCESS
 */
int createVertexAttributes(const fx::gltf::Attributes &src,
		const std::vector<fx::gltf::Accessor> &accessors,
		const std::vector<fx::gltf::BufferView> &bufferViews,
		std::vector<VertexAttribute> &dst)
{
	dst.clear();
	dst.reserve(src.size());
	for (const auto &attrib : src) {
		const fx::gltf::Accessor &accessor = accessors[attrib.second];

		dst.push_back({});
		VertexAttribute &att = dst.back();
		if (attrib.first == "POSITION") {
			att.type = PrimitiveType::POSITION;
		} else if (attrib.first == "NORMAL") {
			att.type = PrimitiveType::NORMAL;
		} else if (attrib.first == "TANGENT") {
			att.type = PrimitiveType::TANGENT;
		} else if (attrib.first == "TEXCOORD_0") {
			att.type = PrimitiveType::TEXCOORD_0;
		} else if (attrib.first == "TEXCOORD_1") {
			att.type = PrimitiveType::TEXCOORD_1;
		} else if (attrib.first == "COLOR0") {
			att.type = PrimitiveType::COLOR_0;
		} else if (attrib.first == "JOINTS_0") {
			att.type = PrimitiveType::JOINTS_0;
		} else if (attrib.first == "WEIGHTS_0") {
			att.type = PrimitiveType::WEIGHTS_0;
		} else {
			att.type = PrimitiveType::UNDEFINED;
			return ASSET_ERROR;
		}
		const fx::gltf::BufferView &buf = bufferViews[accessor.bufferView];
		att.offset = buf.byteOffset;
		att.length = buf.byteLength;
		att.stride = buf.byteStride;
		att.componentType = static_cast<ComponentType>(accessor.componentType);
		if ((att.componentCount = getCompCount(accessor.type) == ASSET_ERROR))
			return ASSET_ERROR;
	}
	return ASSET_SUCCESS;
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

/* TODO Should probably be a member function of vkcv::asset::Material */
bool materialHasTexture(const Material *const m, const PBRTextureTarget t)
{
	return m->textureMask & bitflag(t);
}

/**
 * TODO document
 */
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

/**
 * TODO document
 * Most of the documenting comment above the struct {...} Sampler definition in
 * the header could be moved here.
 */
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
	// TODO There is no information about wrapping for a third axis in
	// glTF...  what's a good heuristic we can use to set addressModeW?
	// The following hardocded solution is only a temporary hack.
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
        recurseExceptionPrint(err, path);
        return ASSET_ERROR;
    } catch (const std::exception &e) {
        recurseExceptionPrint(e, path);
        return ASSET_ERROR;
    }
    // TODO use std::filesystem::path instead of std::string for path/uri.
    // Using simple strings and assuming the path separator symbol to be "/" is
    // not safe across different operating systems.
    size_t pos = path.find_last_of("/");
    auto dir = path.substr(0, pos);

    // file has to contain at least one mesh
    if (sceneObjects.meshes.size() == 0) return ASSET_ERROR;

    std::vector<Material> materials;
    std::vector<Texture> textures;
    std::vector<Sampler> samplers;
    std::vector<Mesh> meshes;
    std::vector<VertexGroup> vertexGroups;
    int groupCount = 0;

    for(int i = 0; i < sceneObjects.meshes.size(); i++){
        std::vector<int> vertexGroupsIndices;
        fx::gltf::Mesh const &objectMesh = sceneObjects.meshes[i];

	// TODO This loop should be moved to its own function just like the
	// code for loading textures and getting vertex attributes.
        for(int j = 0; j < objectMesh.primitives.size(); j++){
            fx::gltf::Primitive const &objectPrimitive = objectMesh.primitives[j];
	    std::vector<VertexAttribute> vertexAttributes;
            vertexAttributes.reserve(objectPrimitive.attributes.size());

	    if (createVertexAttributes(objectPrimitive.attributes,
				    sceneObjects.accessors,
				    sceneObjects.bufferViews,
				    vertexAttributes)
			    != ASSET_SUCCESS) {
		    vkcv_log(LogLevel::ERROR, "Failed to get vertex attributes");
		    return ASSET_ERROR;
	    }

	    // The accessor for the position attribute is used for
	    // 1) getting the vertex buffer view which is only needed to get
	    //    the vertex buffer
	    // 2) getting the vertex count for the VertexGroup
	    // 3) getting the min/max of the bounding box for the VertexGroup
	    fx::gltf::Accessor posAccessor;
	    for (auto const & attrib : objectPrimitive.attributes) {
		    if (attrib.first == "POSITION") {
			    posAccessor = sceneObjects.accessors[attrib.second];
			    break;
		    }
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
                    vkcv_log(LogLevel::ERROR, "Index Type undefined or not supported.");
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

	    // FIXME This does the same in each iteration of the loop and
	    // should be moved outside the loop
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

            vertexGroupsIndices.push_back(groupCount);
            groupCount++;
        }

	Mesh mesh = {};
        mesh.name = sceneObjects.meshes[i].name;
        mesh.vertexGroups = vertexGroupsIndices;

        meshes.push_back(mesh);
    }

    // TODO Are we sure that every node has a mesh defined? What if there is a
    // node with just a Camera? Will it have a default value for the mesh index
    // like 0? In that case we'd overwrite the model matrix of the mesh at
    // Scene.meshes[0].modelMatrix...
    // I'm not 100% certain, but this looks wrong to me, please double-check!
    for(int m = 0; m < sceneObjects.nodes.size(); m++) {
        meshes[sceneObjects.nodes[m].mesh].modelMatrix = computeModelMatrix(sceneObjects.nodes[m].translation,
                                                                            sceneObjects.nodes[m].scale,
                                                                            sceneObjects.nodes[m].rotation,
                                                                            sceneObjects.nodes[m].matrix);
    }

    if (createTextures(sceneObjects.textures, sceneObjects.images, dir, textures) != ASSET_SUCCESS) {
	    size_t missing = sceneObjects.textures.size() - textures.size();
	    vkcv_log(LogLevel::ERROR, "Failed to get %lu textures from glTF source '%s'",
			    missing, path.c_str());
    }

    // TODO creating the materials should be moved to its own function just
    // like with textures, vertex groups and vertex attributes before
    if (sceneObjects.materials.size() > 0){
        materials.reserve(sceneObjects.materials.size());

        for (int l = 0; l < sceneObjects.materials.size(); l++){
            fx::gltf::Material material = sceneObjects.materials[l];
	    // TODO When constructing the the vkcv::asset::Material  we need to
	    // test what kind of texture targets it has and then define the
	    // textureMask for it.
	    // Also, what does the fx::gltf::Material do with indices of
	    // texture targets that don't exist? Maybe we shouldn't set the
	    // index for eb. the normalTexture if there is no normal texture...
	    // It may be a good idea to create an extra function for creating a
	    // material and adding it to the materials array instead of trying
	    // to fit it all into one push_back({...}) call.
	    //
	    // Example for using the bitmask: If a normal texture is there,
	    // modify the materials textureMask like this:
	    //     mat.textureMask |= bitflag(asset::PBRTextureTarget::normal);
            materials.push_back({
               0,	// TODO set this mask
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
