#pragma once
#include "Entity.hpp"
constexpr int NUM_BEETLE_TRIS = 4;
constexpr int NUM_BEETLE_VERTS = 3 * NUM_BEETLE_TRIS;
class Beetle : public Entity
{
public:
	explicit Beetle(Game* owner, Vec2 const& startPos, float orientationDeg);
	~Beetle() {};

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void Die() override;

private:
	virtual void InitializeLocalVerts();

private:
	Vertex_PCU m_localVerts[NUM_BEETLE_VERTS];
};

