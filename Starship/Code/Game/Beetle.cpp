#include "Game/Beetle.hpp"

#include "Engine/Renderer/RendererDX11.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"

Beetle::Beetle(Game* owner, Vec2 const& startPos, float orientationDeg)
	:Entity(owner, startPos, orientationDeg, Rgba8(51,255,51,255))
{
	m_physicsRadius = BEETLE_PHYSICS_RADIUS;
	m_cosmeticRadius = BEETLE_COSMETIC_RADIUS;
	m_health = BEETLE_STARTING_HEALTH;

	m_velocity = GetForwardNormal() * BEETLE_SPEED;

	InitializeLocalVerts();
}

void Beetle::Update(float deltaSeconds)
{
	if (!m_game->AllPlayersDead())
	{
		RotateToFacePosition(m_game->GetNearestPlayerPosition(m_position));
	}

	else if (IsOffScreen())
	{
		WrapToOppositeSide();
	}

	m_velocity = GetForwardNormal() * BEETLE_SPEED;
	m_position += m_velocity * deltaSeconds;
	
}

void Beetle::Render() const
{
	Vertex_PCU worldSpaceVerts[NUM_BEETLE_VERTS];
	for (int vertexIndex = 0; vertexIndex < NUM_BEETLE_VERTS; vertexIndex++)
	{
		worldSpaceVerts[vertexIndex] = m_localVerts[vertexIndex];
	}

	Vec2 fwrdNormal = GetForwardNormal();
	TransformVertexArrayXY3D(NUM_BEETLE_VERTS, worldSpaceVerts, fwrdNormal, fwrdNormal.GetRotated90Degrees(), m_position);

	g_renderer->BindTexture(nullptr);
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_renderer->DrawVertexArray(NUM_BEETLE_VERTS, &worldSpaceVerts[0]);

	if (m_game->m_shouldDrawDebug)
	{
		DebugRender(m_game->m_firstPlayerShip->m_position);
	}
}

void Beetle::Die()
{
	m_game->PlayGameSFX(StarShipSFX::ENEMY_DEATH, m_position);

	m_isDead = true;
	m_isGarbage = true;
	m_game->m_numEnemies--;

	int debrisAmount = g_rng->RollRandomIntInRange(3, 12);
	m_game->SpawnNewDebrisCluster(m_position, debrisAmount, m_velocity, DEBRIS_MAX_SCATTER_SPEED, m_physicsRadius * 0.85f, m_color);

	TryToDropPowerUp(10);
}

void Beetle::InitializeLocalVerts()
{
	//Front body
	m_localVerts[0].m_position = Vec3(2.f, -2.f, 0.f);
	m_localVerts[1].m_position = Vec3(2.f, 2.f, 0.f);
	m_localVerts[2].m_position = Vec3(-2.f, -1.f, 0.f);

	//Back Body
	m_localVerts[3].m_position = Vec3(2.f, 2.f, 0.f);
	m_localVerts[4].m_position = Vec3(-2.f, 1.f, 0.f);
	m_localVerts[5].m_position = Vec3(-2.f, -1.f, 0.f);

	//Left Pincer
	m_localVerts[6].m_position = Vec3(2.f, 1.f, 0.f);
	m_localVerts[7].m_position = Vec3(3.f, 2.f, 0.f);
	m_localVerts[8].m_position = Vec3(2.f, 2.f, 0.f);

	//Right Pincer
	m_localVerts[9].m_position = Vec3(2.f, -1.f, 0.f);
	m_localVerts[10].m_position = Vec3(3.f, -2.f, 0.f);
	m_localVerts[11].m_position = Vec3(2.f, -2.f, 0.f);

	for (int vertIndex = 0; vertIndex < NUM_BEETLE_VERTS; ++vertIndex)
	{
		m_localVerts[vertIndex].m_color = m_color;
	}

}
