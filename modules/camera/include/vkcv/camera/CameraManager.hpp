#pragma once

#include "PilotCameraController.hpp"
#include "TrackballCameraController.hpp"
#include "CameraController.hpp"
#include "vkcv/Window.hpp"
#include <GLFW/glfw3.h>
#include <functional>

namespace vkcv::camera {

    /**
     * @brief Used for specifying existing types of camera controllers when adding a new controller object to the
     * #CameraManager.
     */
    enum class ControllerType {
        NONE,
        PILOT,
        TRACKBALL,
    };

    /**
     * @brief Used for managing an arbitrary amount of camera controllers.
     */
    class CameraManager{
    private:
		event_handle<int, int, int, int> m_keyHandle;
		event_handle<double, double> m_mouseMoveHandle;
		event_handle<double, double> m_mouseScrollHandle;
		event_handle<int, int, int> m_mouseButtonHandle;
		event_handle<int, int> m_resizeHandle;
        event_handle<int> m_gamepadHandle;

        Window& m_window;
        std::vector<Camera> m_cameras;
        std::vector<ControllerType> m_cameraControllerTypes;
        uint32_t m_activeCameraIndex;

        PilotCameraController m_pilotController;
        TrackballCameraController m_trackController;

        double m_lastX;
        double m_lastY;

        double m_inputDelayTimer;
        double m_frameTime;

        /**
         * @brief Binds the camera object to the window event handles.
         */
        void bindCameraToEvents();

        /**
         * @brief A callback function for key events. Currently, cycling between all existing camera controllers via Tab,
         * window closure via Esc and polling key events from the active camera controller are supported.
         * @param[in] key The keyboard key.
         * @param[in] scancode The platform-specific scancode.
         * @param[in] action The key action.
         * @param[in] mods The modifier bits.
         */
        void keyCallback(int key, int scancode, int action, int mods);

        /**
         * @brief A callback function for mouse scrolling events.
         * @param[in] offsetX The offset in horizontal direction.
         * @param[in] offsetY The offset in vertical direction.
         */
        void scrollCallback(double offsetX, double offsetY);

        /**
         * @brief A callback function for mouse movement events.
         * @param[in] x The horizontal mouse position.
         * @param[in] y The vertical mouse position.
         */
        void mouseMoveCallback(double x, double y);

        /**
         * @brief A callback function for mouse button events.
         * @param[in] button The mouse button.
         * @param[in] action The button action.
         * @param[in] mods The modifier bits.
         */
        void mouseButtonCallback(int button, int action, int mods);

        /**
         * @brief A callback function for handling the window resizing event. Each existing camera is resized in respect
         * of the window size.
         * @param[in] width The new width of the window.
         * @param[in] height The new height of the window.
         */
        void resizeCallback(int width, int height);

        /**
         * @brief A callback function for gamepad input events. Currently, inputs are handled only for the first
         * connected gamepad!
         * @param gamepadIndex The gamepad index.
         */
        void gamepadCallback(int gamepadIndex);
	
		/**
		 * @brief Gets a camera controller object of specified @p controllerType.
		 * @param[in] controllerType The type of the camera controller.
		 * @return The specified camera controller object.
		 */
		CameraController& getControllerByType(ControllerType controllerType);
        
        /**
         * @briof A method to get the currently active controller for the active camera.
         * @return Reference to the active #CameraController
         */
        CameraController& getActiveController();

        /**
         * @brief Returns 'true' if the camera has a controller.
         * 
         * @param cameraIndex 
         * @return true 
         * @return false 
         */
        bool CameraManager::cameraHasController(uint32_t cameraIndex);

    public:

        /**
         * @brief The constructor of the #CameraManager.
         * @param[in] window The window.
         */
        CameraManager(Window &window);

        /**
         * @brief The destructor of the #CameraManager. Destroying the #CameraManager leads to deletion of all stored
         * camera objects and camera controller objects.
         */
        ~CameraManager();

        /**
         * @brief Adds a new camera object to the #CameraManager and binds it to a camera controller object of specified
         * @p controllerType.
         * @param[in] controllerType The type of the camera controller.
         * @return The index of the newly created camera object.
         */
		uint32_t addCamera(ControllerType controllerType = ControllerType::NONE);
	
		/**
		 * @brief Adds a new camera object to the #CameraManager and binds it to a camera controller object of specified
		 * @p controllerType.
		 * @param[in] controllerType The type of the camera controller.
		 * @param[in] camera The new camera object.
		 * @return The index of the newly bound camera object.
		 */
		uint32_t addCamera(ControllerType controllerType, const Camera& camera);

        /**
         * @brief Gets the stored camera object located at @p cameraIndex.
         * @param[in] cameraIndex The camera index.
         * @return The camera object at @p cameraIndex.
         * @throws std::runtime_error If @p cameraIndex is not a valid camera index.
         */
        Camera& getCamera(uint32_t cameraIndex);

        /**
         * @brief Gets the stored camera object set as the active camera.
         * @return The active camera.
         */
        Camera& getActiveCamera();

        /**
         * @brief Sets the stored camera object located at @p cameraIndex as the active camera.
         * @param[in] cameraIndex The camera index.
         * @throws std::runtime_error If @p cameraIndex is not a valid camera index.
         */
        void setActiveCamera(uint32_t cameraIndex);

        /**
         * @brief Gets the index of the stored active camera object.
         * @return The active camera index.
         */
        uint32_t getActiveCameraIndex() const;

        /**
         * @brief Binds a stored camera object located at @p cameraIndex to a camera controller of specified
         * @p controllerType.
         * @param[in] cameraIndex The camera index.
         * @param[in] controllerType The type of the camera controller.
         * @throws std::runtime_error If @p cameraIndex is not a valid camera index.
         */
        void setControllerType(uint32_t cameraIndex, ControllerType controllerType);

        /**
         * @brief Gets the currently bound camera controller type of the stored camera object located at @p cameraIndex.
         * @param[in] cameraIndex The camera index.
         * @return The type of the camera controller of the specified camera object.
         * @throws std::runtime_error If @p cameraIndex is not a valid camera index.
         */
        ControllerType getControllerType(uint32_t cameraIndex);
        
        /**
         * @brief Updates all stored camera controllers in respect to @p deltaTime.
         * @param[in] deltaTime The time that has passed since last update.
         */
        void update(double deltaTime);
    };
}
