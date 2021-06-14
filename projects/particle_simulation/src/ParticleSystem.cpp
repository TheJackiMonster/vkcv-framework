#include "ParticleSystem.hpp"

const std::vector<Particle>& ParticleSystem::getParticles() const{
    return m_particles;
}

void ParticleSystem::addParticle( const Particle particle ){
    m_particles.push_back(particle);
}
void ParticleSystem::addParticles( const std::vector<Particle> particles ){
    m_particles.insert(m_particles.end(), particles.begin(), particles.end());
}

void ParticleSystem::updateParticles( const float deltaTime ){
    for(Particle& particle :m_particles){
        bool alive = particle.isAlive();
        particle.setPosition( particle.getPosition() * static_cast<float>(alive) + static_cast<float>(!alive) * m_respawnPos );
        particle.setLifeTime( (particle.getLifeTime() * alive + !alive * m_maxLifeTime ) - deltaTime );
        particle.update(deltaTime);
    }
}

void ParticleSystem::setRespawnPos( const glm::vec3 respawnPos){
    m_respawnPos = respawnPos;
}
void ParticleSystem::setMaxLifeTime( const float respawnTime ){
    m_maxLifeTime = respawnTime;
}
