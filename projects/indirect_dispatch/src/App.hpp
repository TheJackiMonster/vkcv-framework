#pragma once
#include <vkcv/Core.hpp>
#include <vkcv/camera/CameraManager.hpp>
#include "AppSetup.hpp"

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

	MeshResources m_sphereMesh;
	MeshResources m_cubeMesh;

	GraphicPassHandles m_meshPass;
	GraphicPassHandles m_skyPass;
	GraphicPassHandles m_prePass;
	GraphicPassHandles m_skyPrePass;

	ComputePassHandles m_gammaCorrectionPass;
	ComputePassHandles m_motionBlurDummyPass;
	ComputePassHandles m_motionVectorMaxPass;
	ComputePassHandles m_motionVectorMaxNeighbourhoodPass;

	RenderTargets       m_renderTargets;
	vkcv::SamplerHandle m_linearSampler;
};