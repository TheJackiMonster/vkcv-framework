
#include "vkcv/asset/asset_loader.hpp"
#include <iostream>
#include <fx/gltf.h>

namespace vkcv::asset {
	
	int loadMesh(const std::string &path, Mesh &mesh) {
		std::vector<uint8_t> v;
		
		if (fx::base64::TryDecode(path, v)) {
			for (auto& ve : v) {
				std::cout << ve << " ";
			}
			
			std::cout << std::endl;
		} else {
			std::cerr << "Schade!" << std::endl;
		}
		
		// TODO
		return -1;
	}

}
