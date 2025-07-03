#include "Game/Entity.hpp"

#include "Engine/Math/RandomNumberGenerator.hpp"
#include"Engine/Core/Vertex_PCU.hpp"

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"


Entity::Entity(Game* gameInstance, const Vec2& startingPosition, float orientationDeg, Rgba8 color)
	:m_game(gameInstance)
	,m_position(startingPosition)
	,m_orientationDegrees(orientationDeg)
	,m_color(color)
{
}

Entity::Entity(Game* gameInstance, Vec2 const& startingPosition, float orientationDeg)
	:m_game(gameInstance)
	, m_position(startingPosition)
	, m_orientationDegrees(orientationDeg)
{
}

void Entity::DebugRender(Vec2 const& shipPos) const
{
	//Forward Vector
	Vec2 vecFwrd = GetForwardNormal() * m_cosmeticRadius;
	DebugDrawLine2D(m_position, m_position + vecFwrd, DEBUG_LINE_THICKNESS, Rgba8(255, 0, 0));

	//Draw Line To player Ship
	DebugDrawLine2D(m_position, shipPos, DEBUG_LINE_THICKNESS, Rgba8(50, 50, 50));

	//Left Vector
	Vec2 vecLeft = vecFwrd.GetRotated90Degrees();
	DebugDrawLine2D(m_position, m_position + vecLeft, DEBUG_LINE_THICKNESS, Rgba8(0, 255, 0));

	//Cosmetic Ring
	DebugDrawRing(m_position, m_cosmeticRadius, DEBUG_LINE_THICKNESS, Rgba8(255, 0, 255));

	//Physics Ring
	DebugDrawRing(m_position, m_physicsRadius, DEBUG_LINE_THICKNESS, Rgba8(0, 255, 255));

	//Velocity Line
	DebugDrawLine2D(m_position, m_position + m_velocity, DEBUG_LINE_THICKNESS, Rgba8(255, 255, 0));
}

//Version of DebugRender that does not include line to Player ship
void Entity::DebugRender() const
{
	//Forward Vector
	Vec2 vecFwrd = GetForwardNormal() * m_cosmeticRadius;
	DebugDrawLine2D(m_position, m_position + vecFwrd, DEBUG_LINE_THICKNESS, Rgba8(255, 0, 0));

	//Left Vector
	Vec2 vecLeft = vecFwrd.GetRotated90Degrees();
	DebugDrawLine2D(m_position, m_position + vecLeft, DEBUG_LINE_THICKNESS, Rgba8(0, 255, 0));

	//Cosmetic Ring
	DebugDrawRing(m_position, m_cosmeticRadius, DEBUG_LINE_THICKNESS, Rgba8(255, 0, 255));

	//Physics Ring
	DebugDrawRing(m_position, m_physicsRadius, DEBUG_LINE_THICKNESS, Rgba8(0, 255, 255));

	//Velocity Line
	DebugDrawLine2D(m_position, m_position + m_velocity, DEBUG_LINE_THICKNESS, Rgba8(255, 255, 0));
}

//Health
// ---------------------------------------------------------------------------------------------- -
void Entity::LoseHealth()
{
	m_health--;
	m_game->StartScreenShake(.5f, .75f);

	if (m_health <= 0)
	{
		Die();
	}
}

void Entity::Die()
{
	m_isDead = true;
	m_isGarbage = true;

	int debrisAmount = g_rng->RollRandomIntInRange(3, 12);
	m_game->SpawnNewDebrisCluster(m_position, debrisAmount, m_velocity, DEBRIS_MAX_SCATTER_SPEED, m_physicsRadius * 0.85f, m_color);
}

//Helper Mutators
// ----------------------------------------------------------------------------------------------
void Entity::ToggleDebugDraw()
{
	m_shouldDrawDebug = !m_shouldDrawDebug;
}

void Entity::WrapToOppositeSide()
{
	//west wall
	if (m_position.x < -m_cosmeticRadius)
	{
		m_position.x = WORLD_SIZE_X + m_cosmeticRadius;
	}

	//east wall
	else if (m_position.x > WORLD_SIZE_X + m_cosmeticRadius)
	{
		m_position.x = 0.f - m_cosmeticRadius;
	}

	//north wall
	if (m_position.y > WORLD_SIZE_Y + m_cosmeticRadius)
	{
		m_position.y = 0.f - m_cosmeticRadius;
	}

	//south wall
	if (m_position.y < -m_cosmeticRadius)
	{
		m_position.y = WORLD_SIZE_Y + m_cosmeticRadius;
	}
}

void Entity::TryToDropPowerUp(int percentageSuccess) const
{
	int randNum = g_rng->RollRandomIntInRange(0, 100);
	if (randNum > percentageSuccess)
		return;

	m_game->SpawnNewPowerUp(m_position);
}

void Entity::RotateToFacePosition(Vec2 const& position)
{
	Vec2 newdirection = position - m_position;
	m_orientationDegrees = newdirection.GetOrientationDegrees();
}

//Accessors
// ----------------------------------------------------------------------------------------------
bool const Entity::IsAlive() const
{
	return !m_isDead;
}

Vec2 const Entity::GetForwardNormal() const
{
	return Vec2::MakeFromPolarDegrees(m_orientationDegrees, 1.f);
}

float const Entity::GetPhysicsRadius() const
{
	return m_physicsRadius;
}

bool const Entity::IsOffScreen() const
{
	Vec2 worldMax(WORLD_CENTER_X + (WORLD_SIZE_X * 0.5f), WORLD_CENTER_Y + (WORLD_SIZE_Y * 0.5f));
	Vec2 worldMin(WORLD_CENTER_X - (WORLD_SIZE_X * 0.5f), WORLD_CENTER_Y - (WORLD_SIZE_Y * 0.5f));

	if (m_position.x > worldMax.x + m_cosmeticRadius || m_position.y > worldMax.y + m_cosmeticRadius
		|| m_position.x < worldMin.x - m_cosmeticRadius || m_position.y < worldMin.y - m_cosmeticRadius)
	{
		return true;
	}

	return false;
}

