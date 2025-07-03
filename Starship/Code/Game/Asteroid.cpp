#include "Game/Asteroid.hpp"

#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/RendererDX11.hpp"
#include "Engine/Math/AABB2.hpp"

#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/PlayerShip.hpp"



Asteroid::Asteroid(Game* owner, Vec2 const& startPos, float orientationDeg)
	:Entity(owner, startPos, orientationDeg, Rgba8(100, 100, 100))
{
	m_physicsRadius = ASTEROID_PHYSICS_RADIUS;
	m_cosmeticRadius = ASTEROID_COSMETIC_RADIUS;
	m_health = ASTEROID_STARTING_HEALTH;

	m_angularVelocity = g_rng->RollRandomFloatInRange(-200, 200);
	m_velocity = GetForwardNormal() * g_rng->RollRandomFloatInRange(ASTEROID_MIN_SPEED, ASTEROID_MAX_SPEED);

	InitializeLocalVerts();
}

void Asteroid::Update(float deltaSeconds)
{
	m_position += m_velocity * deltaSeconds;
	m_orientationDegrees += m_angularVelocity * deltaSeconds;
	
	if (m_health <= 0)
	{
		Die();
	}

	if (IsOffScreen())
	{
		WrapToOppositeSide();
	}
}

void Asteroid::Render() const
{
	Vertex_PCU worldSpaceVerts[NUM_ASTEROID_VERTS];
	for (int vertexIndex = 0; vertexIndex < NUM_ASTEROID_VERTS; vertexIndex++)
	{
		worldSpaceVerts[vertexIndex] = m_localVerts[vertexIndex];
	}

	Vec2 fwrdNormal = GetForwardNormal();
	TransformVertexArrayXY3D(NUM_ASTEROID_VERTS, worldSpaceVerts, fwrdNormal, fwrdNormal.GetRotated90Degrees(), m_position);

	g_renderer->DrawVertexArray(NUM_ASTEROID_VERTS, &worldSpaceVerts[0]);

	if (m_game->m_shouldDrawDebug)
	{
		DebugRender(m_game->m_playerShips[0]->m_position);
	}
}

void Asteroid::Die()
{
	m_game->PlayGameSFX(StarShipSFX::ASTEROID_BROKEN, m_position);

	m_isDead = true;
	m_isGarbage = true;

	int debrisAmount = g_rng->RollRandomIntInRange(3, 12);
	m_game->SpawnNewDebrisCluster(m_position, debrisAmount, m_velocity, DEBRIS_MAX_SCATTER_SPEED, m_physicsRadius * 0.85f, m_color);

	TryToDropPowerUp(25);
}

void Asteroid::LoseHealth()
{
	m_game->PlayGameSFX(StarShipSFX::ASTEROID_HIT, m_position);
	m_game->StartScreenShake(.5f, .75f);
	m_health--;

	if (m_health <= 0)
	{
		Die();
	}
}


void Asteroid::InitializeLocalVerts()
{
	//compute random radii along each triangle seam
	float asteroidRadii[NUM_ASTEROID_SIDES] = {};
	for (int sideNum = 0; sideNum < NUM_ASTEROID_SIDES; ++sideNum)
	{
		asteroidRadii[sideNum] = g_rng->RollRandomFloatInRange(m_physicsRadius, m_cosmeticRadius);
	}

	//compute 2d vertex offsets
	constexpr float degreesPerAstroidSide = 360.f / static_cast<float>(NUM_ASTEROID_SIDES);
	Vec2 asteroidLocalVertPositions[NUM_ASTEROID_SIDES] = {};
	for (int sideNum = 0; sideNum < NUM_ASTEROID_SIDES; ++sideNum)
	{
		float degrees = degreesPerAstroidSide * static_cast<float>(sideNum);
		float radius = asteroidRadii[sideNum];
		asteroidLocalVertPositions[sideNum].x = radius * CosDegrees(degrees);
		asteroidLocalVertPositions[sideNum].y = radius * SinDegrees(degrees);
	}

	//build triangles
	for (int triNum = 0; triNum < NUM_ASTEROID_TRIS; ++triNum)
	{
		int startRadiusIndex = triNum;
		int endRadiusIndex = (triNum + 1) % NUM_ASTEROID_SIDES;
		int firstVertIndex = (triNum * 3) + 0;
		int secondVertIndex = (triNum * 3) + 1;
		int thirdVertIndex = (triNum * 3) + 2;
		Vec2 secondVertOfs = asteroidLocalVertPositions[startRadiusIndex];
		Vec2 thirdVertOfs = asteroidLocalVertPositions[endRadiusIndex];
		m_localVerts[firstVertIndex].m_position = Vec3(0.f, 0.f, 0.f);
		m_localVerts[secondVertIndex].m_position = Vec3(secondVertOfs.x, secondVertOfs.y, 0.f);
		m_localVerts[thirdVertIndex].m_position = Vec3(thirdVertOfs.x, thirdVertOfs.y, 0.f);
	}

	//set color
	for (int vertIndex = 0; vertIndex < NUM_ASTEROID_VERTS; ++vertIndex)
	{
		m_localVerts[vertIndex].m_color = m_color;
	}
}

