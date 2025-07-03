#include "Game/Bullet.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/RendererDX11.hpp"

#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/PlayerShip.hpp"


Bullet::Bullet(Game* owner, Vec2 const& startingPosition, float orientationDeg, int playerID, PlayerShip* playerShip)
	:Entity(owner, startingPosition, orientationDeg)
{
	m_speed = BULLET_SPEED;
	m_cosmeticRadius = BULLET_COSMETIC_RADIUS;
	m_physicsRadius = BULLET_PHYSICS_RADIUS;
	m_health = 1;
	m_playerOwnerID = playerID;
	m_owningPlayerShip = playerShip;
	m_spawnLocation = startingPosition;
	m_maxBulletRange = 50.f;
	m_powerUpType = PowerUpTypes::NUM_POWERUP_TYPES;
	InitializeLocalVerts();
}

Bullet::Bullet(Game* owner, Vec2 const& startingPosition, float orientationDeg, int playerID, PlayerShip* playerShip, PowerUpTypes bulletType)
	:Entity(owner, startingPosition, orientationDeg)
{
	m_cosmeticRadius = BULLET_COSMETIC_RADIUS;
	m_physicsRadius = BULLET_PHYSICS_RADIUS;
	m_health = 1;
	m_playerOwnerID = playerID;
	m_owningPlayerShip = playerShip;
	m_spawnLocation = startingPosition;
	m_powerUpType = bulletType;
	SetBulletRangeAndSpeed();
	InitializeLocalVerts();
}


void Bullet::Update(float deltaSeconds)
{
	m_velocity = GetForwardNormal() * m_speed;
	m_position += m_velocity * deltaSeconds;
	
	float distanceFromShip = GetDistance2D(m_spawnLocation, m_position);
	if (distanceFromShip >= m_maxBulletRange)
	{
		Die();
	}

	if (IsOffScreen())
	{
		Die();
	}
}

void Bullet::Render() const
{
	Vertex_PCU worldSpaceVerts[NUM_BULLET_VERTS];
	for (int vertexIndex = 0; vertexIndex < NUM_BULLET_VERTS; vertexIndex++)
	{
		worldSpaceVerts[vertexIndex] = m_localVerts[vertexIndex];
	}

	Vec2 fwrdNormal = GetForwardNormal();
	TransformVertexArrayXY3D(NUM_BULLET_VERTS, worldSpaceVerts, fwrdNormal, fwrdNormal.GetRotated90Degrees(), m_position);

	g_renderer->DrawVertexArray(NUM_BULLET_VERTS, &worldSpaceVerts[0]);

	if (m_game->m_shouldDrawDebug)
		DebugRender(m_game->m_firstPlayerShip->m_position);
}

void Bullet::Die()
{
	if (m_powerUpType == PowerUpTypes::SNIPER_BULLET)
	{
		if (GetDistance2D(m_spawnLocation, m_position) < m_maxBulletRange)
			return;
	}

	m_isDead = true;
	m_isGarbage = true;
}


void Bullet::LoseHealth()
{
	Die();
}

int Bullet::GetOwningPlayerID() const
{
	return m_playerOwnerID;
}

void Bullet::InitializeLocalVerts()
{
	//bullet nose vertex position
	m_localVerts[0].m_position = Vec3(0.f, -0.5f, 0.f);
	m_localVerts[1].m_position = Vec3(0.5f, 0.f, 0.f);
	m_localVerts[2].m_position = Vec3(0.f, 0.5f, 0.f);
	//bullet nose color
	m_localVerts[0].m_color = Rgba8(255, 255, 0, 255);
	m_localVerts[1].m_color = Rgba8(255, 255, 0, 255);
	m_localVerts[2].m_color = Rgba8(255, 255, 0, 255);

	//bullet tail vertex positions
	m_localVerts[3].m_position = Vec3(0.f, -0.5f, 0.f);
	m_localVerts[4].m_position = Vec3(0.f, 0.5f, 0.f);
	m_localVerts[5].m_position = Vec3(-2.f, 0.f, 0.f);

	//bullet tail colors
	m_localVerts[3].m_color = Rgba8(255, 0, 0, 255);
	m_localVerts[4].m_color = Rgba8(255, 0, 0, 255);
	m_localVerts[5].m_color = Rgba8(255, 0, 0, 0); //transparent end vertex
}

void Bullet::SetBulletRangeAndSpeed()
{
	switch (m_powerUpType)
	{
	case PowerUpTypes::TRI_BULLET:
		m_maxBulletRange = 50.f;
		m_speed = BULLET_SPEED;
		break;
	case PowerUpTypes::FIVE_BULLET:
		m_maxBulletRange = 50.f;
		m_speed = BULLET_SPEED;
		break;
	case PowerUpTypes::SNIPER_BULLET:
		m_maxBulletRange = 300.f;
		m_speed = BULLET_SPEED * 1.5f;
		break;
	case PowerUpTypes::BURST_BULLET:
		m_maxBulletRange = 10.f;
		m_speed = BULLET_SPEED;
		break;
	default:
		m_speed =  BULLET_SPEED;
		break;
	}
}


