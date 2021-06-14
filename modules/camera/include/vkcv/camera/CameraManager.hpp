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
         * @brief Adds a new camera object to the #CameraManager.
         * @return The index of the newly created camera object.
         */
        int addCamera();

        // TODO: Add docu!
        int addCamera(ControllerType controllerType);

        /**
         * @brief Gets the stored camera object located at @p cameraIndex.
         * @param cameraIndex The index of the stored camera object.
         * @return The camera object at @p cameraIndex.
         */
        Camera& getCamera(uint32_t cameraIndex);

        // TODO: Add docu!
        Camera& getActiveCamera();

        // TODO: Add docu!
        void setActiveCamera(uint32_t cameraIndex);

        

        // TODO: Add docu!
        uint32_t getActiveCameraIndex();

        // TODO: Add docu!
        void setControllerType(uint32_t cameraIndex, ControllerType controllerType);

        // TODO: Add docu!
        ControllerType getControllerType(uint32_t cameraIndex);

        // TODO: Add docu!
        CameraController& getControllerByType(ControllerType controllerType);

        /**
         * @brief Updates all stored camera controllers in respect to @p deltaTime.
         * @param deltaTime The time that has passed since last update.
         */
        void update(double deltaTime);

    };
}
