#include "vkcv/camera/TrackballCamera.hpp"

namespace vkcv {

    TrackballCamera::TrackballCamera() {
        m_pitch = 90.0f;
        m_yaw = 0.0f;
        m_radius = 1.5f;

        m_center = glm::vec3(0.0f,0.0f,0.0f);
    }

    TrackballCamera::~TrackballCamera() = default;

    float TrackballCamera::getRadius() {
        return m_radius;
    }

    void TrackballCamera::setRadius(float radius) {
        if (m_radius < 0.1f) {
            m_radius = 0.1f;
        }
        m_radius = radius;
    }

    glm::vec3& TrackballCamera::getCenter() {
        return m_center;
    }

    void TrackballCamera::setCenter(const glm::vec3 &center) {
        m_center = center;
    }

    void TrackballCamera::setPitch(float pitch) {
        if (pitch > 89.0f) {
            pitch = 89.0f;
        }
        if (pitch < -89.0f) {
            pitch = -89.0f;
        }
        m_pitch = pitch;
    }

    void TrackballCamera::setYaw(float yaw) {
        if (m_yaw < 0.0f) {
            m_yaw += 360.0f;
        }
        else if (m_yaw > 360.0f) {
            m_yaw -= 360.0f;
        }
        m_yaw = yaw;
    }

    glm::mat4 TrackballCamera::updateView(double deltaTime)  {
        updatePosition(deltaTime);
        return m_view = glm::lookAt(m_position, m_center, m_up);
    }

    // TODO: deal with gimbal lock problem, e.g. using quaternions, angleaxis etc.
    void TrackballCamera::updatePosition(double deltaTime) {
        m_position.x = m_center.x + m_radius * sin(glm::radians(m_pitch)) * sin(glm::radians(m_yaw));
        m_position.y = m_center.y + m_radius * cos(glm::radians(m_pitch));
        m_position.z = m_center.z + m_radius * sin(glm::radians(m_pitch)) * cos(glm::radians(m_yaw));
    }

}