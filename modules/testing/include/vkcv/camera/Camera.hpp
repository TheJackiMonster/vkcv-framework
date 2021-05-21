#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>

namespace vkcv {

    class Camera {
    protected:
        GLFWwindow *m_window;

        glm::mat4 m_view, m_projection;

        int m_width, m_height;

        float m_oldX, m_oldY;

        glm::vec3 m_position, m_direction, m_up;

        float m_near, m_far;
        float m_fov, m_ratio;

    public:
        Camera();

        ~Camera();

        virtual void update(GLFWwindow* window) {};

        void setPerspective(float fov, float ratio, float near, float far);

        const glm::mat4& getView();

        void getView(glm::vec3 &x, glm::vec3 &y, glm::vec3 &z, glm::vec3 &pos);

        void setView( const glm::mat4 view );

        void lookAt(glm::vec3 position, glm::vec3 center, glm::vec3 up);

        const glm::mat4& Camera::getProjection();

        void setProjection( const glm::mat4 projection);

        void getNearFar( float &near, float &far);

        float getFov();

        void setFov( float fov);

        void updateRatio( float ratio);

        float getRatio();

        void setNearFar( float near, float far);


    };

}
