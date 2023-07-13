#pragma once

#include <glm/glm.hpp>

class Particle {

public:
    Particle(glm::vec3 position, glm::vec3 velocity);

    const glm::vec3& getPosition()const;

    void setPosition( const glm::vec3 pos );

    const glm::vec3& getVelocity()const;

    void setVelocity( const glm::vec3 vel );

    const float& getDensity()const;

    const glm::vec3& getForce()const;

    const float& getPressure()const;



private:
    // all properties of the Particle
    glm::vec3 m_position;
	[[maybe_unused]] [[maybe_unused]] float m_padding;
    glm::vec3 m_velocity;
    float m_density;
    glm::vec3 m_force;
    float m_pressure;
};
