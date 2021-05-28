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
	const char *primitive_types[] = {
		"unknown", "position", "normal", "texcoord0"
	};
	printf("Mesh %s (%s) has %lu vertex group(s) and %lu material(s):\n",
			mesh.name.c_str(), path, mesh.vertexGroups.size(),
			mesh.materials.size());
	for (size_t i = 0; i < mesh.vertexGroups.size(); i++) {
		printf("--- vertex group %lu ---\n", i);
		const auto &vg = mesh.vertexGroups[i];
		printf("primitive mode: %d (%s)\n", vg.mode,
				primitive_modes[vg.mode]);
		printf("index buffer: %lu bytes for %lu indices ",
				vg.indexBuffer.data.size(), vg.numIndices);
		const auto itype = vg.indexBuffer.type;
		printf("(%s @ %p)\n",
				itype == vkcv::asset::UINT32 ? "UINT32" :
				itype == vkcv::asset::UINT16 ? "UINT16" :
				"UINT8", vg.indexBuffer.data.data());
		printf("\tindices: ");
		const size_t n = vg.numIndices;
		if (vg.indexBuffer.type == vkcv::asset::UINT32) {
			uint32_t *idx = (uint32_t*)vg.indexBuffer.data.data();
			for (size_t j = 0; j < n; j++) printf("%u ", idx[j]);
		} else
		if (vg.indexBuffer.type == vkcv::asset::UINT16) {
			uint16_t *idx = (uint16_t*)vg.indexBuffer.data.data();
			for (size_t j = 0; j < n; j++) printf("%u ", idx[j]);
		} else
		if (vg.indexBuffer.type == vkcv::asset::UINT8) {
			uint8_t *idx = (uint8_t*)vg.indexBuffer.data.data();
			for (size_t j = 0; j < n; j++) printf("%u ", idx[j]);
		} else {
			fprintf(stderr, "ERROR Invalid IndexType: %d\n",
					vg.indexBuffer.type);
			return 0;
		}
		printf("\n");
		printf("vertex buffer: %lu bytes for %lu vertices with %lu "
				"attributes (starting at %p)\n",
				vg.vertexBuffer.data.size(), vg.numVertices,
				vg.vertexBuffer.attributes.size(),
				vg.vertexBuffer.data.data());
		printf("attributes:\toffset\tlength\tstride\tcomponents\n");
		for (const auto att : vg.vertexBuffer.attributes) {
			printf("%11s\t%u\t%u\t%u\t%hhux%hu\n",
					primitive_types[att.type],
					att.offset, att.length, att.stride,
					att.componentCount, att.componentType);
		}
	}
	
	return 0;
}
