#include "asset_loader/asset_loader.hpp"
// TODO include the library: fx-gltf


int loadMesh(const std::string &path, Mesh &mesh)
{
	// TODO load the gltf file using the library, return 0 if there is an
	// error.
	// If the library throws excaptions, catch them and maybe print an
	// error, but do not pass on the exception to the caller!
	// Looking at the example code of fx-gltf I would assume that
	// fx::gltf::LoadFromText(); is a good starting point.

	// TODO Verify that all required information can be found in the
	// struct/class returned by fx-gltf (eg. check if there is exactly one
	// mesh and that it has at least 3 vertices or something like that) and
	// return 0 if something is missing.
	// The important thing here is that we don't create an incomplete mesh
	// because we start copying data before making sure all the required
	// data is available.

	// TODO Fill the output argument 'mesh' with the data from the loaded
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
	
	// Finally return 1 to signal that all is fine
}
