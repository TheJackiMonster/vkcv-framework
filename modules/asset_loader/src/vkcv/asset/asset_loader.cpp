#include "vkcv/asset/asset_loader.hpp"
#include <iostream>
#include <string.h>	// memcpy(3)
#include <set>
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
	static void recurseExceptionPrint(const std::exception& e, const std::string &path) {
		vkcv_log(LogLevel::ERROR, "Loading file %s: %s", path.c_str(), e.what());
		
		try {
			std::rethrow_if_nested(e);
		} catch (const std::exception& nested) {
			recurseExceptionPrint(nested, path);
		}
	}

	/**
	 * Returns the component count for an accessor type of the fx-gltf library.
	 * @param type	The accessor type
	 * @return	An unsigned integer count
	 */
	static uint8_t getComponentCount(const fx::gltf::Accessor::Type type) {
		switch (type) {
		case fx::gltf::Accessor::Type::Scalar:
			return 1;
		case fx::gltf::Accessor::Type::Vec2:
			return 2;
		case fx::gltf::Accessor::Type::Vec3:
			return 3;
		case fx::gltf::Accessor::Type::Vec4:
		case fx::gltf::Accessor::Type::Mat2:
			return 4;
		case fx::gltf::Accessor::Type::Mat3:
			return 9;
		case fx::gltf::Accessor::Type::Mat4:
			return 16;
		case fx::gltf::Accessor::Type::None:
		default:
			return 0;
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
	enum IndexType getIndexType(const enum fx::gltf::Accessor::ComponentType &type) {
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
	 * This function fills the array of vertex attributes of a VertexGroup (usually
	 * part of a vkcv::asset::Mesh) object based on the description of attributes
	 * for a fx::gltf::Primitive.
	 *
	 * @param src	The description of attribute objects from the fx-gltf library
	 * @param gltf	The main glTF document
	 * @param dst	The array of vertex attributes stored in an asset::Mesh object
	 * @return	ASSET_ERROR when at least one VertexAttribute could not be
	 * 		constructed properly, otherwise ASSET_SUCCESS
	 */
	static int loadVertexAttributes(const fx::gltf::Attributes &src,
									const std::vector<fx::gltf::Accessor> &accessors,
									const std::vector<fx::gltf::BufferView> &bufferViews,
									std::vector<VertexAttribute> &dst) {
		for (const auto &attrib : src) {
			VertexAttribute att;
			
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
			}
			
			if (att.type != PrimitiveType::UNDEFINED) {
				const fx::gltf::Accessor &accessor = accessors[attrib.second];
				const fx::gltf::BufferView &buf = bufferViews[accessor.bufferView];
				
				att.offset = buf.byteOffset;
				att.length = buf.byteLength;
				att.stride = buf.byteStride;
				att.componentType = static_cast<ComponentType>(accessor.componentType);
				att.componentCount = getComponentCount(accessor.type);
			}
			
			if ((att.type == PrimitiveType::UNDEFINED) ||
				(att.componentCount == 0)) {
				return ASSET_ERROR;
			}
			
			dst.push_back(att);
		}
		
		return ASSET_SUCCESS;
	}

	/**
	 * This function calculates the modelMatrix out of the data given in the gltf file.
	 * It also checks, whether a modelMatrix was given.
	 *
	 * @param translation possible translation vector (default 0,0,0)
	 * @param scale possible scale vector (default 1,1,1)
	 * @param rotation possible rotation, given in quaternion (default 0,0,0,1)
	 * @param matrix possible modelmatrix (default identity)
	 * @return model Matrix as an array of floats
	 */
	static std::array<float, 16> calculateModelMatrix(const std::array<float, 3>& translation,
													  const std::array<float, 3>& scale,
													  const std::array<float, 4>& rotation,
													  const std::array<float, 16>& matrix){
		std::array<float, 16> modelMatrix = {
				1,0,0,0,
				0,1,0,0,
				0,0,1,0,
				0,0,0,1
		};
		
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

	bool Material::hasTexture(const PBRTextureTarget target) const {
		return textureMask & bitflag(target);
	}

	/**
	 * This function translates a given fx-gltf-sampler-wrapping-mode-enum to its vulkan sampler-adress-mode counterpart.
	 * @param mode: wrapping mode of a sampler given as fx-gltf-enum
	 * @return int vulkan-enum representing the same wrapping mode
	 */
	static int translateSamplerMode(const fx::gltf::Sampler::WrappingMode mode) {
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
	static vkcv::asset::Sampler loadSampler(const fx::gltf::Sampler &src) {
		Sampler dst;
		
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
		
		return dst;
	}

	/**
	 * TODO document
	 */
	static int loadVertexGroups(const fx::gltf::Mesh &objectMesh,
								const fx::gltf::Document &sceneObjects,
								Scene &scene, Mesh &mesh) {
		mesh.vertexGroups.reserve(objectMesh.primitives.size());
	
		for (const auto & objectPrimitive : objectMesh.primitives) {
			VertexGroup vertexGroup;
			
			vertexGroup.vertexBuffer.attributes.reserve(
					objectPrimitive.attributes.size()
			);
	
			if (ASSET_SUCCESS != loadVertexAttributes(
					objectPrimitive.attributes,
					sceneObjects.accessors,
					sceneObjects.bufferViews,
					vertexGroup.vertexBuffer.attributes)) {
				vkcv_log(LogLevel::ERROR, "Failed to get vertex attributes of '%s'",
						 mesh.name.c_str());
				return ASSET_ERROR;
			}
			
			// The accessor for the position attribute is used for
			// 1) getting the vertex buffer view which is only needed to get
			//    the vertex buffer
			// 2) getting the vertex count for the VertexGroup
			// 3) getting the min/max of the bounding box for the VertexGroup
			fx::gltf::Accessor posAccessor;
			bool noPosition = true;
			
			for (auto const& attrib : objectPrimitive.attributes) {
				if (attrib.first == "POSITION") {
					posAccessor = sceneObjects.accessors[attrib.second];
					noPosition = false;
					break;
				}
			}
			
			if (noPosition) {
				vkcv_log(LogLevel::ERROR, "Position attribute not found from '%s'",
						 mesh.name.c_str());
				return ASSET_ERROR;
			}
	
			const fx::gltf::Accessor& indexAccessor = sceneObjects.accessors[objectPrimitive.indices];
			
			int indexBufferURI;
			if (objectPrimitive.indices >= 0) { // if there is no index buffer, -1 is returned from fx-gltf
				const fx::gltf::BufferView& indexBufferView = sceneObjects.bufferViews[indexAccessor.bufferView];
				const fx::gltf::Buffer& indexBuffer = sceneObjects.buffers[indexBufferView.buffer];
				
				// Because the buffers are already preloaded into the memory by the gltf-library,
				// it makes no sense to later them later on manually again into memory.
				vertexGroup.indexBuffer.data.resize(indexBufferView.byteLength);
				memcpy(vertexGroup.indexBuffer.data.data(),
					   indexBuffer.data.data() + indexBufferView.byteOffset,
					   indexBufferView.byteLength);
			} else {
				indexBufferURI = -1;
			}
			
			vertexGroup.indexBuffer.type = getIndexType(indexAccessor.componentType);
			
			if (IndexType::UNDEFINED == vertexGroup.indexBuffer.type) {
				vkcv_log(LogLevel::ERROR, "Index Type undefined or not supported.");
				return ASSET_ERROR;
			}
	
			const fx::gltf::BufferView& vertexBufferView = sceneObjects.bufferViews[posAccessor.bufferView];
			const fx::gltf::Buffer& vertexBuffer = sceneObjects.buffers[vertexBufferView.buffer];
			
			// only copy relevant part of vertex data
			uint32_t relevantBufferOffset = std::numeric_limits<uint32_t>::max();
			uint32_t relevantBufferEnd = 0;
			
			for (const auto& attribute : vertexGroup.vertexBuffer.attributes) {
				relevantBufferOffset = std::min(attribute.offset, relevantBufferOffset);
				relevantBufferEnd = std::max(relevantBufferEnd, attribute.offset + attribute.length);
			}
			
			const uint32_t relevantBufferSize = relevantBufferEnd - relevantBufferOffset;
			
			vertexGroup.vertexBuffer.data.resize(relevantBufferSize);
			memcpy(vertexGroup.vertexBuffer.data.data(),
				   vertexBuffer.data.data() + relevantBufferOffset,
				   relevantBufferSize);
			
			// make vertex attributes relative to copied section
			for (auto& attribute : vertexGroup.vertexBuffer.attributes) {
				attribute.offset -= relevantBufferOffset;
			}
			
			vertexGroup.mode = static_cast<PrimitiveMode>(objectPrimitive.mode);
			vertexGroup.numIndices = sceneObjects.accessors[objectPrimitive.indices].count;
			vertexGroup.numVertices = posAccessor.count;
			
			memcpy(&(vertexGroup.min), posAccessor.min.data(), sizeof(vertexGroup.min));
			memcpy(&(vertexGroup.max), posAccessor.max.data(), sizeof(vertexGroup.max));
			
			vertexGroup.materialIndex = static_cast<uint8_t>(objectPrimitive.material);
			
			mesh.vertexGroups.push_back(static_cast<int>(scene.vertexGroups.size()));
			scene.vertexGroups.push_back(vertexGroup);
		}
		
		return ASSET_SUCCESS;
	}

	/**
	 * TODO document
	 */
	static uint16_t generateTextureMask(fx::gltf::Material &material) {
		uint16_t textureMask = 0;
		
		if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
			textureMask |= bitflag(asset::PBRTextureTarget::baseColor);
		}
		if (material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) {
			textureMask |= bitflag(asset::PBRTextureTarget::metalRough);
		}
		if (material.normalTexture.index >= 0) {
			textureMask |= bitflag(asset::PBRTextureTarget::normal);
		}
		if (material.occlusionTexture.index >= 0) {
			textureMask |= bitflag(asset::PBRTextureTarget::occlusion);
		}
		if (material.emissiveTexture.index >= 0) {
			textureMask |= bitflag(asset::PBRTextureTarget::emissive);
		}
		
		return textureMask;
	}

	int probeScene(const std::filesystem::path& path, Scene& scene) {
		fx::gltf::Document sceneObjects;
	
		try {
			if (path.extension() == ".glb") {
				sceneObjects = fx::gltf::LoadFromBinary(path.string());
			} else {
				sceneObjects = fx::gltf::LoadFromText(path.string());
			}
		} catch (const std::system_error& err) {
			recurseExceptionPrint(err, path.string());
			return ASSET_ERROR;
		} catch (const std::exception& e) {
			recurseExceptionPrint(e, path.string());
			return ASSET_ERROR;
		}
		
		const auto directory = path.parent_path();
		
		scene.meshes.clear();
		scene.vertexGroups.clear();
		scene.materials.clear();
		scene.textures.clear();
		scene.samplers.clear();
	
		// file has to contain at least one mesh
		if (sceneObjects.meshes.empty()) {
			vkcv_log(LogLevel::ERROR, "No meshes found! (%s)", path.c_str());
			return ASSET_ERROR;
		} else {
			scene.meshes.reserve(sceneObjects.meshes.size());
			
			for (size_t i = 0; i < sceneObjects.meshes.size(); i++) {
				Mesh mesh;
				mesh.name = sceneObjects.meshes[i].name;
				
				if (loadVertexGroups(sceneObjects.meshes[i], sceneObjects, scene, mesh) != ASSET_SUCCESS) {
					vkcv_log(LogLevel::ERROR, "Failed to load vertex groups of '%s'! (%s)",
							 mesh.name.c_str(), path.c_str());
					return ASSET_ERROR;
				}
				
				scene.meshes.push_back(mesh);
			}
			
			// This only works if the node has a mesh and it only loads the meshes and ignores cameras and lights
			for (const auto& node : sceneObjects.nodes) {
				if ((node.mesh >= 0) && (node.mesh < scene.meshes.size())) {
					scene.meshes[node.mesh].modelMatrix = calculateModelMatrix(
							node.translation,
							node.scale,
							node.rotation,
							node.matrix
					);
				}
			}
		}
		
		if (sceneObjects.samplers.empty()) {
			vkcv_log(LogLevel::WARNING, "No samplers found! (%s)", path.c_str());
		} else {
			scene.samplers.reserve(sceneObjects.samplers.size());
			
			for (const auto &samplerObject : sceneObjects.samplers) {
				scene.samplers.push_back(loadSampler(samplerObject));
			}
		}
		
		if (sceneObjects.textures.empty()) {
			vkcv_log(LogLevel::WARNING, "No textures found! (%s)", path.c_str());
		} else {
			scene.textures.reserve(sceneObjects.textures.size());
			
			for (const auto& textureObject : sceneObjects.textures) {
				if ((textureObject.sampler < 0) ||
					(static_cast<size_t>(textureObject.sampler) >= scene.samplers.size())) {
					vkcv_log(LogLevel::WARNING, "Sampler of texture '%s' missing (%s)",
							 textureObject.name.c_str(), path.c_str());
					return ASSET_ERROR;
				}
				
				Texture texture;
				texture.sampler = textureObject.sampler;
				
				if ((textureObject.source < 0) ||
					(static_cast<size_t>(textureObject.source) >= sceneObjects.images.size())) {
					vkcv_log(LogLevel::WARNING, "Failed to load texture '%s' (%s)",
							 textureObject.name.c_str(), path.c_str());
					return ASSET_ERROR;
				}
				
				const auto& image = sceneObjects.images[textureObject.source];
				
				if (image.uri.empty()) {
					const fx::gltf::BufferView bufferView = sceneObjects.bufferViews[image.bufferView];
					
					texture.path.clear();
					texture.data.resize(bufferView.byteLength);
					memcpy(texture.data.data(),
						   sceneObjects.buffers[bufferView.buffer].data.data() + bufferView.byteOffset,
						   bufferView.byteLength);
				} else {
					texture.path = directory / image.uri;
				}
				
				scene.textures.push_back(texture);
			}
		}
		
		if (sceneObjects.materials.empty()) {
			vkcv_log(LogLevel::WARNING, "No materials found! (%s)", path.c_str());
		} else {
			scene.materials.reserve(sceneObjects.materials.size());
			
			for (auto material : sceneObjects.materials) {
				scene.materials.push_back({
						generateTextureMask(material),
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
	
		return ASSET_SUCCESS;
	}
	
	static int loadTextureData(Texture& texture) {
		if ((texture.width > 0) && (texture.height > 0) && (texture.channels > 0) &&
			(!texture.data.empty())) {
			return ASSET_SUCCESS; // Texture data was loaded already!
		}
		
		uint8_t* data;
		
		if (texture.path.empty()) {
			data = stbi_load_from_memory(
					reinterpret_cast<uint8_t*>(texture.data.data()),
					static_cast<int>(texture.data.size()),
					&texture.width,
					&texture.height,
					&texture.channels, 4
			);
		} else {
			data = stbi_load(
					texture.path.c_str(),
					&texture.width,
					&texture.height,
					&texture.channels, 4
			);
		}
		
		if (!data) {
			vkcv_log(LogLevel::ERROR, "Texture could not be loaded from '%s'",
					 texture.path.c_str());
			
			texture.width = 0;
			texture.height = 0;
			texture.channels = 0;
			return ASSET_ERROR;
		}
		
		texture.data.resize(texture.width * texture.height * 4);
		memcpy(texture.data.data(), data, texture.data.size());
		stbi_image_free(data);
	
		return ASSET_SUCCESS;
	}

	int loadMesh(Scene &scene, int index) {
		if ((index < 0) || (static_cast<size_t>(index) >= scene.meshes.size())) {
			vkcv_log(LogLevel::ERROR, "Mesh index out of range: %d", index);
			return ASSET_ERROR;
		}
		
		const Mesh &mesh = scene.meshes[index];
		
		for (const auto& vg : mesh.vertexGroups) {
			const VertexGroup &vertexGroup = scene.vertexGroups[vg];
			const Material& material = scene.materials[vertexGroup.materialIndex];
			
			if (material.hasTexture(PBRTextureTarget::baseColor)) {
				const int result = loadTextureData(scene.textures[material.baseColor]);
				if (ASSET_SUCCESS != result) {
					vkcv_log(LogLevel::ERROR, "Failed loading baseColor texture of mesh '%s'",
							 mesh.name.c_str())
					return result;
				}
			}
			
			if (material.hasTexture(PBRTextureTarget::metalRough)) {
				const int result = loadTextureData(scene.textures[material.metalRough]);
				if (ASSET_SUCCESS != result) {
					vkcv_log(LogLevel::ERROR, "Failed loading metalRough texture of mesh '%s'",
							 mesh.name.c_str())
					return result;
				}
			}
			
			if (material.hasTexture(PBRTextureTarget::normal)) {
				const int result = loadTextureData(scene.textures[material.normal]);
				if (ASSET_SUCCESS != result) {
					vkcv_log(LogLevel::ERROR, "Failed loading normal texture of mesh '%s'",
							 mesh.name.c_str())
					return result;
				}
			}
			
			if (material.hasTexture(PBRTextureTarget::occlusion)) {
				const int result = loadTextureData(scene.textures[material.occlusion]);
				if (ASSET_SUCCESS != result) {
					vkcv_log(LogLevel::ERROR, "Failed loading occlusion texture of mesh '%s'",
							 mesh.name.c_str())
					return result;
				}
			}
			
			if (material.hasTexture(PBRTextureTarget::emissive)) {
				const int result = loadTextureData(scene.textures[material.emissive]);
				if (ASSET_SUCCESS != result) {
					vkcv_log(LogLevel::ERROR, "Failed loading emissive texture of mesh '%s'",
							 mesh.name.c_str())
					return result;
				}
			}
		}
	
		return ASSET_SUCCESS;
	}
	
	int loadScene(const std::filesystem::path &path, Scene &scene) {
		int result = probeScene(path, scene);
		
		if (result != ASSET_SUCCESS) {
			vkcv_log(LogLevel::ERROR, "Loading scene failed '%s'",
					 path.c_str());
			return result;
		}
		
		for (size_t i = 0; i < scene.meshes.size(); i++) {
			result = loadMesh(scene, static_cast<int>(i));
			
			if (result != ASSET_SUCCESS) {
				vkcv_log(LogLevel::ERROR, "Loading mesh with index %d failed '%s'",
						 static_cast<int>(i), path.c_str());
				return result;
			}
		}
		
		return ASSET_SUCCESS;
	}
	
	Texture loadTexture(const std::filesystem::path& path) {
		Texture texture;
		texture.path = path;
		texture.sampler = -1;
		loadTextureData(texture);
		return texture;
	}

}
