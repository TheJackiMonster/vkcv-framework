#pragma once
/**
 * @authors Vanessa Karolek, Josch Morgenstern, Sebastian Gaida, Katharina Krämer, Tobias Frisch, Alexander Gauggel
 * @file include/vkcv/camera/CameraManager.hpp
 * @brief CameraManager class of the camera module for the vkcv framework. The camera manager manages several camera
 * controller objects. Camera objects can be created and bound to a specific camera controller via this class.
 */

#include "CameraHandle.hpp"
#include "ControllerType.hpp"
#include "PilotCameraController.hpp"
#include "TrackballCameraController.hpp"
#include "CameraController.hpp"

#include "vkcv/Window.hpp"

#include <GLFW/glfw3.h>
#include <functional>

namespace vkcv::camera {

    /**
     * @addtogroup vkcv_camera
     * @{
     */

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
        uint64_t m_activeCameraIndex;

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
		 * @brief Returns a camera controller object of specified @p controllerType.
		 * @param[in] controllerType The type of the camera controller.
		 * @return The specified camera controller object.
		 */
		CameraController& getControllerByType(ControllerType controllerType);

    public:

        /**
         * @brief The constructor of the #CameraManager.
         * @param[in] window The window.
         */
        explicit CameraManager(Window &window);

        /**
         * @brief The destructor of the #CameraManager. Destroying the #CameraManager leads to deletion of all stored
         * camera objects and camera controller objects.
         */
        ~CameraManager();

        /**
         * @brief Adds a new camera object to the #CameraManager and binds it to a camera controller object of specified
         * @p controllerType.
         * @param[in] controllerType The type of the camera controller.
         * @return The handle of the newly created camera object.
         */
		CameraHandle addCamera(ControllerType controllerType = ControllerType::NONE);
	
		/**
		 * @brief Adds a new camera object to the #CameraManager and binds it to a camera controller object of specified
		 * @p controllerType.
		 * @param[in] controllerType The type of the camera controller.
		 * @param[in] camera The new camera object.
		 * @return The handle of the newly bound camera object.
		 */
		CameraHandle addCamera(ControllerType controllerType, const Camera& camera);

        /**
         * @brief Returns the stored camera object located by @p cameraHandle.
         * @param[in] cameraHandle The camera handle.
         * @return The camera object by @p cameraHandle.
         * @throws std::runtime_error If @p cameraHandle is not a valid camera handle.
         */
		[[nodiscard]]
        Camera& getCamera(const CameraHandle& cameraHandle);

        /**
         * @brief Returns the stored camera object set as the active camera.
         * @return The active camera.
         */
		[[nodiscard]]
        Camera& getActiveCamera();

        /**
         * @brief Sets the stored camera object located at @p cameraHandle as the active camera.
         * @param[in] cameraHandle The camera handle.
         * @throws std::runtime_error If @p cameraHandle is not a valid camera handle.
         */
        void setActiveCamera(const CameraHandle& cameraHandle);

        /**
         * @brief Returns the handle of the stored active camera object.
         * @return The active camera handle.
         */
		[[nodiscard]]
		CameraHandle getActiveCameraHandle() const;

        /**
         * @brief Binds a stored camera object located by @p cameraHandle to a camera controller of specified
         * @p controllerType.
         * @param[in] cameraHandle The camera handle.
         * @param[in] controllerType The type of the camera controller.
         * @throws std::runtime_error If @p cameraHandle is not a valid camera handle.
         */
        void setControllerType(const CameraHandle& cameraHandle, ControllerType controllerType);

        /**
         * @brief Returns the currently bound camera controller type of the stored camera object located by @p cameraHandle.
         * @param[in] cameraHandle The camera handle.
         * @return The type of the camera controller of the specified camera object.
         * @throws std::runtime_error If @p cameraHandle is not a valid camera handle.
         */
		[[nodiscard]]
        ControllerType getControllerType(const CameraHandle& cameraHandle);

        /**
         * @brief Updates all stored camera controllers in respect to @p deltaTime.
         * @param[in] deltaTime The time that has passed since last update.
         */
        void update(double deltaTime);
    };

    /** @} */

}
