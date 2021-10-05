
#include "Particle.hpp"

Particle::Particle(glm::vec3 position, glm::vec3 velocity)
: m_position(position),
  m_velocity(velocity)
{
    m_density = 0.f;
    m_force = glm::vec3(0.f);
    m_pressure = 0.f;
}

const glm::vec3& Particle::getPosition()const{
    return m_position;
}

void Particle::setPosition( const glm::vec3 pos ){
    m_position = pos;
}

const glm::vec3& Particle::getVelocity()const{
    return m_velocity;
}

void Particle::setVelocity( const glm::vec3 vel ){
    m_velocity = vel;
}

const float& Particle::getDensity()const {
    return m_density;
}

const glm::vec3& Particle::getForce()const {
    return m_force;
}

const float& Particle::getPressure()const {
    return m_pressure;
}