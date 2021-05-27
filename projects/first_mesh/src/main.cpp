#include <iostream>
#include <stdio.h>
#include <vkcv/asset/asset_loader.hpp>


int main(int argc, const char** argv) {
	vkcv::asset::Mesh mesh;
	
	const char *path = argc > 1 ? argv[1] : "resources/cube/cube.gltf";
	int result = vkcv::asset::loadMesh(path, mesh);
	
	if (result == 1) {
		std::cout << "Mesh loading successful!" << std::endl;
	} else {
		std::cout << "Mesh loading failed: " << result << std::endl;
		return 1;
	}

	/* Demonstration of how to use the vkcv::asset::Mesh struct. */
	const char *primitive_modes[] = {
		"points", "lines", "line loop", "line strip", "triangles",
		"triangle strip", "triangle fan"
	};
	printf("Mesh %s (%s) has %lu vertex group(s) and %lu material(s):\n",
			mesh.name.c_str(), path, mesh.vertexGroups.size(),
			mesh.materials.size());
	for (size_t i = 0; i < mesh.vertexGroups.size(); i++) {
		printf("--- vertex group %lu ---\n", i);
		const auto &vg = mesh.vertexGroups[i];
		printf("primitive mode: %d (%s)\n", vg.mode,
				primitive_modes[vg.mode]);
		printf("index buffer: %lu bytes for %lu indices (offset into "
				"%p is %u)\n", vg.indexBuffer.byteLength,
				vg.numIndices, vg.indexBuffer.data,
				vg.indexBuffer.byteOffset);
		uint16_t *indices = (uint16_t*)vg.indexBuffer.data;
		printf("\tindices: ");
		for (size_t j = 0; j < vg.numIndices; j++) {
			printf("%u ", indices[j]);
		}
		printf("\n");
		printf("vertex buffer: %lu bytes for %lu vertices with %lu "
				"attributes (starting at %p)\n",
				vg.vertexBuffer.byteLength, vg.numVertices,
				vg.vertexBuffer.attributes.size(),
				vg.vertexBuffer.data);
	}
	
	return 0;
}
