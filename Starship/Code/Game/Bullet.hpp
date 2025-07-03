#pragma once
#include "Game/Entity.hpp"

constexpr int NUM_BULLET_TRIS = 2;
constexpr int NUM_BULLET_VERTS = 3 * NUM_BULLET_TRIS;
class PlayerShip;
enum class PowerUpTypes;
class Bullet : public Entity
{
public:
	explicit Bullet(Game* owner, Vec2 const& startingPosition, float orientationDeg, int playerID, PlayerShip* playerShip);
	explicit Bullet(Game* owner, Vec2 const& startingPosition, float orientationDeg, int playerID, PlayerShip* playerShip, PowerUpTypes bulletType);
	~Bullet() {};

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void Die() override;

	virtual void LoseHealth() override;
	int GetOwningPlayerID() const;

private:
	virtual void InitializeLocalVerts();
	void SetBulletRangeAndSpeed();

private:
	Vertex_PCU m_localVerts[NUM_BULLET_VERTS];
	PlayerShip* m_owningPlayerShip;
	int m_playerOwnerID = -2;
	Vec2 m_spawnLocation;
	PowerUpTypes m_powerUpType;
	float m_maxBulletRange = 50.f;
	float m_speed = 0.f;
};

