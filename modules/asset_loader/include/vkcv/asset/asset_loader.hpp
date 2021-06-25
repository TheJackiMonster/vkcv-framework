#pragma once
/**
 * @authors Trevor Hollmann, Mara Vogt, Susanne Dötsch
 * @file include/vkcv/asset/asset_loader.h
 * @brief Interface of the asset loader module for the vkcv framework.
 */

#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <filesystem>

/** These macros define limits of the following structs. Implementations can
 * test against these limits when performing sanity checks. The main constraint
 * expressed is that of the data type: Material indices are identified by a
 * uint8_t in the VertexGroup struct, so there can't be more than UINT8_MAX
 * materials in the mesh. Should these limits be too narrow, the data type has
 * to be changed, but the current ones should be generous enough for most use
 * cases. */
#define MAX_MATERIALS_PER_MESH UINT8_MAX
#define MAX_VERTICES_PER_VERTEX_GROUP UINT32_MAX

/** LOADING MESHES
 * The description of meshes is a hierarchy of structures with the Mesh at the
 * top.
 *
 * Each Mesh has an array of one or more vertex groups (called "primitives" in
 * glTF parlance). Specifically, it has an array of indices into an array of
 * vertex groups defined by the Scene struct.
 *
 * Each vertex group describes a part of the meshes vertices by defining how
 * they should be rendered (as points, lines, triangles), how many indices and
 * vertices there are, how the content of the vertex buffer is to be
 * interpreted and which material from the Scenes materials array should be
 * used for the surface of the vertices.
 * As a bonus there is also the axis aligned bounding box of the vertices.
 *
 * The vertex buffer is presented as a single block of binary data with a given
 * length in bytes.
 * The layout of the vertex buffer is described by an array of VertexAttribute
 * structs that define the type of attribute, the offset, length and stride in
 * bytes and number and type of components of the attribute.
 * These values can directly be given to vulkan when describing the content of
 * vertex buffers. */

namespace vkcv::asset {

/* These return codes are limited to the asset loader module. If unified return
 * codes are defined for the vkcv framework, these will be used instead. */
#define ASSET_ERROR 0
#define ASSET_SUCCESS 1

/** This enum matches modes in fx-gltf, the library returns a standard mode
 * (TRIANGLES) if no mode is given in the file. */
enum class PrimitiveMode : uint8_t {
	POINTS=0, LINES, LINELOOP, LINESTRIP, TRIANGLES, TRIANGLESTRIP,
	TRIANGLEFAN
};

/** The indices in the index buffer can be of different bit width. */
enum class IndexType : uint8_t { UNDEFINED=0, UINT8=1, UINT16=2, UINT32=3 };

/* NOTE: glTF defines samplers based on OpenGL, which can not be
 * directly translated to Vulkan. The vkcv::asset::Sampler struct defined here
 * adheres to the Vulkan spec, having alerady translated the flags from glTF to
 * Vulkan. All values here can directly be passed to VkSamplerCreateInfo.
 * */
typedef struct {
	int minFilter, magFilter;
	int mipmapMode;
	float minLOD, maxLOD;
	int addressModeU, addressModeV, addressModeW;
} Sampler;

/** struct for defining the loaded texture */
typedef struct {
	int sampler;		// index into the sampler array of the Scene
	uint8_t channels;	// number of channels
	uint16_t w, h;		// width and height of the texture
	std::vector<uint8_t> data;	// binary data of the decoded texture
} Texture;

/** The asset loader module only supports the PBR-MetallicRoughness model for
 * materials.*/
typedef struct {
	uint16_t textureMask;	// bit mask with active texture targets
	// Indices into the Scene.textures vector
	int baseColor, metalRough, normal, occlusion, emissive;
	// Scaling factors for each texture target
	struct { float r, g, b, a; } baseColorFactor;
	float metallicFactor, roughnessFactor;
	float normalScale;
	float occlusionStrength;
	struct { float r, g, b; } emissiveFactor;
} Material;

/** Flags for the bit-mask in the Material struct. To check if a material has a
 * certain texture target, you can use the hasTexture() function below, passing
 * the material struct and the enum. */
enum class PBRTextureTarget {
	baseColor=1, metalRough=2, normal=4, occlusion=8, emissive=16
};

/** This macro translates the index of an enum in the defined order to an
 * integer with a single bit set in the corresponding place. It is used for
 * working with the bitmask of texture targets ("textureMask") in the Material
 * struct:
 * 	Material mat = ...;
 * 	if (mat.textureMask & bitflag(PBRTextureTarget::baseColor)) {...}
 * However, this logic is also encapsulated in the convenience-function
 * materialHasTexture() so users of the asset loader module can avoid direct
 * contact with bit-level operations. */
#define bitflag(ENUM) (0x1u << ((unsigned)(ENUM)))

/**
 * To signal that a certain texture target is active in a Material struct, its
 * bit is set in the textureMask. You can use this function to check that:
 * Material mat = ...;
 * if (materialHasTexture(&mat, baseColor)) {...}
 * @param m The material to query
 * @param t The target to query for
 * @return Boolean to signal whether the texture target is active in the
 * material.
 */
bool materialHasTexture(const Material *const m, const PBRTextureTarget t);

/* With these enums, 0 is reserved to signal uninitialized or invalid data. */
enum class PrimitiveType : uint32_t {
    UNDEFINED = 0, POSITION = 1, NORMAL, TANGENT, TEXCOORD_0, TEXCOORD_1,
    COLOR_0, JOINTS_0, WEIGHTS_0
};

/** These integer values are used the same way in OpenGL, Vulkan and glTF. This
 * enum is not needed for translation, it's only for the programmers
 * convenience (easier to read in if/switch statements etc). While this enum
 * exists in (almost) the same definition in the fx-gltf library, we want to
 * avoid exposing that dependency, thus it is re-defined here. */
enum class ComponentType : uint16_t {
    NONE = 0, INT8 = 5120, UINT8 = 5121, INT16 = 5122, UINT16 = 5123,
    UINT32 = 5125, FLOAT32 = 5126
};

/** This struct describes one vertex attribute of a vertex buffer. */
typedef struct {
    PrimitiveType type;			// POSITION, NORMAL, ...
    uint32_t offset;			// offset in bytes
    uint32_t length;			// length of ... in bytes
    uint32_t stride;			// stride in bytes
    ComponentType componentType;	// eg. 5126 for float
    uint8_t  componentCount;		// eg. 3 for vec3
} VertexAttribute;

/** This struct represents one (possibly the only) part of a mesh. There is
 * always one vertexBuffer and zero or one indexBuffer (indexed rendering is
 * common but not always used). If there is no index buffer, this is indicated
 * by indexBuffer.data being empty. Each vertex buffer can have one or more
 * vertex attributes. */
typedef struct {
	enum PrimitiveMode mode;	// draw as points, lines or triangle?
	size_t numIndices, numVertices;
	struct {
		enum IndexType type;	// data type of the indices
		std::vector<uint8_t> data; // binary data of the index buffer
	} indexBuffer;
	struct {
		std::vector<uint8_t> data; // binary data of the vertex buffer
		std::vector<VertexAttribute> attributes; // description of one
	} vertexBuffer;
	struct { float x, y, z; } min;	// bounding box lower left
	struct { float x, y, z; } max;	// bounding box upper right
	int materialIndex;		// index to one of the materials
} VertexGroup;

/** This struct represents a single mesh as it was loaded from a glTF file. It
 * consists of at least one VertexGroup, which then references other resources
 * such as Materials. */
typedef struct {
	std::string name;
	std::array<float, 16> modelMatrix;
	std::vector<int> vertexGroups;
} Mesh;

/** The scene struct is simply a collection of objects in the scene as well as
 * the resources used by those objects.
 * For now the only type of object are the meshes and they are represented in a
 * flat array.
 * Note that parent-child relations are not yet possible. */
typedef struct {
	std::vector<Mesh> meshes;
	std::vector<VertexGroup> vertexGroups;
	std::vector<Material> materials;
	std::vector<Texture> textures;
	std::vector<Sampler> samplers;
} Scene;

/**
 * Load every mesh from the glTF file, as well as materials and textures.
 *
 * @param path must be the path to a glTF or glb file.
 * @param scene is a reference to a Scene struct that will be filled with the
 * 	content of the glTF file being loaded.
 * */
int loadScene(const std::filesystem::path &path, Scene &scene);


}
