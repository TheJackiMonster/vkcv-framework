#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Particle{
    Particle(glm::vec3 position, glm::vec3 velocity)
    noexcept
    : m_position(position),
    m_velocity(velocity)
    {}

    // all properties of the Particle
    glm::vec3 m_position;
    float padding = 0.f;
    glm::vec3 m_velocity;
    float padding_2 = 0.f;
};

class ParticleSystem {

public:
    const std::vector<Particle> &getParticles() const;
    void addParticle( const Particle particle );
    void addParticles( const std::vector<Particle> particles );

private:
    std::vector<Particle> m_particles;

};
