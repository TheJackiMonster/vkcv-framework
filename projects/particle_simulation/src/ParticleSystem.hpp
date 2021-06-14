#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Particle.hpp"

class ParticleSystem {

public:
    const std::vector<Particle> &getParticles() const;
    void addParticle( const Particle particle );
    void addParticles( const std::vector<Particle> particles );

private:
    std::vector<Particle> m_particles;
};