#include <iostream>
#include <vkcv/asset/asset_loader.hpp>

int main(int argc, const char** argv) {
	vkcv::asset::Mesh mesh;
	
	int result = vkcv::asset::loadMesh("test.gltf", mesh);
	
	if (result == 0) {
		std::cout << "Mesh loading successful!" << std::endl;
	} else {
		std::cout << "Mesh loading failed: " << result << std::endl;
	}
	
	return 0;
}
