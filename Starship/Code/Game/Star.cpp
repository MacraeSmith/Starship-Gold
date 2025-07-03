#include "Game/Star.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/RendererDX11.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

#include "Game/GameCommon.hpp"

Star::Star(Game* owner, Vec2 const& startPos, float orientationDeg, float scale)
	:Entity(owner, startPos, orientationDeg, Rgba8(255, 255, 255, static_cast<unsigned char>(g_rng->RollRandomFloatInRange(100, 255))))
{
	m_scale = scale;
	m_twinkleSpeed = g_rng->RollRandomFloatInRange(1.f, 10.f);
	InitializeLocalVerts();
}

void Star::Update(float deltaSeconds)
{
	m_timeSinceLastTwinkle += deltaSeconds;
	if (m_timeSinceLastTwinkle >= m_twinkleSpeed)
	{
		unsigned char starOpacity = static_cast<unsigned char>(g_rng->RollRandomFloatInRange(50, 255));
		m_timeSinceLastTwinkle = 0.f;
		for (int vertIndex = 0; vertIndex < NUM_STAR_VERTS; ++vertIndex)
		{
			m_localVerts[vertIndex].m_color.a = starOpacity;
		}
	}
	
}

void Star::Render() const
{
	Vertex_PCU worldSpaceVerts[NUM_STAR_VERTS];
	for (int vertIndex = 0; vertIndex < NUM_STAR_VERTS; ++vertIndex)
	{
		worldSpaceVerts[vertIndex] = m_localVerts[vertIndex];
	}

	Vec2 fwrdNormal = GetForwardNormal() * m_scale;
	TransformVertexArrayXY3D(NUM_STAR_VERTS, &worldSpaceVerts[0], fwrdNormal, fwrdNormal.GetRotated90Degrees(), m_position);
	g_renderer->DrawVertexArray(NUM_STAR_VERTS, &worldSpaceVerts[0]);
}

void Star::InitializeLocalVerts()
{
	for (int vertIndex = 0; vertIndex < NUM_STAR_VERTS; ++vertIndex)
	{
		m_localVerts[vertIndex].m_color = m_color;
	}

	m_localVerts[0].m_position = Vec3(0.f, .4f, 0.f);
	m_localVerts[1].m_position = Vec3(0.2f, 0.f, 0.f);
	m_localVerts[2].m_position = Vec3(-0.2f, 0.f, 0.f);

	m_localVerts[3].m_position = Vec3(0.f, -.4f, 0.f);
	m_localVerts[4].m_position = Vec3(0.2f, 0.f, 0.f);
	m_localVerts[5].m_position = Vec3(-0.2f, 0.f, 0.f);

}
