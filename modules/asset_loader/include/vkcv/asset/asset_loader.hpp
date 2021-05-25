#pragma once
/**
 * @authors Trevor Hollmann
 * @file include/vkcv/asset/asset_loader.h
 * @brief Interface of the asset loader module for the vkcv framework.
 */

#include <string>
#include <vector>
#include <cstdint>

/* These macros define limits of the following structs. Implementations can
 * test against these limits when performing sanity checks. The main constraint
 * expressed is that of the data type: Material indices are identified by a
 * uint8_t in the VertexGroup struct, so there can't be more than UINT8_MAX
 * materials in the mesh. Should these limits be too narrow, the data type has
 * to be changed, but the current ones should be generous enough for most use
 * cases. */
#define MAX_MATERIALS_PER_MESH UINT8_MAX
#define MAX_VERTICES_PER_VERTEX_GROUP UINT32_MAX

/* LOADING MESHES
 * The description of meshes is a hierarchy of structures with the Mesh at the
 * top.
 *
 * Each Mesh has an array of one or more vertex groups (called "primitives" in
 * glFW parlance) and an array of zero or more Materials.
 *
 * Each vertex group describes a part of the meshes vertices by defining how
 * they should be rendered (as points, lines, triangles), how many indices and
 * vertices there are, how the content of the vertex buffer is to be
 * interpreted and which material from the Meshes materials array should be
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

// enum matches modes in fx-gltf, the library returns a standard mode (TRIANGLES) if no mode is given in the file
enum PrimitiveMode { POINTS=0, LINES, LINELOOP, LINESTRIP, TRIANGLES, TRIANGLESTRIP, TRIANGLEFAN };
/* With these enums, 0 is reserved to signal uninitialized or invalid data. */
enum PrimitiveType { POSITION=1, NORMAL, TEXCOORD_0 };

typedef struct {
	// TODO not yet needed for the first (unlit) triangle
} Material;

/* This struct describes one vertex attribute of a vertex buffer. */
typedef struct {
	PrimitiveType type;		// POSITION, NORMAL, ...
	uint32_t offset;		// offset in bytes
	uint32_t length;		// length of ... in bytes
	uint32_t stride;		// stride in bytes
	uint16_t componentType;		// eg. 5126 for float
	uint8_t  componentCount;	// eg. 3 for vec3
} VertexAttribute;

/* This struct represents one (possibly the only) part of a mesh. There is
 * always one vertexBuffer and zero or one indexBuffer (indexed rendering is
 * common but not always used). If there is no indexBuffer, this is indicated
 * by indexBuffer.data being NULL.
 * Each vertex buffer can have one or more vertex attributes.
 * Note that the indexBuffer and vertexBuffer might be pointing to the same
 * block of memory.
 * 
 * TODO For now, the caller of loadMesh() has to free this memory when they are
 * done, but since this is not generally good practice in C++, this behaviour
 * will likely be changed later. */
typedef struct {
	enum PrimitiveMode mode;	// draw as points, lines or triangle?
	size_t numIndices, numVertices;
	struct {
		void *data;		// binary data of the index buffer
		size_t byteLength;	// length of the index buffer
		uint32_t byteOffset;	// offset into the buffer in bytes
	} indexBuffer;
	struct {
		void *data;		// the binary data of the buffer
		size_t byteLength;	// the length of the entire buffer in bytes
		std::vector<VertexAttribute> attributes;
	} vertexBuffer;
	struct { float x, y, z; } min;	// bounding box lower left
	struct { float x, y, z; } max;	// bounding box upper right
	uint8_t materialIndex;		// index to one of the meshes materials
} VertexGroup;

/* This struct represents a single mesh loaded from a glTF file. It consists of
 * at least one VertexVroup and any number of Materials. */
typedef struct {
	std::string name;
	std::vector<VertexGroup> vertexGroups;
	std::vector<Material> materials;
} Mesh;


/**
 * In its first iteration the asset loader module will only allow loading
 * single meshes, one per glTF file.
 * It will later be extended to allow loading entire scenes from glTF files.
 *
 * @param path must be the path to a glTF file containing a single mesh.
 * @param mesh is a reference to a Mesh struct that will be filled with the
 * 	content of the glTF file being loaded.
 * */
int loadMesh(const std::string &path, Mesh &mesh);


}
