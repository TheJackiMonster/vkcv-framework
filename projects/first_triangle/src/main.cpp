#include <iostream>
#include <vkcv/Core.hpp>
#include <vkcv/Window.hpp>
#include <vkcv/ShaderProgram.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/Camera.hpp>
#include <vkcv/camera/TrackballCamera.hpp>


int main(int argc, const char** argv) {
    const char* applicationName = "First Triangle";

	const int windowWidth = 800;
	const int windowHeight = 600;
    vkcv::Window window = vkcv::Window::create(
		applicationName,
		windowWidth,
		windowHeight,
		false
    );

    // TODO: this code will be put in a camera controller class
    vkcv::Camera camera;
    std::shared_ptr<vkcv::TrackballCamera> trackball;
    camera.setPerspective( glm::radians(60.0f), windowWidth / (float)windowHeight, 0.1f, 10.f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    glm::vec3 position(1.0f, 0.0f, 0.0f);
    glm::vec3 front(0.0f, 0.0f, -1.0f);
    glm::vec3 center = position + front;
    camera.lookAt(position, center, up);
    const float radius = 10.0f;
    const float cameraSpeed = 0.05f;
    float roll = 0.0;
    float pitch = 0.0;
    float yaw = 0.0;

    //TODO? should the standard camera support rotation?

    bool firstMouse = true;
    double lastX, lastY;

    // showing basic usage lambda events of window
    window.e_mouseMove.add([&](double x, double y) {
        std::cout << "movement: " << x << " , " << y << std::endl;

        if (firstMouse) {
            lastX = x;
            lastY = y;
            firstMouse = false;
        }

        float xoffset = x - lastX;
        float yoffset = lastY - y;
        lastX = x;
        lastY = y;

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (pitch > 89.0f) {
            pitch = 89.0f;
        }
        if (pitch < -89.0f) {
            pitch = -89.0f;
        }

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        front = glm::normalize(direction);
        center = position + front;
        camera.lookAt(position, center, up);

        std::cout << "New center: " << center.x << ", " << center.y << ", " << center.z << std::endl;
    });

    window.e_mouseScroll.add([&](double xoffset, double yoffset) {
        float fov = camera.getFov();
        fov -= (float)yoffset;
        if (fov < 1.0f) {
            fov = 1.0f;
        }
        if (fov > 45.0f) {
            fov = 45.0f;
        }
        camera.setFov(fov);
        std::cout << "New FOV: " << fov << std::endl;
    });

    window.e_key.add([&](int key, int scancode, int action, int mods) {
        switch (key) {
            case GLFW_KEY_W:
                std::cout << "Move forward" << std::endl;
                position += cameraSpeed * front;
                center = position + front;
                camera.lookAt(position, center, up);
                break;
            case GLFW_KEY_S:
                std::cout << "Move left" << std::endl;
                position -= cameraSpeed * front;
                center = position + front;
                camera.lookAt(position, center, up);
                break;
            case GLFW_KEY_A:
                std::cout << "Move backward" << std::endl;
                position -= glm::normalize(glm::cross(front, up)) * cameraSpeed;
                center = position + front;
                camera.lookAt(position, center, up);
                break;
            case GLFW_KEY_D:
                std::cout << "Move right" << std::endl;
                position += glm::normalize(glm::cross(front, up)) * cameraSpeed;
                center = position + front;
                camera.lookAt(position, center, up);
                break;
            default:
                std::cout << "this key is not supported yet: " << std::endl;
        }
    });

    window.initEvents();

	vkcv::Core core = vkcv::Core::create(
            window,
            applicationName,
		VK_MAKE_VERSION(0, 0, 1),
            {vk::QueueFlagBits::eTransfer,vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute},
		{},
		{"VK_KHR_swapchain"}
	);

	const auto &context = core.getContext();
	const vk::Instance& instance = context.getInstance();
	const vk::PhysicalDevice& physicalDevice = context.getPhysicalDevice();
	const vk::Device& device = context.getDevice();
	
	struct vec3 {
		float x, y, z;
	};
	
	auto buffer = core.createBuffer<vec3>(vkcv::BufferType::VERTEX, 3);
	
	vec3* m = buffer.map();
	m[0] = { 0, 0, 0 };
	m[0] = { 0, 0, 0 };
	m[0] = { 0, 0, 0 };

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
	const vkcv::AttachmentDescription present_color_attachment(
		vkcv::AttachmentLayout::UNDEFINED,
		vkcv::AttachmentLayout::COLOR_ATTACHMENT,
		vkcv::AttachmentLayout::PRESENTATION,
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		core.getSwapchainImageFormat());

	vkcv::PassConfig trianglePassDefinition({present_color_attachment});
	vkcv::PassHandle trianglePass = core.createPass(trianglePassDefinition);

	if (trianglePass.id == 0)
	{
		std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	vkcv::ShaderProgram triangleShaderProgram{};
	triangleShaderProgram.addShader(vkcv::ShaderStage::VERTEX, std::filesystem::path("shaders/vert.spv"));
	triangleShaderProgram.addShader(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("shaders/frag.spv"));

	const vkcv::PipelineConfig trianglePipelineDefinition(triangleShaderProgram, windowWidth, windowHeight, trianglePass);
	vkcv::PipelineHandle trianglePipeline = core.createGraphicsPipeline(trianglePipelineDefinition);
	if (trianglePipeline.id == 0)
	{
		std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

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
		core.beginFrame();
	    core.renderTriangle(trianglePass, trianglePipeline, windowWidth, windowHeight);
	    core.endFrame();
	}
	return 0;
}
