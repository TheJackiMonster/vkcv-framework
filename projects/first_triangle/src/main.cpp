
#include <iostream>
#include <vkcv/Context.hpp>

int main(int argc, const char** argv) {
	vkcv::Context context = vkcv::Context::create(
			"First Triangle",
			VK_MAKE_VERSION(0, 0, 1)
	);

	std::cout << "Hello world" << std::endl;

	return 0;
}
