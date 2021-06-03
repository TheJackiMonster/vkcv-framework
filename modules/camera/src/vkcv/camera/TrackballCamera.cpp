#include "vkcv/camera/TrackballCamera.hpp"
#include <iostream>

namespace vkcv {

    TrackballCamera::TrackballCamera() {
        m_pitch = 0.0f;
        m_yaw = 0.0f;
        m_radius = 1.5f;
        m_center = glm::vec3(0.0f,0.0f,0.0f);
        m_cameraSpeed = 5.0f;
        m_scrollSensitivity = 0.05f;
    }

    TrackballCamera::~TrackballCamera() = default;

    float TrackballCamera::getRadius() const{
        return m_radius;
    }

    void TrackballCamera::setRadius( const float radius) {
        if (m_radius < 0.1f) {
            m_radius = 0.1f;
        }
        m_radius = radius;
    }

    const glm::vec3& TrackballCamera::getCenter() {
        return m_center;
    }

    void TrackballCamera::setCenter(const glm::vec3 &center) {
        m_center = center;
    }

    void TrackballCamera::setPitch(float pitch) {
        if (pitch < 0.0f) {
            pitch += 360.0f;
        }
        else if (pitch > 360.0f) {
            pitch -= 360.0f;
        }
        m_pitch = pitch;
    }

    void TrackballCamera::setYaw(float yaw) {
        if (yaw < 0.0f) {
            yaw += 360.0f;
        }
        else if (yaw > 360.0f) {
            yaw -= 360.0f;
        }
        m_yaw = yaw;
    }

    void TrackballCamera::changeFov(double offset) {
        setRadius(m_radius - offset * m_scrollSensitivity);
    }

    void TrackballCamera::panView(double xOffset, double yOffset) {
        setYaw(m_yaw + xOffset * m_cameraSpeed);
        setPitch(m_pitch + yOffset * m_cameraSpeed);
    }

    glm::mat4 TrackballCamera::updateView(double deltaTime)  {
        updatePosition(deltaTime);
        return m_view = glm::lookAt(m_position, m_center, m_up);
    }

    void TrackballCamera::updatePosition(double deltaTime) {
        glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(m_yaw), glm::vec3(0.0f, -1.0f, 0.0f));
        glm::mat4 rotationX = glm::rotate(rotationY, -glm::radians(m_pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec3 translate = glm::vec3(0.0f,0.0f,m_radius);
        translate = glm::vec3(rotationX * glm::vec4(translate, 0.0f));
        m_position = m_center + translate;
        m_up = glm::vec3(rotationX * glm::vec4(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f));
    }
}