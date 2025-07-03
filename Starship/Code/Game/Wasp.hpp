#pragma once
#include "Entity.hpp"

constexpr int NUM_WASP_TRIS = 2;
constexpr int NUM_WASP_VERTS = 3 * NUM_WASP_TRIS;
class Wasp : public Entity
{
public:
	explicit Wasp(Game* owner, Vec2 const& startPos, float orientationDeg);
	~Wasp() {};

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	virtual void Die() override;

private:
	virtual void InitializeLocalVerts();

private:
	Vertex_PCU m_localVerts[NUM_WASP_VERTS];

};

