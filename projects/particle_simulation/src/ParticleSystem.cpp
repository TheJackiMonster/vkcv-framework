#include "ParticleSystem.hpp"

const std::vector<Particle>& ParticleSystem::getParticles() const {
    return m_particles;
}

void ParticleSystem::addParticle( const Particle particle ){
    m_particles.push_back(particle);
}
void ParticleSystem::addParticles( const std::vector<Particle> particles ){
    m_particles.insert(m_particles.end(), particles.begin(), particles.end());
}