#pragma once
#include "Entity.hpp"
constexpr int NUM_STAR_TRIS = 2;
constexpr int NUM_STAR_VERTS = 3 * NUM_STAR_TRIS;
class Star : public Entity
{
public:
	explicit Star(Game* owner, Vec2 const& startPos, float orientationDeg, float scale);
	~Star() {};

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;

private:
	virtual void InitializeLocalVerts();

private:
	Vertex_PCU m_localVerts[NUM_STAR_VERTS];
	float m_scale = 1.f;
	float m_twinkleSpeed = 5;
	float m_timeSinceLastTwinkle = 0.f;
};

