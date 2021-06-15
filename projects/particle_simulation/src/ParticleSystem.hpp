#pragma once

#include <vector>
#include "Particle.hpp"
#include <random>

class ParticleSystem {

public:
    ParticleSystem(uint32_t particleCount , glm::vec3 minVelocity = glm::vec3(0.f,0.f,0.f), glm::vec3 maxVelocity = glm::vec3(1.f,1.f,0.f), glm::vec2 lifeTime = glm::vec2(2.f,3.f));
    const std::vector<Particle> &getParticles() const;
    void updateParticles( const float deltaTime );
    void setRespawnPos( const glm::vec3 respawnPos );
    void setRdmLifeTime( const glm::vec2 lifeTime );
    void setRdmVelocity( glm::vec3 minVelocity, glm::vec3 maxVelocity );
private:

    void addParticle( const Particle particle );
    void addParticles( const std::vector<Particle> particles );
    glm::vec3 getRandomVelocity();
    float getRandomLifeTime();

    std::vector<Particle> m_particles;
    glm::vec3 m_respawnPos = glm::vec3(0.f);

    std::vector<std::uniform_real_distribution<float>> m_rdmVel;
    std::uniform_real_distribution<float> m_rdmLifeTime;
    std::default_random_engine m_rdmEngine;
};