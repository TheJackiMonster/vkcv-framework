#include "vkcv/camera/InterpolationLinear.hpp"
#include <vkcv/Logger.hpp>
#include <iostream>


namespace vkcv::camera {

    InterpolationLinear::InterpolationLinear(Camera &camera)
    : m_camera(camera) {
        resetTimer();
    }

    void InterpolationLinear::resetTimer() {
        m_timeSinceStart = glfwGetTime();
    }

    void InterpolationLinear::addPosition(const glm::vec3& pos) {
        m_positions.push_back(pos);
    }

    glm::vec3 interpolate(glm::vec3 posA, glm::vec3 posB, double time, double duration) {
        glm::vec3 posDiff = posB - posA;
        glm::vec3 posInterp = posA + (posDiff * static_cast<float>(time / duration));
        return posInterp;
    }

    void InterpolationLinear::updateCamera() {

        if (m_positions.size() < 2) {
            vkcv_log(LogLevel::ERROR, "At least 2 positions needed for interpolation.");
            return;
        }
        double t = glfwGetTime() - m_timeSinceStart;
        double segmentDuration = 2.0;
        int numberOfSegments = m_positions.size()-1;
        double totalDuration = numberOfSegments * segmentDuration;
        double modt = fmod(t, totalDuration);

        int currentSegment = static_cast<int>((modt / totalDuration) *  numberOfSegments);

        glm::vec3 posInterp = interpolate(m_positions[currentSegment], m_positions[currentSegment+1], fmod(modt, segmentDuration), segmentDuration);

        // const glm::vec3 front = m_camera.getFront();
		// const glm::vec3 up = m_camera.getUp();

        // m_camera.lookAt(posInterp, front+posInterp, up);

        m_camera.setPosition(posInterp); // always look at center
    }
}