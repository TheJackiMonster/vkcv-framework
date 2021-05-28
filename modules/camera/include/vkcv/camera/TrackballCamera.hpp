#pragma once

#include "Camera.hpp"

namespace vkcv {

    class TrackballCamera : public vkcv::Camera {
    public:

        TrackballCamera( int width, int height, glm::mat4 projection);

        TrackballCamera(int width, int height);

        ~TrackballCamera();

        float getSensitivity() const;

        void setSensitivity(float sensitivity);

        float getStepSize() const;

        void setStepSize(float stepSize);

        float getTheta() const;

        void setTheta(float theta);

        float getPhi() const;

        void setPhi(float phi);

        float getRadius() const;

        void setRadius(float radius);

        const glm::vec3& getCenter() const;

        void setCenter(const glm::vec3 &center);

    private:
        float m_sensitivity;
        float m_stepSize, m_theta, m_phi, m_radius;
        glm::vec3 m_center;

        float m_oldX;
        float m_oldY;
    };

}