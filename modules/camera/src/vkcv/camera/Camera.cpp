#include "vkcv/camera/Camera.hpp"

#include <math.h>

namespace vkcv::camera {

    Camera::Camera() {
        lookAt(
			glm::vec3(0.0f, 0.0f, -1.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
  
		setFront(glm::normalize(m_center - m_position));
    }

    Camera::~Camera() = default;

    void Camera::lookAt(const glm::vec3& position, const glm::vec3& center, const glm::vec3& up) {
    	m_position = position;
    	m_center = center;
    	m_up = up;
    	
		setView(glm::lookAt(position, center, up));
    }

    void Camera::getNearFar( float &near, float &far) const {
        near = m_near;
        far = m_far;
    }

    const glm::mat4& Camera::getView() const {
        return m_view;
    }
    
    void Camera::setView(const glm::mat4 &view) {
		m_view = view;
	}

    const glm::mat4 y_correction(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    glm::mat4 Camera::getProjection() const {
        return y_correction * m_projection;
    }

    void Camera::setProjection(const glm::mat4& projection) {
        m_projection = projection;
    }

    glm::mat4 Camera::getMVP() const {
        return y_correction * m_projection * m_view;
    }

    float Camera::getFov() const {
    	const float tanHalfFovy = 1.0f / m_projection[1][1];
    	float halfFovy = std::atan(tanHalfFovy);
    	
    	if (halfFovy < 0) {
    		halfFovy += static_cast<float>(M_PI);
    	}
    	
        return halfFovy * 2.0f;
    }

    void Camera::setFov( float fov){
        setPerspective(fov, getRatio(), m_near, m_far);
    }

    float Camera::getRatio() const {
    	const float aspectProduct = 1.0f / m_projection[0][0];
		const float tanHalfFovy = 1.0f / m_projection[1][1];
		
        return aspectProduct / tanHalfFovy;
    }

    void Camera::setRatio(float ratio){
        setPerspective( getFov(), ratio, m_near, m_far);
    }

    void Camera::setNearFar(float near, float far){
        setPerspective(getFov(), getRatio(), near, far);
    }

    void Camera::setPerspective(float fov, float ratio, float near, float far) {
		m_near = near;
		m_far = far;
		setProjection(glm::perspective(fov, ratio, near, far));
    }

    glm::vec3 Camera::getFront() const {
        glm::vec3 direction;
        direction.x = std::sin(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
        direction.y = std::sin(glm::radians(m_pitch));
        direction.z = std::cos(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
        return glm::normalize(direction);
    }
    
    void Camera::setFront(const glm::vec3 &front) {
		m_pitch = std::atan2(front.y, std::sqrt(front.x * front.x + front.z * front.z));
		m_yaw = std::atan2(front.x, front.z);
    }

    const glm::vec3& Camera::getPosition() const {
        return m_position;
    }

    void Camera::setPosition( const glm::vec3& position ){
		lookAt(position, m_center, m_up);
    }

    const glm::vec3& Camera::getCenter() const {
        return m_center;
    }

    void Camera::setCenter(const glm::vec3& center) {
		lookAt(m_position, center, m_up);
    }
	
	const glm::vec3& Camera::getUp() const {
		return m_up;
	}
	
	void Camera::setUp(const glm::vec3 &up) {
		lookAt(m_position, m_center, up);
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

}