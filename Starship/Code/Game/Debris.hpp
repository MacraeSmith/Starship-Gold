#pragma once
#include "Entity.hpp"
constexpr int NUM_DEBRIS_SIDES = 8;
constexpr int NUM_DEBRIS_TRIS = NUM_DEBRIS_SIDES;
constexpr int NUM_DEBRIS_VERTS = 3 * NUM_DEBRIS_TRIS;
class Debris : public Entity
{
public:
	explicit Debris(Game* owner, Vec2 const& startPos, float orientationDeg, Vec2 const& velocity, Rgba8 color, float cosmeticRadius, float physicsRadius);
	~Debris() {};

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void Die() override;

private:
	void UpdateOpacity();
	virtual void InitializeLocalVerts();

private:
	Vertex_PCU m_localVerts[NUM_DEBRIS_VERTS];

};

