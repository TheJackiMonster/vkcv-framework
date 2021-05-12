#include <iostream>
#include <vkcv/Core.hpp>
#include <vkcv/Window.hpp>

int main(int argc, const char** argv) {
    const char* applicationName = "First Triangle";
	vkcv::Window window = vkcv::Window::create(
            applicationName,
        800,
        600,
		false
	);
	vkcv::Core core = vkcv::Core::create(
            applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		20,
		{vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eTransfer}
	);

	const auto &context = core.getContext();
	const vk::Instance& instance = context.getInstance();
	const vk::PhysicalDevice& physicalDevice = context.getPhysicalDevice();
	const vk::Device& device = context.getDevice();

	std::cout << "Physical device: " << physicalDevice.getProperties().deviceName << std::endl;

	switch (physicalDevice.getProperties().vendorID) {
		case 0x1002: std::cout << "Running AMD huh? You like underdogs, are you a Linux user?" << std::endl; break;
		case 0x10DE: std::cout << "An NVidia GPU, how predictable..." << std::endl; break;
		case 0x8086: std::cout << "Poor child, running on an Intel GPU, probably integrated..."
			"or perhaps you are from the future with a dedicated one?" << std::endl; break;
		case 0x13B5: std::cout << "ARM? What the hell are you running on, next thing I know you're trying to run Vulkan on a leg..." << std::endl; break;
		default: std::cout << "Unknown GPU vendor?! Either you're on an exotic system or your driver is broken..." << std::endl;
	}

    // an example attachment for passes that output to the window
	vkcv::AttachmentDescription present_color_attachment(vkcv::AttachmentLayout::UNDEFINED,
                                                    vkcv::AttachmentLayout::COLOR_ATTACHMENT,
                                                    vkcv::AttachmentLayout::PRESENTATION,
                                                    vkcv::AttachmentOperation::STORE,
                                                    vkcv::AttachmentOperation::CLEAR);
    // an example attachment for passes that output to a depth buffer
	vkcv::AttachmentDescription present_depth_attachment(vkcv::AttachmentLayout::UNDEFINED,
                                                    vkcv::AttachmentLayout::DEPTH_STENCIL_ATTACHMENT,
                                                    vkcv::AttachmentLayout::DEPTH_STENCIL_READ_ONLY,
                                                    vkcv::AttachmentOperation::STORE,
                                                    vkcv::AttachmentOperation::CLEAR);

	// this pass will output to the window, and produce a depth buffer
	vkcv::Renderpass test_pass{};
	test_pass.attachments.push_back(present_color_attachment);
	test_pass.attachments.push_back(present_depth_attachment);

	std::vector<vkcv::RenderpassHandle> test_handles{};
    // render pass creation test
    for(uint32_t i = 0; i < 1000; i++)
    {
        vkcv::RenderpassHandle tmp_handle{};
        if(!core.createRenderpass(test_pass, tmp_handle))
        {
            std::cout << "Oops. Something went wrong in the renderpass creation. Exiting." << std::endl;
            return EXIT_FAILURE;
        }
        test_handles.push_back(tmp_handle);
    }
    std::cout << "Wow. You just made 1000 render passes. (That are all identical, though...)" << std::endl;

	/*
	 * BufferHandle triangleVertices = core.createBuffer(vertices);
	 * BufferHandle triangleIndices = core.createBuffer(indices);
	 *
	 * // triangle Model creation goes here
	 *
	 *
	 * // attachment creation goes here
	 * PassHandle trianglePass = core.CreatePass(presentationPass);
	 *
	 * // shader creation goes here
	 * // material creation goes here
	 *
	 * PipelineHandle trianglePipeline = core.CreatePipeline(trianglePipeline);
	 */

	while (window.isWindowOpen())
	{
        // core.beginFrame(); or something like that
	    // core.execute(trianglePass, trianglePipeline, triangleModel);
	    // core.endFrame(); or something like that

	    // TBD: synchronization

		window.pollEvents();
	}
	return 0;
}
