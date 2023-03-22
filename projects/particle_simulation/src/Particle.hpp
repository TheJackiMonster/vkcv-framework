#pragma once

#include <glm/glm.hpp>

class Particle {

public:
    Particle(glm::vec3 position, glm::vec3 velocity, float lifeTime = 1.f);
	
	[[nodiscard]]
    const glm::vec3& getPosition() const;

    void setPosition(const glm::vec3& pos);
	
	[[nodiscard]]
    const glm::vec3& getVelocity() const;

    void setVelocity(const glm::vec3& vel);

    void update(float delta);

    [[nodiscard]]
	bool isAlive() const;

    void setLifeTime(float lifeTime);
	
	[[nodiscard]]
	float getLifeTime() const;

private:
    // all properties of the Particle
    glm::vec3 m_position;
    float m_lifeTime;
    glm::vec3 m_velocity;
	[[maybe_unused]] float m_mass;
	[[maybe_unused]] glm::vec3 m_reset_velocity;
	[[maybe_unused]] float m_padding;
};
