#pragma once
#include "Game/Entity.hpp"
constexpr int NUM_ASTEROID_SIDES = 16;
constexpr int NUM_ASTEROID_TRIS = NUM_ASTEROID_SIDES;
constexpr int NUM_ASTEROID_VERTS = 3 * NUM_ASTEROID_TRIS;
class Asteroid : public Entity
{
public:
	explicit Asteroid(Game* owner, Vec2 const& startPos, float orientationDeg);
	~Asteroid() {};

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;

	virtual void LoseHealth() override;
	virtual void Die() override;

private:
	void InitializeLocalVerts();
	Vertex_PCU m_localVerts[NUM_ASTEROID_VERTS];


};

