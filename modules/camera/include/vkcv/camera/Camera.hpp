#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>

namespace vkcv {

    class Camera {
    protected:
		glm::mat4 m_view;
		glm::mat4 m_projection;

		int m_width;
		int m_height;

		float m_oldX;
		float m_oldY;
		float m_near;
		float m_far;
		float m_fov;
		float m_ratio;

        glm::vec3 m_up;
        glm::vec3 m_position;
        float m_cameraSpeed;
        float m_roll;
        float m_pitch;
        float m_yaw;

        bool m_forward;
        bool m_backward;
        bool m_left;
        bool m_right;

    public:
        Camera();

        ~Camera();

        void setPerspective(float fov, float ratio, float near, float far);

        const glm::mat4 getView();

        void getView(glm::vec3 &x, glm::vec3 &y, glm::vec3 &z, glm::vec3 &pos);

        glm::mat4 updateView();

        void lookAt(glm::vec3 position, glm::vec3 center, glm::vec3 up);

        const glm::mat4& getProjection();

        void setProjection(const glm::mat4 projection);

        void getNearFar(float &near, float &far);

        void setUp(const glm::vec3 &Up);

        float getFov();

        void setFov(float fov);

        void updateRatio(float ratio);

        float getRatio();

        void setNearFar(float near, float far);

        glm::vec3 getFront();

        glm::vec3 getPosition();

        void setPosition( glm::vec3 position );

        float getPitch() const;

        void setPitch(float pitch);

        float getYaw() const;

        void setYaw(float yaw);

        void panView( double xOffset, double yOffset );

        void updatePosition();

        void moveForward(int action);

        void moveBackward(int action);

        void moveLeft(int action);

        void moveRight(int action);


    };

}
