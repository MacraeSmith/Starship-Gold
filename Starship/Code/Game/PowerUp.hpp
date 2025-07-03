#pragma once
#include "Game/Entity.hpp"

enum class PowerUpTypes
{
	TRI_BULLET,
	FIVE_BULLET,
	BURST_BULLET,
	SNIPER_BULLET,
	SHIELD,
	HEALTH,
	NUM_POWERUP_TYPES,
};

constexpr int NUM_POWERUP_TRIS = 2;
constexpr int NUM_POWERUP_VERTS = 3 * NUM_POWERUP_TRIS;
class PowerUp : public Entity
{
public:
	explicit PowerUp(Game* owner, Vec2 const& startingPosition, float orientationDeg);
	~PowerUp() {};

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void Die() override;

private:
	virtual void InitializeLocalVerts();
	void ChooseRandomPowerUp();

public:
	PowerUpTypes m_powerupType;

private:
	Vertex_PCU m_localVerts[NUM_POWERUP_VERTS];
	float m_textOffset = 0.f;
};

