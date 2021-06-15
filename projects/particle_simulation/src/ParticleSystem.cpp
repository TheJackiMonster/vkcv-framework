#include "ParticleSystem.hpp"

ParticleSystem::ParticleSystem(uint32_t particleCount ,glm::vec3 minVelocity , glm::vec3 maxVelocity , glm::vec2 lifeTime )
{
    m_rdmVel.resize(3);
    m_rdmVel[0] = std::uniform_real_distribution<float>(minVelocity.x, maxVelocity.x);
    m_rdmVel[1] = std::uniform_real_distribution<float>(minVelocity.y, maxVelocity.y);
    m_rdmVel[2] = std::uniform_real_distribution<float>(minVelocity.z, maxVelocity.z);
    m_rdmLifeTime = std::uniform_real_distribution<float>(lifeTime.x, lifeTime.y);

    for(uint32_t i = 0; i < particleCount ;i++ ){
        addParticle(Particle(m_respawnPos, getRandomVelocity(), getRandomLifeTime()));
    }
}

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
        particle.setVelocity( particle.getVelocity() * static_cast<float>(alive) + static_cast<float>(!alive) *  getRandomVelocity());
        particle.setLifeTime( (particle.getLifeTime() * alive + !alive * getRandomLifeTime() ) - deltaTime );
        particle.update(deltaTime);
    }
}

glm::vec3 ParticleSystem::getRandomVelocity(){
    return glm::vec3(m_rdmVel[0](m_rdmEngine), m_rdmVel[1](m_rdmEngine),m_rdmVel[2](m_rdmEngine));
}

float ParticleSystem::getRandomLifeTime(){
    return m_rdmLifeTime(m_rdmEngine);
}

void ParticleSystem::setRespawnPos( const glm::vec3 respawnPos){
    m_respawnPos = respawnPos;
}
void ParticleSystem::setRdmLifeTime( const glm::vec2 lifeTime ){
    m_rdmLifeTime = std::uniform_real_distribution<float> (lifeTime.x,lifeTime.y);
}

void ParticleSystem::setRdmVelocity( glm::vec3 minVelocity, glm::vec3 maxVelocity ){
    m_rdmVel[0] = std::uniform_real_distribution<float> (minVelocity.x,maxVelocity.x);
    m_rdmVel[1] = std::uniform_real_distribution<float> (minVelocity.y,maxVelocity.y);
    m_rdmVel[2] = std::uniform_real_distribution<float> (minVelocity.z,maxVelocity.z);
}
