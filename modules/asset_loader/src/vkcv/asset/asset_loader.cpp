#include "vkcv/asset/asset_loader.hpp"
#include <iostream>
#include <string.h>	// memcpy(3)
#include <stdlib.h>	// calloc(3)
#include <vulkan/vulkan.hpp>
#include <fx/gltf.h>
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
uint8_t getCompCount(const fx::gltf::Accessor::Type type) {
	switch (type) {
	case fx::gltf::Accessor::Type::Scalar :
		return 1;
	case fx::gltf::Accessor::Type::Vec2 :
		return 2;
	case fx::gltf::Accessor::Type::Vec3 :
		return 3;
	case fx::gltf::Accessor::Type::Vec4 :
	case fx::gltf::Accessor::Type::Mat2 :
		return 4;
	case fx::gltf::Accessor::Type::Mat3 :
		return 9;
	case fx::gltf::Accessor::Type::Mat4 :
		return 16;
	case fx::gltf::Accessor::Type::None :
	default:
		return ASSET_ERROR;
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
		vkcv_log(LogLevel::ERROR, "Index type not supported: %u", static_cast<uint16_t>(type));
		return IndexType::UNDEFINED;
	}
}

/**
 * This function loads all the textures of a Scene described by the texture-
 * and image- array of an fx::gltf::Document.
 * @param tex_src The array of textures from a fx::gltf::Document
 * @param img_src The array of images from a fx::gltf::Document
 * @param buf_src The Array of buffers from a fx::gltf::Document
 * @param bV_src The Array of bufferViews from a fx::gltf::Document
 * @param dir	  The path of directory in which the glTF file is located
 * @param dst	  The array from the vkcv::Scene to write the textures to
 * @return	  ASSET_ERROR if at least one texture could not be constructed
 * 		  properly, otherwise ASSET_SUCCESS
 */
int createTextures(const std::vector<fx::gltf::Texture>& tex_src,
	const std::vector<fx::gltf::Image>& img_src,
	const std::vector<fx::gltf::Buffer>& buf_src,
	const std::vector<fx::gltf::BufferView>& bV_src,
	const std::string& dir, std::vector<Texture>& dst)
{
	dst.clear();
	dst.reserve(tex_src.size());
	for (const auto& tex : tex_src) {
		std::string uri = dir + "/" + img_src[tex.source].uri;
		int w, h, c;
		uint8_t* data;
		if (!uri.empty()) {
			data = stbi_load(uri.c_str(), &w, &h, &c, 4);
			if (!data) {
				vkcv_log(LogLevel::ERROR, "Failed to load image data from %s",
					uri.c_str());
				return ASSET_ERROR;
			}
		} else {
			//TODO this is untested. Find gltf file without uri to test it!
			const fx::gltf::BufferView bufferView = bV_src[img_src[tex.source].bufferView];
			data = stbi_load_from_memory(
					&buf_src[bufferView.buffer].data[bufferView.byteOffset],
					static_cast<int>(bufferView.byteLength),
					&w, &h, &c, 4
			);
			
			if (!data) {
				vkcv_log(LogLevel::ERROR, "Failed to load image data from Buffer %s",
					buf_src[bufferView.buffer].name.c_str());
				return ASSET_ERROR;
			}
		}
		c = 4;	// FIXME hardcoded to always have RGBA channel layout
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
			tex.sampler,
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
		} else if (attrib.first == "COLOR_0") {
			att.type = PrimitiveType::COLOR_0;
		} else if (attrib.first == "COLOR_1") {
			att.type = PrimitiveType::COLOR_1;
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
		att.componentCount = getCompCount(accessor.type);
		
		if (att.componentCount == ASSET_ERROR) {
			return ASSET_ERROR;
		}
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
  * This function translates a given fx-gltf-sampler-wrapping-mode-enum to its vulkan sampler-adress-mode counterpart.
  * @param mode: wrapping mode of a sampler given as fx-gltf-enum
  * @return int vulkan-enum representing the same wrapping mode
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
 * If the glTF doesn't define samplers, we use the defaults defined by fx-gltf.
 * The following are details about the glTF/OpenGL to Vulkan translation.
 * magFilter (VkFilter?):
 * 	GL_NEAREST -> VK_FILTER_NEAREST
 * 	GL_LINEAR -> VK_FILTER_LINEAR
 * minFilter (VkFilter?):
 * mipmapMode (VkSamplerMipmapMode?):
 * Vulkans minFilter and mipmapMode combined correspond to OpenGLs
 * GL_minFilter_MIPMAP_mipmapMode:
 * 	GL_NEAREST_MIPMAP_NEAREST:
 * 		minFilter=VK_FILTER_NEAREST
 * 		mipmapMode=VK_SAMPLER_MIPMAP_MODE_NEAREST
 * 	GL_LINEAR_MIPMAP_NEAREST:
 * 		minFilter=VK_FILTER_LINEAR
 * 		mipmapMode=VK_SAMPLER_MIPMAP_MODE_NEAREST
 * 	GL_NEAREST_MIPMAP_LINEAR:
 * 		minFilter=VK_FILTER_NEAREST
 * 		mipmapMode=VK_SAMPLER_MIPMAP_MODE_LINEAR
 * 	GL_LINEAR_MIPMAP_LINEAR:
 * 		minFilter=VK_FILTER_LINEAR
 * 		mipmapMode=VK_SAMPLER_MIPMAP_MODE_LINEAR
 * The modes of GL_LINEAR and GL_NEAREST have to be emulated using
 * mipmapMode=VK_SAMPLER_MIPMAP_MODE_NEAREST with specific minLOD and maxLOD:
 * 	GL_LINEAR:
 * 		minFilter=VK_FILTER_LINEAR
 * 		mipmapMode=VK_SAMPLER_MIPMAP_MODE_NEAREST
 * 		minLOD=0, maxLOD=0.25
 * 	GL_NEAREST:
 * 		minFilter=VK_FILTER_NEAREST
 * 		mipmapMode=VK_SAMPLER_MIPMAP_MODE_NEAREST
 * 		minLOD=0, maxLOD=0.25
 * Setting maxLOD=0 causes magnification to always be performed (using
 * the defined magFilter), this may be valid if the min- and magFilter
 * are equal, otherwise it won't be the expected behaviour from OpenGL
 * and glTF; instead using maxLod=0.25 allows the minFilter to be
 * performed while still always rounding to the base level.
 * With other modes, minLOD and maxLOD default to:
 * 	minLOD=0
 * 	maxLOD=VK_LOD_CLAMP_NONE
 * wrapping:
 * gltf has wrapS, wrapT with {clampToEdge, MirroredRepeat, Repeat} while
 * Vulkan has addressModeU, addressModeV, addressModeW with values
 * VK_SAMPLER_ADDRESS_MODE_{REPEAT,MIRRORED_REPEAT,CLAMP_TO_EDGE,
 * 			    CAMP_TO_BORDER,MIRROR_CLAMP_TO_EDGE}
 * Translation from glTF to Vulkan is straight forward for the 3 existing
 * modes, default is repeat, the other modes aren't available.
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
		break;
	case fx::gltf::Sampler::MinFilter::Linear:
		dst.minFilter = VK_FILTER_LINEAR;
		dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		dst.maxLOD = 0.25;
		break;
	case fx::gltf::Sampler::MinFilter::NearestMipMapNearest:
		dst.minFilter = VK_FILTER_NEAREST;
		dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		break;
	case fx::gltf::Sampler::MinFilter::LinearMipMapNearest:
		dst.minFilter = VK_FILTER_LINEAR;
		dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		break;
	case fx::gltf::Sampler::MinFilter::NearestMipMapLinear:
		dst.minFilter = VK_FILTER_NEAREST;
		dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		break;
	case fx::gltf::Sampler::MinFilter::LinearMipMapLinear:
		dst.minFilter = VK_FILTER_LINEAR;
		dst.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		break;
	default:
		break;
	}

	switch (src.magFilter) {
	case fx::gltf::Sampler::MagFilter::None:
	case fx::gltf::Sampler::MagFilter::Nearest:
		dst.magFilter = VK_FILTER_NEAREST;
		break;
	case fx::gltf::Sampler::MagFilter::Linear:
		dst.magFilter = VK_FILTER_LINEAR;
		break;
	default:
		break;
	}

	dst.addressModeU = translateSamplerMode(src.wrapS);
	dst.addressModeV = translateSamplerMode(src.wrapT);
	// There is no information about wrapping for a third axis in glTF and
	// we have to hardcode this value.
	dst.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	return ASSET_SUCCESS;
}

/**
 * TODO document
 */
int createVertexGroups(fx::gltf::Mesh const& objectMesh,
	fx::gltf::Document &sceneObjects, 
	std::vector<VertexGroup> &vertexGroups,
	std::vector<int> &vertexGroupsIndices,
	int &groupCount, bool probe) {

	const size_t numVertexGroups = objectMesh.primitives.size();
	vertexGroups.reserve(numVertexGroups);

	for (const auto & objectPrimitive : objectMesh.primitives) {
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
		for (auto const& attrib : objectPrimitive.attributes) {
			if (attrib.first == "POSITION") {
				posAccessor = sceneObjects.accessors[attrib.second];
				break;
			}
		}

		IndexType indexType;
		std::vector<uint8_t> indexBufferData = {};
		const fx::gltf::Accessor& indexAccessor = sceneObjects.accessors[objectPrimitive.indices];
		if (objectPrimitive.indices >= 0 && !probe) { // if there is no index buffer, -1 is returned from fx-gltf
			const fx::gltf::BufferView& indexBufferView = sceneObjects.bufferViews[indexAccessor.bufferView];
			const fx::gltf::Buffer& indexBuffer = sceneObjects.buffers[indexBufferView.buffer];

			indexBufferData.resize(indexBufferView.byteLength);
			{
				const size_t off = indexBufferView.byteOffset;
				const void* const ptr = ((char*)indexBuffer.data.data()) + off;
				if (!memcpy(indexBufferData.data(), ptr, indexBufferView.byteLength)) {
					vkcv_log(LogLevel::ERROR, "Copying index buffer data");
					return ASSET_ERROR;
				}
			}
		}
		indexType = getIndexType(indexAccessor.componentType);
		if (indexType == IndexType::UNDEFINED) {
			vkcv_log(LogLevel::ERROR, "Index Type undefined or not supported.");
			return ASSET_ERROR;
		}

		const fx::gltf::BufferView& vertexBufferView = sceneObjects.bufferViews[posAccessor.bufferView];
		const fx::gltf::Buffer& vertexBuffer = sceneObjects.buffers[vertexBufferView.buffer];

		// only copy relevant part of vertex data
		uint32_t relevantBufferOffset = std::numeric_limits<uint32_t>::max();
		uint32_t relevantBufferEnd = 0;
		for (const auto& attribute : vertexAttributes) {
			relevantBufferOffset = std::min(attribute.offset, relevantBufferOffset);
			const uint32_t attributeEnd = attribute.offset + attribute.length;
			relevantBufferEnd = std::max(relevantBufferEnd, attributeEnd);
		}
		const uint32_t relevantBufferSize = relevantBufferEnd - relevantBufferOffset;

		// FIXME: This only works when all vertex attributes are in one buffer
		std::vector<uint8_t> vertexBufferData;
		if (!probe) {
			vertexBufferData.resize(relevantBufferSize);
			{
				const void* const ptr = ((char*)vertexBuffer.data.data()) + relevantBufferOffset;
				if (!memcpy(vertexBufferData.data(), ptr, relevantBufferSize)) {
					vkcv_log(LogLevel::ERROR, "Copying vertex buffer data");
					return ASSET_ERROR;
				}
			}

			// make vertex attributes relative to copied section
			for (auto& attribute : vertexAttributes) {
				attribute.offset -= relevantBufferOffset;
			}
		}
		
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
	return ASSET_SUCCESS;
}

/**
 * TODO document
 */
void generateTextureMask(fx::gltf::Material &material, uint16_t &textureMask) {
	if (material.pbrMetallicRoughness.baseColorTexture.index > -1) {
		textureMask |= bitflag(asset::PBRTextureTarget::baseColor);
	}
	if (material.pbrMetallicRoughness.metallicRoughnessTexture.index > -1) {
		textureMask |= bitflag(asset::PBRTextureTarget::metalRough);
	}
	if (material.normalTexture.index > -1) {
		textureMask |= bitflag(asset::PBRTextureTarget::normal);
	}
	if (material.occlusionTexture.index > -1) {
		textureMask |= bitflag(asset::PBRTextureTarget::occlusion);
	}
	if (material.emissiveTexture.index > -1) {
		textureMask |= bitflag(asset::PBRTextureTarget::emissive);
	}
}

/**
 * TODO document
 */
int createMaterial(fx::gltf::Document &sceneObjects, std::vector<Material> &materials) {
	if (!sceneObjects.materials.empty()) {
		materials.reserve(sceneObjects.materials.size());

		for (auto material : sceneObjects.materials) {
				uint16_t textureMask = 0;
			generateTextureMask(material, textureMask);
			materials.push_back({
			textureMask,	
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
	} else {
		return ASSET_ERROR;
	}
	return ASSET_SUCCESS;
}

int loadScene(const std::filesystem::path &path, Scene &scene){
    fx::gltf::Document sceneObjects;

	try {
		if ( path.extension() == ".glb") {
			sceneObjects = fx::gltf::LoadFromBinary(path.string());
		}
		else {
			sceneObjects = fx::gltf::LoadFromText(path.string());
		}
	} catch (const std::system_error& err) {
		recurseExceptionPrint(err, path.string());
		return ASSET_ERROR;
	} catch (const std::exception& e) {
		recurseExceptionPrint(e, path.string());
		return ASSET_ERROR;
	}
	auto dir = path.parent_path().string();

    // file has to contain at least one mesh
    if (sceneObjects.meshes.empty()) {
    	return ASSET_ERROR;
    }

    std::vector<Material> materials;
    std::vector<Texture> textures;
    std::vector<Sampler> samplers;
    std::vector<Mesh> meshes;
    std::vector<VertexGroup> vertexGroups;
	int groupCount = 0;

    for (size_t i = 0; i < sceneObjects.meshes.size(); i++){
        std::vector<int> vertexGroupsIndices;
        fx::gltf::Mesh const &objectMesh = sceneObjects.meshes[i];

		if (createVertexGroups(objectMesh, sceneObjects, vertexGroups, vertexGroupsIndices, groupCount, false) != ASSET_SUCCESS) {
			vkcv_log(LogLevel::ERROR, "Failed to get Vertex Groups!");
			return ASSET_ERROR;
		}

		Mesh mesh = {};
        mesh.name = sceneObjects.meshes[i].name;
        mesh.vertexGroups = vertexGroupsIndices;

        meshes.push_back(mesh);
    }

    // This only works if the node has a mesh and it only loads the meshes and ignores cameras and lights
    for (auto & node : sceneObjects.nodes) {
		if (node.mesh > -1) {
			meshes[node.mesh].modelMatrix = computeModelMatrix(
					node.translation,
					node.scale,
					node.rotation,
					node.matrix
			);
		}
	}

    if (createTextures(sceneObjects.textures, sceneObjects.images,sceneObjects.buffers,sceneObjects.bufferViews, dir, textures) != ASSET_SUCCESS) {
	    size_t missing = sceneObjects.textures.size() - textures.size();
	    vkcv_log(LogLevel::ERROR, "Failed to get %lu textures from glTF source '%s'",
			    missing, path.c_str());
    }


	if (createMaterial(sceneObjects, materials) != ASSET_SUCCESS) {
		vkcv_log(LogLevel::ERROR, "Failed to get Materials!");
		return ASSET_ERROR;
	}

    samplers.reserve(sceneObjects.samplers.size());
    for (const auto &it : sceneObjects.samplers) {
	    samplers.push_back({});
	    auto &sampler = samplers.back();
	    if (translateSampler(it, sampler) != ASSET_SUCCESS) {
			return ASSET_ERROR;
		}
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

TextureData loadTexture(const std::filesystem::path& path) {
    TextureData texture;
    
    uint8_t* data = stbi_load(path.string().c_str(), &texture.width, &texture.height, &texture.componentCount, 4);
    
    if (!data) {
		vkcv_log(LogLevel::ERROR, "Texture could not be loaded from '%s'", path.c_str());
    	
    	texture.width = 0;
    	texture.height = 0;
    	texture.componentCount = 0;
    	return texture;
    }
    
    texture.data.resize(texture.width * texture.height * 4);
    memcpy(texture.data.data(), data, texture.data.size());
    return texture;
}

int probeScene(const std::filesystem::path& path, Scene& scene) {
	fx::gltf::Document sceneObjects;

	try {
		if (path.extension() == ".glb") {
			sceneObjects = fx::gltf::LoadFromBinary(path.string());
		}
		else {
			sceneObjects = fx::gltf::LoadFromText(path.string());
		}
	}
	catch (const std::system_error& err) {
		recurseExceptionPrint(err, path.string());
		return ASSET_ERROR;
	}
	catch (const std::exception& e) {
		recurseExceptionPrint(e, path.string());
		return ASSET_ERROR;
	}
	auto dir = path.parent_path().string();

	// file has to contain at least one mesh
	if (sceneObjects.meshes.empty()) {
		return ASSET_ERROR;
	}

	std::vector<Material> materials;
	std::vector<Texture> textures;
	std::vector<Sampler> samplers;
	std::vector<Mesh> meshes;
	std::vector<VertexGroup> vertexGroups;
	int groupCount = 0;

	for (size_t i = 0; i < sceneObjects.meshes.size(); i++) {
		std::vector<int> vertexGroupsIndices;
		fx::gltf::Mesh const& objectMesh = sceneObjects.meshes[i];

		if (createVertexGroups(objectMesh, sceneObjects, vertexGroups, vertexGroupsIndices, groupCount, true) != ASSET_SUCCESS) {
			vkcv_log(LogLevel::ERROR, "Failed to get Vertex Groups!");
			return ASSET_ERROR;
		}

		Mesh mesh = {};
		mesh.name = sceneObjects.meshes[i].name;
		mesh.vertexGroups = vertexGroupsIndices;

		meshes.push_back(mesh);
	}

	// This only works if the node has a mesh and it only loads the meshes and ignores cameras and lights
	for (auto& node : sceneObjects.nodes) {
		if (node.mesh > -1) {
			meshes[node.mesh].modelMatrix = computeModelMatrix(
				node.translation,
				node.scale,
				node.rotation,
				node.matrix
			);
		}
	}

	/*if (createTextures(sceneObjects.textures, sceneObjects.images, sceneObjects.buffers, sceneObjects.bufferViews, dir, textures) != ASSET_SUCCESS) {
		size_t missing = sceneObjects.textures.size() - textures.size();
		vkcv_log(LogLevel::ERROR, "Failed to get %lu textures from glTF source '%s'",
			missing, path.c_str());
	}*/


	if (createMaterial(sceneObjects, materials) != ASSET_SUCCESS) {
		vkcv_log(LogLevel::ERROR, "Failed to get Materials!");
		return ASSET_ERROR;
	}

	/*samplers.reserve(sceneObjects.samplers.size());
	for (const auto& it : sceneObjects.samplers) {
		samplers.push_back({});
		auto& sampler = samplers.back();
		if (translateSampler(it, sampler) != ASSET_SUCCESS) {
			return ASSET_ERROR;
		}
	}*/

	scene = {
			meshes,
			vertexGroups,
			materials,
			textures,
			samplers
	};

	return ASSET_SUCCESS;
}


int loadMesh(const std::filesystem::path &path, Scene &scene, const std::string &name) {
	fx::gltf::Document sceneObjects;

	try {
		if (path.extension() == ".glb") {
			sceneObjects = fx::gltf::LoadFromBinary(path.string());
		}
		else {
			sceneObjects = fx::gltf::LoadFromText(path.string());
		}
	}
	catch (const std::system_error& err) {
		recurseExceptionPrint(err, path.string());
		return ASSET_ERROR;
	}
	catch (const std::exception& e) {
		recurseExceptionPrint(e, path.string());
		return ASSET_ERROR;
	}
	auto dir = path.parent_path().string();

	// file has to contain at least one mesh
	if (sceneObjects.meshes.empty()) {
		return ASSET_ERROR;
	}

	std::vector<Material> materials;
	std::vector<Texture> textures;
	std::vector<Sampler> samplers;
	std::vector<Mesh> meshes;
	std::vector<VertexGroup> vertexGroups;
	int groupCount = 0;
	int meshIndex = -1;

	for (size_t i = 0; i < sceneObjects.meshes.size(); i++) {
		if (sceneObjects.meshes[i].name == name) {
			std::vector<int> vertexGroupsIndices;
			fx::gltf::Mesh const& objectMesh = sceneObjects.meshes[i];

			if (createVertexGroups(objectMesh, sceneObjects, vertexGroups, vertexGroupsIndices, groupCount, false) != ASSET_SUCCESS) {
				vkcv_log(LogLevel::ERROR, "Failed to get Vertex Groups!");
				return ASSET_ERROR;
			}

			Mesh mesh = {};
			mesh.name = sceneObjects.meshes[i].name;
			mesh.vertexGroups = vertexGroupsIndices;

			meshes.push_back(mesh);
			meshIndex = i;
			break;
		}
	}

	if (meshes.empty()) {
		vkcv_log(LogLevel::ERROR, "No mesh by that name!");
		return ASSET_ERROR;
	}

	// This only works if the node has a mesh and it only loads the meshes and ignores cameras and lights
	if (sceneObjects.nodes[meshIndex].mesh > -1) {
		meshes[sceneObjects.nodes[meshIndex].mesh].modelMatrix = computeModelMatrix(
			sceneObjects.nodes[meshIndex].translation,
			sceneObjects.nodes[meshIndex].scale,
			sceneObjects.nodes[meshIndex].rotation,
			sceneObjects.nodes[meshIndex].matrix
		);
	}

	if (createTextures(sceneObjects.textures, sceneObjects.images, sceneObjects.buffers, sceneObjects.bufferViews, dir, textures) != ASSET_SUCCESS) {
		size_t missing = sceneObjects.textures.size() - textures.size();
		vkcv_log(LogLevel::ERROR, "Failed to get %lu textures from glTF source '%s'",
			missing, path.c_str());
	}


	if (createMaterial(sceneObjects, materials) != ASSET_SUCCESS) {
		vkcv_log(LogLevel::ERROR, "Failed to get Materials!");
		return ASSET_ERROR;
	}

	samplers.reserve(sceneObjects.samplers.size());
	for (const auto& it : sceneObjects.samplers) {
		samplers.push_back({});
		auto& sampler = samplers.back();
		if (translateSampler(it, sampler) != ASSET_SUCCESS) {
			return ASSET_ERROR;
		}
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
