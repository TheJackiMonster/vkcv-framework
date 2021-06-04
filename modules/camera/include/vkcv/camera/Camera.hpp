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

		float m_near;
		float m_far;
		float m_fov;
		float m_ratio;

        glm::vec3 m_up;
        glm::vec3 m_position;
        float m_cameraSpeed;
        float m_pitch;
        float m_yaw;

        int m_fov_nsteps;
        float m_fov_min;
        float m_fov_max;

        bool m_forward;
        bool m_backward;
        bool m_left;
        bool m_right;

    public:
        Camera();

        virtual ~Camera();

        void setPerspective(float fov, float ratio, float near, float far);

        const glm::mat4 getView() const;

        void getView(glm::vec3 &x, glm::vec3 &y, glm::vec3 &z, glm::vec3 &pos);

        glm::mat4 updateView(double deltatime);

        void lookAt(glm::vec3 position, glm::vec3 center, glm::vec3 up);

        const glm::mat4& getProjection() const;

        void setProjection(const glm::mat4 projection);

        void getNearFar(float &near, float &far) const;

        void setUp(const glm::vec3 &Up);

        float getFov() const;

        void setFov(float fov);
        
        void changeFov(double fov);

        void updateRatio(int width, int height);

        float getRatio() const;

        void setNearFar(float near, float far);

        glm::vec3 getFront() const;

        glm::vec3 getPosition() const;

        void setPosition( glm::vec3 position );

        float getPitch() const;

        void setPitch(float pitch);

        float getYaw() const;

        void setYaw(float yaw);

        void panView( double xOffset, double yOffset );

        void updatePosition(double deltatime);

        void moveForward(int action);

        void moveBackward(int action);

        void moveLeft(int action);

        void moveRight(int action);


    };

}
