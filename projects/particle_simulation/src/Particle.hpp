#pragma once

class Particle {

public:
    Particle(glm::vec3 position, glm::vec3 velocity)
    noexcept
            : m_position(position),
              m_velocity(velocity) {}

    const glm::vec3& getPosition()const{
        return m_position;
    };
private:
    // all properties of the Particle
    glm::vec3 m_position;
    float padding = 0.f;
    glm::vec3 m_velocity;
    float padding_2 = 0.f;
};