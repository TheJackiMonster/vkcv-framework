#include "vkcv/camera/Camera.hpp"
#include <iostream>

namespace vkcv {

    Camera::Camera() {
        m_position = glm::vec3(0.0f, 0.0f, -1.0f);
        m_up = glm::vec3(0.0f, 1.0f, 0.0f);
        m_center = glm::vec3(0.0f, 0.0f, 0.0f);
        lookAt(m_position, m_center, m_up);
        glm::vec3 front = glm::normalize(m_center - m_position);
        m_pitch = std::atan2(front.y, std::sqrt(front.x * front.x + front.z * front.z));
        m_yaw = std::atan2(front.x, front.z);
    }

    Camera::~Camera() = default;

    void Camera::lookAt(glm::vec3 position, glm::vec3 center, glm::vec3 up){
		m_position = position;
		m_center = center;
		m_up = up;
        m_view = glm::lookAt(m_position, m_center, m_up);
    }

    void Camera::getNearFar( float &near, float &far) const {
        near = m_near;
        far = m_far;
    }

    glm::mat4& Camera::getView() {
        return m_view;
    }

    glm::mat4& Camera::getProjection() {
        return m_projection;
    }

    void Camera::setProjection(const glm::mat4 projection){
        m_projection = projection;
    }

    glm::mat4 Camera::getMVP() const {
        return m_projection * m_view;
    }

    float Camera::getFov() const {
        return m_fov;
    }

    void Camera::setFov( float fov){
        setPerspective(fov, m_ratio, m_near, m_far);
    }

    float Camera::getRatio() const {
        return m_ratio;
    }

    void Camera::setRatio(float ratio){
        setPerspective( m_fov, ratio, m_near, m_far);
    }

    void Camera::setNearFar(float near, float far){
        setPerspective(m_fov, m_ratio, near, far);
    }

    void Camera::setPerspective(float fov, float ratio, float near, float far) {
    	const glm::mat4 y_correction (
    			1.0f,  0.0f,  0.0f,  0.0f,
    			0.0f, -1.0f,  0.0f,  0.0f,
    			0.0f,  0.0f,  1.0f,  0.0f,
    			0.0f,  0.0f,  0.0f,  1.0f
		);
    	
        m_fov = fov;
        m_ratio = ratio;
        m_near = near;
        m_far = far;
        m_projection = y_correction * glm::perspective(m_fov, m_ratio, m_near, m_far);
    }

    glm::vec3 Camera::getFront() const {
        glm::vec3 direction;
        direction.x = std::sin(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
        direction.y = std::sin(glm::radians(m_pitch));
        direction.z = std::cos(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
        return glm::normalize(direction);
    }

    glm::vec3 Camera::getPosition() const {
        return m_position;
    }

    void Camera::setPosition( glm::vec3 position ){
        m_position = position;
    }

    glm::vec3 Camera::getCenter() const {
        return m_center;
    }

    void Camera::setCenter(glm::vec3 center) {
        m_center = center;
    }

    float Camera::getPitch() const {
        return m_pitch;
    }

    void Camera::setPitch(float pitch) {
        m_pitch = pitch;
    }

    float Camera::getYaw() const {
        return m_yaw;
    }

    void Camera::setYaw(float yaw) {
        m_yaw = yaw;
    }

    glm::vec3 Camera::getUp() const {
        return m_up;
    }

    void Camera::setUp(const glm::vec3 &up) {
        m_up = up;
    }

}