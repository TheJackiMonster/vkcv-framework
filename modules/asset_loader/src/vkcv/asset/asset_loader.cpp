
#include "vkcv/asset/asset_loader.hpp"
#include <iostream>
#include <fx/gltf.h>

namespace vkcv::asset {

    /**
     * convert the accessor type from the fx-gltf library to an unsigned int
     * @param type
     * @return unsigned integer representation
     */
    uint8_t convertTypeToInt(const fx::gltf::Accessor::Type type){
        switch (type){
            case fx::gltf::Accessor::Type::None :
                return 0;
            case fx::gltf::Accessor::Type::Scalar :
                return 1;
            case fx::gltf::Accessor::Type::Vec2 :
                return 2;
            case fx::gltf::Accessor::Type::Vec3 :
                return 3;
            case fx::gltf::Accessor::Type::Vec4 :
                return 4;
            default: return 10; // TODO add cases for matrices (or maybe change the type in the struct itself)
        }
    }
	
	int loadMesh(const std::string &path, Mesh &mesh) {
		// load the gltf file using the library, return 0 if there is an
		// error.
		// If the library throws exceptions, catch them and maybe print an
		// error, but do not pass on the exception to the caller!
		// Looking at the example code of fx-gltf I would assume that
		// fx::gltf::LoadFromText(); is a good starting point.

        fx::gltf::Document object = {};
        std::vector<VertexAttribute> vertexAttributes = {};
        std::vector<VertexGroup> vertexGroups = {};
        std::vector<Material> materials = {};

        try {
            if (path.rfind(".glb") != std::string::npos){
                object = fx::gltf::LoadFromBinary(path);
            } else {
                object = fx::gltf::LoadFromText(path);
            }
        }
        catch (std::system_error){ // catch exception of invalid file path
            return 0;
        }
        catch (...){
            return 0;
        }

		// Verify that all required information can be found in the
		// struct/class returned by fx-gltf (eg. check if there is exactly one
		// mesh and that it has at least 3 vertices or something like that) and
		// return 0 if something is missing.
		// The important thing here is that we don't create an incomplete mesh
		// because we start copying data before making sure all the required
		// data is available.

        std::cout << "Number of Meshes loaded: " << object.meshes.size() << std::endl;
        std::cout << "Number of Primitives loaded: " << object.meshes[0].primitives.size() << std::endl;

        if (object.meshes.size() != 1){ // only continue if exactly one mesh is loaded
            return 0;
        }
        fx::gltf::Mesh const &objectMesh = object.meshes[0];
        fx::gltf::Primitive const &objectPrimitive = objectMesh.primitives[0];
        fx::gltf::Accessor posAccessor;

        // fill vertex attributes vector
        // for some reason the loop seems to run through the attributes the wrong way round. It starts with 2 and ends on 0.
        for (auto const & attrib : objectPrimitive.attributes){
            //std::cout << "AttType: " << attrib.second << std::endl;
            fx::gltf::Accessor accessor =  object.accessors[attrib.second];

            vertexAttributes.push_back(VertexAttribute{});
            // Primitive Type
            if (attrib.first == "POSITION"){
                vertexAttributes.back().type = POSITION;
                posAccessor = accessor;
            } else if (attrib.first == "NORMAL"){
                vertexAttributes.back().type = NORMAL;
            } else if (attrib.first == "TEXCOORD_0"){
                vertexAttributes.back().type = TEXCOORD_0;
            } else {
                return 0;
            }
            // Offset
            vertexAttributes.back().offset = object.bufferViews[accessor.bufferView].byteOffset;
            // Length
            vertexAttributes.back().length = object.bufferViews[accessor.bufferView].byteLength;
            // Stride
            vertexAttributes.back().stride = object.bufferViews[accessor.bufferView].byteStride;
            // Component Type
            vertexAttributes.back().componentType = static_cast<uint16_t>(accessor.componentType);
            // Component Count
            vertexAttributes.back().componentCount = convertTypeToInt(accessor.type); // Conversion doesn't work, don't know why
            std::cout << "Should be a number: " << vertexAttributes.back().componentCount << std::endl;
        }

        // Fill the output argument 'mesh' with the data from the loaded
		// glTF file.
		// Look at the structs 'Mesh', 'VertexGroup' and 'VertexAttribute'
		// defined in asset_loader.hpp and compare it to the glTF cheat sheet:
		// https://raw.githubusercontent.com/KhronosGroup/glTF/master/specification/2.0/figures/gltfOverview-2.0.0b.png
		//
		// If the fx::gltf::Document struct/class from the fx-gltf library
		// don't really match our structs, we may need to re-think how our
		// structs are defined (they are only based on the glTF specification).
		//
		// In the first iteration our goal is to simply load the vertices of a
		// mesh without textures and materials. (Maybe start with our triangle
		// since that does not have textures)
		// Once this works, we can add support for materials/textures, but I
		// wouldn't include this feature until loading of vertices+indices is
		// proven to work

        // indexBuffer
        fx::gltf::Accessor  & indexAccessor = object.accessors[objectPrimitive.indices];
        fx::gltf::BufferView  & indexBufferView = object.bufferViews[indexAccessor.bufferView];
        fx::gltf::Buffer  & indexBuffer = object.buffers[indexBufferView.buffer];

        // vertexBuffer
        fx::gltf::BufferView& vertexBufferView = object.bufferViews[posAccessor.bufferView];
        fx::gltf::Buffer& vertexBuffer = object.buffers[vertexBufferView.buffer];

        // fill vertex groups vector
        vertexGroups.push_back(VertexGroup{});
        vertexGroups.back() = {
                static_cast<PrimitiveMode>(objectPrimitive.mode), // mode
                object.accessors[objectPrimitive.indices].count, // num indices
                posAccessor.count, // num vertices
                { //index buffer
                        &indexBuffer.data[static_cast<uint64_t>(indexBufferView.byteOffset) + indexAccessor.byteOffset],
                        indexBufferView.byteLength,
                        indexBufferView.byteOffset
                },
                { //vertex buffer
                        &vertexBuffer.data[static_cast<uint64_t>(vertexBufferView.byteOffset) + posAccessor.byteOffset],
                        vertexBufferView.byteLength,
                        vertexAttributes
                },
                {posAccessor.min[0], posAccessor.min[1], posAccessor.min[2]}, // bounding box min
                {posAccessor.max[0], posAccessor.max[1], posAccessor.max[2]}, // bounding box max
                static_cast<uint8_t>(objectPrimitive.material) // material index
        };

        // fill mesh struct
        mesh = {
                object.meshes[0].name,
                vertexGroups,
                materials
        };

		// Finally return 1 to signal that all is fine
		return 1;
	}

}
