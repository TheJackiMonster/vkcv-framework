#include "vkcv/camera/TrackballCamera.hpp"

namespace vkcv{


    TrackballCamera::TrackballCamera(int width, int height)
    {
        m_position = glm::vec3(0.0f, 0.0f, 5.0);
        m_center = glm::vec3( 0.0f, 0.0f, 0.0f);
        m_up = glm::vec3(0.0f, 1.0f, 0.0f);

        m_width = width;
        m_height = height;

        m_sensitivity = 0.010f;
        m_stepSize = 0.1f;
        m_theta = glm::pi<float>() / 2.0f;
        m_phi = 0.f;
        m_radius = 1.5;

        m_view = glm::lookAt( m_center + m_position, m_center, m_up);

        m_oldX = width/2.f;
        m_oldY = height/2.f;

        m_fov = glm::radians(60.f);
        m_ratio = m_width / (float) m_height;
        m_near = 0.001f;
        m_far = 10.f;
        glm::mat4 projection = glm::perspective(m_fov, m_ratio, m_near, m_far);

        setProjection(projection);
    }

    void TrackballCamera::update( GLFWwindow* window) {

    }

    float TrackballCamera::getSensitivity() const {
        return m_sensitivity;
    }

    void TrackballCamera::setSensitivity(float sensitivity) {
        m_sensitivity = sensitivity;
    }

    float TrackballCamera::getStepSize() const {
        return m_stepSize;
    }

    void TrackballCamera::setStepSize(float stepSize) {
        m_stepSize = stepSize;
    }

    float TrackballCamera::getTheta() const {
        return m_theta;
    }

    void TrackballCamera::setTheta(float theta) {
        m_theta = theta;
    }

    float TrackballCamera::getPhi() const {
        return m_phi;
    }

    void TrackballCamera::setPhi(float phi) {
        m_phi = phi;
    }

    float TrackballCamera::getRadius() const {
        return m_radius;
    }

    void TrackballCamera::setRadius(float radius) {
        m_radius = radius;
    }

    const glm::vec3& TrackballCamera::getCenter() const {
        return m_center;
    }

    void TrackballCamera::setCenter(const glm::vec3 &center) {
        m_center = center;
    }
}