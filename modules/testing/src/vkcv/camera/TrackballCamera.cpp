#include "vkcv/camera/TrackballCamera.hpp"

namespace vkcv{

    TrackballCamera::TrackballCamera( int width, int height, glm::mat4 projection)
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

        setProjection(projection);
    }


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

    TrackballCamera::~TrackballCamera()
    {
    }


    void TrackballCamera::update( GLFWwindow* window) {

        double x, y;

        glfwGetCursorPos( window, &x, &y);
        if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            float changeX = ((float) x - m_oldX) * m_sensitivity;
            float changeY = ((float) y - m_oldY) * m_sensitivity;

            m_theta -= changeY;
            if (m_theta < 0.01f) m_theta = 0.01f;
            else if (m_theta > glm::pi<float>() - 0.01f) m_theta = glm::pi<float>() - 0.01f;

            m_phi -= changeX;
            if (m_phi < 0) m_phi += 2*glm::pi<float>();
            else if (m_phi > 2*glm::pi<float>()) m_phi -= 2*glm::pi<float>();
        }

        m_oldX = (float) x;
        m_oldY = (float) y;

        if (glfwGetKey( window, GLFW_KEY_UP) == GLFW_PRESS)
            m_radius -= m_stepSize;
        if (glfwGetKey( window, GLFW_KEY_DOWN) == GLFW_PRESS)
            m_radius += m_stepSize;
        if (m_radius < 0.1f) m_radius = 0.1f;

        m_position.x = m_center.x + m_radius * sin(m_theta) * sin(m_phi);
        m_position.y = m_center.y + m_radius * cos(m_theta);
        m_position.z = m_center.z + m_radius * sin(m_theta) * cos(m_phi);

        m_view = glm::lookAt( m_position, m_center, m_up);

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