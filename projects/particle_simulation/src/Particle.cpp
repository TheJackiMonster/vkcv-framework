
#include "Particle.hpp"

Particle::Particle(glm::vec3 position, glm::vec3 velocity, float lifeTime)
: m_position(position),
  m_lifeTime(lifeTime),
  m_velocity(velocity),
  m_mass(1.0f),
  m_reset_velocity(velocity)
{}

const glm::vec3& Particle::getPosition() const {
    return m_position;
}

bool Particle::isAlive() const {
    return m_lifeTime > 0.f;
}

void Particle::setPosition(const glm::vec3& pos) {
    m_position = pos;
}

const glm::vec3& Particle::getVelocity() const {
    return m_velocity;
}

void Particle::setVelocity(const glm::vec3& vel) {
    m_velocity = vel;
}

void Particle::update(float delta) {
    m_position += m_velocity * delta;
}

void Particle::setLifeTime(float lifeTime) {
    m_lifeTime = lifeTime;
}

float Particle::getLifeTime() const {
    return m_lifeTime;
}