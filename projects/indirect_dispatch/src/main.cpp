#include "App.hpp"

int main(int argc, const char** argv) {

	App app;
	if (!app.initialize()) {
		std::cerr << "Application initialization failed, exiting" << std::endl;
	}
	app.run();

	return 0;
}
