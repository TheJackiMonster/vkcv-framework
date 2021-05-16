#pragma once
/**
 * @authors Trevor Hollmann
 * @file include/asset_loader/asset_loader.h
 * @brief Interface of the asset loader module for the vkcv framework.
 */

#include <string>
#include <vector>
#include <cstdint>


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

namespace asset {


/* With these enums, 0 is reserved to signal uninitialized or invalid data. */
enum PrimitiveMode { POINTS=1, LINES, TRIANGLES };
enum PrimitiveType { POSITION=1, NORMAL, TEXCOORD_0 };

typedef struct {
	// TODO not yet needed for the first (unlit) triangle
} Material;

typedef struct {
	PrimitiveType type;		// POSITION, NORMAL, ...
	uint32_t offset;		// offset in bytes
	uint32_t length;		// length of ... in bytes
	uint32_t stride;		// stride in bytes
	uint16_t componentType;		// eg. 5126 for float
	uint8_t  componentCount;	// eg. 3 for vec3
} VertexAttribute;

typedef struct {
	enum PrimitiveMode mode;	// draw as points, lines or triangle?
	size_t numIndices, numVertices;
	uint32_t *indices;		// array of indices for indexed rendering
	struct {
		void *data;		// the binary data of the buffer
		size_t byteLength;	// the length of the entire buffer in bytes
		std::vector<VertexAttribute> attributes;
	} vertexBuffer;
	struct { float x, y, z; } min;	// bounding box lower left
	struct { float x, y, z; } max;	// bounding box upper right
	uint8_t materialIndex;		// index to one of the meshes materials
} VertexGroup;

/* TODO This is just an initial draft of how a mesh loaded from a glTF file may
 * be presented to other modules of the vkcv framework. */
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
