#pragma once

#include "Camera.hpp"

namespace vkcv {

    class TrackballCamera : public vkcv::Camera {
    protected:
        glm::vec3 m_center;
        float m_radius;
        float m_scrollSensitivity;

    public:

        TrackballCamera();

        ~TrackballCamera();

        float getRadius();

        void setRadius(float radius);

        glm::vec3 &getCenter();

        void setCenter(const glm::vec3 &center);

        void setPitch(float pitch);

        void setYaw(float yaw);

        void changeFov(double fov);

        void panView(double xOffset, double yOffset);

        glm::mat4 updateView(double deltaTime);

        void updatePosition(double deltaTime);
    };

}