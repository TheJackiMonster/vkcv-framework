#pragma once

#include "PilotCameraController.hpp"
#include "TrackballCameraController.hpp"
#include "CameraController.hpp"
#include "vkcv/Window.hpp"
#include <GLFW/glfw3.h>
#include <functional>

namespace vkcv {

    /**
     * @brief Used for specifying existing types of camera controllers when adding a new controller object to the
     * #CameraManager.
     */
    enum class ControllerType {
        NONE,
        PILOT,
        TRACKBALL,
        TRACE
    };

    /**
     * @brief Used for managing an arbitrary amount of camera controllers.
     */
    class CameraManager{
    private:
        std::function<void(int, int, int, int)> m_keyHandle;
        std::function<void(double, double)> m_mouseMoveHandle;
        std::function<void(double, double)> m_mouseScrollHandle;
        std::function<void(int, int, int)> m_mouseButtonHandle;
        std::function<void(int, int)> m_resizeHandle;

        Window& m_window;
        std::vector<Camera> m_cameras;
        std::vector<ControllerType> m_cameraControllerTypes;
        uint32_t m_activeCameraIndex;

        PilotCameraController m_pilotController;
        TrackballCameraController m_trackController;

        double m_lastX;
        double m_lastY;

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


    public:

        /**
         * @brief The constructor of the #CameraManager.
         * @param[in] window The window.
         * @param[in] width The width of the window.
         * @param[in] height The height of the window.
         */
        CameraManager(Window &window, float width, float height);

        /**
         * @brief The destructor of the #CameraManager. Destroying the #CameraManager leads to deletion of all stored
         * camera objects and camera controller objects.
         */
        ~CameraManager();

        /**
         * @brief Adds a new camera object to the #CameraManager. The camera is not binded to a controller type.
         * @return The index of the newly created camera object.
         */
        int addCamera();

        /**
         * @brief Adds a new camera object to the #CameraManager and binds it to a camera controller object of specified
         * @p controllerType.
         * @param controllerType The type of the camera controller
         * @return The index of the newly created camera object.
         */
        int addCamera(ControllerType controllerType);

        /**
         * @brief Gets the stored camera object located at @p cameraIndex.
         * @param cameraIndex The camera index.
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
         * @param cameraIndex The camera index.
         * @throws std::runtime_error If @p cameraIndex is not a valid camera index.
         */
        void setActiveCamera(uint32_t cameraIndex);

        /**
         * @brief Gets the index of the stored active camera object.
         * @return The active camera index.
         */
        uint32_t getActiveCameraIndex();

        /**
         * @brief Binds a stored camera object located at @p cameraIndex to a camera controller of specified
         * @p controllerType.
         * @param cameraIndex The camera index.
         * @param controllerType The type of the camera controller.
         * @throws std::runtime_error If @p cameraIndex is not a valid camera index.
         */
        void setControllerType(uint32_t cameraIndex, ControllerType controllerType);

        /**
         * @brief Gets the currently bound camera controller type of the stored camera object located at @p cameraIndex.
         * @param cameraIndex The camera index.
         * @return The type of the camera controller of the specified camera object.
         * @throws std::runtime_error If @p cameraIndex is not a valid camera index.
         */
        ControllerType getControllerType(uint32_t cameraIndex);

        /**
         * @brief Gets a camera controller object of specified @p controllerType.
         * @param controllerType The type of the camera controller.
         * @return The specified camera controller object.
         */
        CameraController& getControllerByType(ControllerType controllerType);

        /**
         * @brief Updates all stored camera controllers in respect to @p deltaTime.
         * @param deltaTime The time that has passed since last update.
         */
        void update(double deltaTime);

    };
}
