#include "Game/Wasp.hpp"

#include "Engine/Renderer/RendererDX11.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"

Wasp::Wasp(Game* owner, Vec2 const& startPos, float orientationDeg)
	:Entity(owner, startPos, orientationDeg, Rgba8(255, 255, 0, 255))
{
	m_physicsRadius = WASP_PHYSICS_RADIUS;
	m_cosmeticRadius = WASP_COSMETIC_RADIUS;
	m_health = WASP_STARTING_HEALTH;

	InitializeLocalVerts();
}

void Wasp::Update(float deltaSeconds)
{
	if (!m_game->AllPlayersDead())
	{
		RotateToFacePosition(m_game->GetNearestPlayerPosition(m_position));
	}
		
	else if (IsOffScreen())
	{
		WrapToOppositeSide();
	}
	
	Vec2 fwdNormal = GetForwardNormal();
	Vec2 acceleration = fwdNormal * WASP_ACCELERATION;
	m_velocity += acceleration * deltaSeconds;
	m_velocity.ClampLength(WASP_MAX_SPEED);

	m_position += m_velocity * deltaSeconds;
}

void Wasp::Render() const
{
	Vertex_PCU worldSpaceVerts[NUM_WASP_VERTS];
	for (int vertexIndex = 0; vertexIndex < NUM_WASP_VERTS; vertexIndex++)
	{
		worldSpaceVerts[vertexIndex] = m_localVerts[vertexIndex];
	}

	Vec2 fwrdNormal = GetForwardNormal();
	TransformVertexArrayXY3D(NUM_WASP_VERTS, worldSpaceVerts, fwrdNormal, fwrdNormal.GetRotated90Degrees(), m_position);

	g_renderer->DrawVertexArray(NUM_WASP_VERTS, &worldSpaceVerts[0]);

	if (m_game->m_shouldDrawDebug)
	{
		DebugRender(m_game->m_firstPlayerShip->m_position);
	}
		
}

void Wasp::Die()
{
	m_game->PlayGameSFX(StarShipSFX::ENEMY_DEATH, m_position);

	m_isDead = true;
	m_isGarbage = true;
	m_game->m_numEnemies--;

	int debrisAmount = g_rng->RollRandomIntInRange(3, 12);
	m_game->SpawnNewDebrisCluster(m_position, debrisAmount, m_velocity, DEBRIS_MAX_SCATTER_SPEED, m_physicsRadius * 0.85f, m_color);

	TryToDropPowerUp(20);
}

void Wasp::InitializeLocalVerts()
{
	//Head
	m_localVerts[0].m_position = Vec3(2.f, 0.f, 0.f);
	m_localVerts[1].m_position = Vec3(0.f, 2.f, 0.f);
	m_localVerts[2].m_position = Vec3(0.f, -2.f, 0.f);

	//Tail
	m_localVerts[3].m_position = Vec3(0.f, -1.f, 0.f);
	m_localVerts[4].m_position = Vec3(0.f, 1.f, 0.f);
	m_localVerts[5].m_position = Vec3(-2.f, 0.f, 0.f);


	for (int vertIndex = 0; vertIndex < NUM_WASP_VERTS; ++vertIndex)
	{
		m_localVerts[vertIndex].m_color = m_color;
	}
}
