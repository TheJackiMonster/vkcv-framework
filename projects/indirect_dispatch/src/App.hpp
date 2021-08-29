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

	int m_windowWidth;
	int m_windowHeight;

	vkcv::Window                m_window;
	vkcv::Core                  m_core;
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