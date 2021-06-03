#pragma once

#include "Camera.hpp"

namespace vkcv {

    class TrackballCamera : public vkcv::Camera {
    protected:
        glm::vec3 m_center;
        float m_radius;
        float m_scrollSensitivity;

    public:

        /**
         * @brief The default constructor of the trackball camera
         */
        TrackballCamera();

        /**
         * @brief The destructor of the trackball camera (default behavior)
         */
        ~TrackballCamera();

        /**
         * @brief Gets the radius of the trackball camera that specifies the distance of the trackball camera to the center point
         * @return The radius of the trackball camera
         */
        float getRadius() const;

        /**
         * @brief Sets the current radius of the trackball camera to @p radius
         * @param[in] radius The new radius of the trackball camera
         */
        void setRadius( const float radius);

        /**
         * @brief Gets the center point the trackball camera is looking at
         * @return The center point of the trackball camera
         */
        const glm::vec3& getCenter();

        /**
         * @brief Sets the current center point of the trackball camera to @p center
         * @param[in] center The new center point of the trackball camera
         */
        void setCenter(const glm::vec3 &center);

        /**
         * @brief Sets the pitch value of the trackball camera to @p pitch
         * @param[in] pitch The new pitch value of the trackball camera
         */
        void setPitch(float pitch);

        /**
         * @brief Sets the yaw value of the trackball camera to @p yaw
         * @param[in] yaw The new yaw value of the trackball camera
         */
        void setYaw(float yaw);

        /**
         * @brief Changes the field of view of the trackball camera with an @p offset in degrees
         * @param[in] offset The offset in degrees
         */
        void changeFov(double offset);

        /**
         * @brief Pans the view of the trackball camera according to the pitch and yaw values and additional offsets @p xOffset and @p yOffset (e.g. taken from mouse movement)
         * @param[in] xOffset The offset added to the yaw value
         * @param[in] yOffset The offset added to the pitch value
         */
        void panView(double xOffset, double yOffset);

        /**
         * @brief Updates the view matrix of the trackball camera with respect to @p deltatime
         * @param deltaTime The time that has passed since last update
         * @return The updated view matrix of the trackball camera
         */
        glm::mat4 updateView(double deltaTime);

        /**
         * @brief Updates the position of the trackball camera with respect to @p deltaTime
         * @param[in] deltaTime The time that has passed since last update
         */
        void updatePosition(double deltaTime);
    };

}