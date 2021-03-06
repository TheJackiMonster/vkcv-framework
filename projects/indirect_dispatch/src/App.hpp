#pragma once
#include <vkcv/Core.hpp>
#include <vkcv/camera/CameraManager.hpp>
#include "AppSetup.hpp"
#include "MotionBlur.hpp"

class App {
public:
	App();
	bool initialize();
	void run();
private:
	const char* m_applicationName;

	uint32_t m_windowWidth;
	uint32_t m_windowHeight;

	vkcv::Core                  m_core;
	vkcv::WindowHandle          m_windowHandle;
	vkcv::camera::CameraManager m_cameraManager;

	MotionBlur m_motionBlur;

	vkcv::ImageHandle m_gridTexture;

	MeshResources m_cubeMesh;
	MeshResources m_groundMesh;

	GraphicPassHandles m_meshPass;
	GraphicPassHandles m_skyPass;
	GraphicPassHandles m_prePass;
	GraphicPassHandles m_skyPrePass;

	ComputePassHandles m_gammaCorrectionPass;

	AppRenderTargets    m_renderTargets;
	vkcv::SamplerHandle m_linearSampler;
};