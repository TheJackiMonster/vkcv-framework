#include "vkcv/camera/Camera.hpp"
#include <iostream>

namespace vkcv {

    Camera::Camera(){
        m_up = glm::vec3(0.0f, -1.0f, 0.0f);
        m_position = glm::vec3(0.0f, 0.0f, 0.0f);
        m_cameraSpeed = 2.f;

        m_pitch = 0.0;
        m_yaw = 180.0;

        m_fov_nsteps = 100;
        m_fov_min = 10;
        m_fov_max = 120;

        m_forward = false;
        m_backward = false;
        m_left = false;
        m_right = false;
    }

    Camera::~Camera() = default;

    void Camera::lookAt(glm::vec3 position, glm::vec3 center, glm::vec3 up){
        m_view = glm::lookAt(position, center, up);
    }

    glm::mat4 Camera::updateView(double deltaTime){
        updatePosition(deltaTime);
        return m_view = glm::lookAt(m_position, m_position + getFront() , m_up);
    }

    void Camera::getView(glm::vec3 &x, glm::vec3 &y, glm::vec3 &z, glm::vec3 &pos){
        x = glm::vec3( glm::row(m_view, 0));
        y = glm::vec3( glm::row(m_view, 1));
        z = glm::vec3( glm::row(m_view, 2));
        pos = glm::vec3( glm::column(m_view, 3));
        glm::mat3 mat_inv = glm::inverse( glm::mat3(m_view));
        pos = -mat_inv * pos;
    }

    void Camera::getNearFar( float &near, float &far) const {
        near = m_near;
        far = m_far;
    }


    const glm::mat4 Camera::getView() const {
        return m_view;
    }

    const glm::mat4& Camera::getProjection() const {
        return m_projection;
    }

    void Camera::setProjection(const glm::mat4 projection){
        m_projection = projection;
    }

    float Camera::getFov() const {
        return m_fov;
    }

    void Camera::setFov( float fov){
        m_fov = fov;
        setPerspective( m_fov, m_ratio, m_near, m_far);
    }

    void Camera::changeFov(double offset){
        float fov = m_fov;
        float fov_range = m_fov_max - m_fov_min;
        float fov_stepsize = glm::radians(fov_range)/m_fov_nsteps;
        fov -= (float) offset*fov_stepsize;
        if (fov < glm::radians(m_fov_min)) {
            fov = glm::radians(m_fov_min);
        }
        if (fov > glm::radians(m_fov_max)) {
            fov = glm::radians(m_fov_max);
        }
        setFov(fov);
    }

    void Camera::updateRatio( float ratio){
        m_ratio = ratio;
        setPerspective( m_fov, m_ratio, m_near, m_far);
    }

    float Camera::getRatio() const {
        return 0.0f;
    }

    void Camera::setNearFar( float near, float far){
        m_near = near;
        m_far = far;
        setPerspective(m_fov, m_ratio, m_near, m_far);
    }

    void Camera::setPerspective(float fov, float ratio, float near, float far){
        m_fov = fov;
        m_ratio = ratio;
        m_near = near;
        m_far = far;
        m_projection = glm::perspective( m_fov, ratio, m_near, m_far);
    }

    glm::vec3 Camera::getFront() const {
        glm::vec3 direction;
        direction.x = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        direction.y = sin(glm::radians(m_pitch));
        direction.z = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        return glm::normalize(direction);
    }

    glm::vec3 Camera::getPosition() const {
        return m_position;
    }

    void Camera::setPosition( glm::vec3 position ){
        m_position = position;
    }

    void Camera::setUp(const glm::vec3 &up) {
        m_up = up;
    }

    float Camera::getPitch() const {
        return m_pitch;
    }

    void Camera::setPitch(float pitch) {
        if (pitch > 89.0f) {
            pitch = 89.0f;
        }
        if (pitch < -89.0f) {
            pitch = -89.0f;
        }
        m_pitch = pitch;
    }

    float Camera::getYaw() const {
        return m_yaw;
    }

    void Camera::setYaw(float yaw) {
        if (yaw < 0.0f) {
            yaw += 360.0f;
        }
        else if (yaw > 360.0f) {
            yaw -= 360.0f;
        }
        m_yaw = yaw;
    }

    void Camera::panView(double xOffset, double yOffset) {
        setYaw(m_yaw + xOffset);
        setPitch(m_pitch + yOffset);
    }

    void Camera::updatePosition(double deltaTime ){
        m_position += (m_cameraSpeed * getFront() * static_cast<float> (m_forward) * static_cast<float>(deltaTime));
        m_position -= (m_cameraSpeed * getFront() * static_cast<float> (m_backward) * static_cast<float>(deltaTime));
        m_position -= (glm::normalize(glm::cross(getFront(), m_up)) * m_cameraSpeed * static_cast<float> (m_left) * static_cast<float>(deltaTime));
        m_position += (glm::normalize(glm::cross(getFront(), m_up)) * m_cameraSpeed * static_cast<float> (m_right) * static_cast<float>(deltaTime));
    }

    void Camera::moveForward(int action){
        m_forward = static_cast<bool>(action);
    }

    void Camera::moveBackward(int action){
        m_backward = static_cast<bool>(action);
    }

    void Camera::moveLeft(int action){
        m_left = static_cast<bool>(action);
    }

    void Camera::moveRight(int action){
        m_right = static_cast<bool>(action);
    }
}