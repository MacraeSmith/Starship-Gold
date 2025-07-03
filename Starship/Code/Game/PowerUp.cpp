#include "Game/PowerUp.hpp"

#include "Engine/Renderer/RendererDX11.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"


#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"


PowerUp::PowerUp(Game* owner, Vec2 const& startingPosition, float orientationDeg)
	:Entity(owner, startingPosition, orientationDeg, Rgba8(1, 246, 243, 255))
{
	m_physicsRadius = POWERUP_PHYSICS_RADIUS;
	m_cosmeticRadius = POWERUP_COSMETIC_RADIUS;
	m_velocity = GetForwardNormal() * g_rng->RollRandomFloatInRange(POWERUP_MIN_SPEED, POWERUP_MAX_SPEED);

	m_textOffset = GetSimpleTriangleStringWidth("?", 2.5) * 0.5f;
	InitializeLocalVerts();

	ChooseRandomPowerUp();
}

void PowerUp::Update(float deltaSeconds)
{
	m_position += m_velocity * deltaSeconds;
	if (IsOffScreen())
	{
		WrapToOppositeSide();
	}
}

void PowerUp::Render() const
{
	Vertex_PCU worldSpaceVerts[NUM_POWERUP_VERTS];
	for (int vertexIndex = 0; vertexIndex < NUM_POWERUP_VERTS; vertexIndex++)
	{
		worldSpaceVerts[vertexIndex] = m_localVerts[vertexIndex];
	}

	Vec2 fwrdVector = Vec2::MakeFromPolarDegrees(0.f);
	TransformVertexArrayXY3D(NUM_POWERUP_VERTS, worldSpaceVerts, fwrdVector, fwrdVector.GetRotated90Degrees(), m_position);

	g_renderer->DrawVertexArray(NUM_POWERUP_VERTS, &worldSpaceVerts[0]);

	if (m_game->m_shouldDrawDebug)
	{
		DebugRender(m_game->m_playerShips[0]->m_position);
	}

	std::vector<Vertex_PCU> textVerts;
	AddVertsForTextTriangles2D(textVerts, "?", Vec2(m_position.x - m_textOffset, m_position.y - (m_textOffset * 2.f)), 2.5f, Rgba8(255, 255, 255, 255));
	g_renderer->DrawVertexArray(textVerts);

}

void PowerUp::Die()
{
	m_isDead = true;
	m_isGarbage = true;

	int debrisAmount = g_rng->RollRandomIntInRange(3, 12);
	m_game->SpawnNewDebrisCluster(m_position, debrisAmount, m_velocity, DEBRIS_MAX_SCATTER_SPEED, m_physicsRadius * 0.85f, m_color);
}

void PowerUp::InitializeLocalVerts()
{
	m_localVerts[0].m_position = Vec3(-1.5f, -1.5f, 0.f);
	m_localVerts[1].m_position = Vec3(1.5f, -1.5f, 0.f);
	m_localVerts[2].m_position = Vec3(1.5f, 1.5f, 0.f);
	
	m_localVerts[0].m_color = Rgba8(1, 246, 243, 255); //blue
	m_localVerts[1].m_color = Rgba8(246, 152, 1, 255); //orange
	m_localVerts[2].m_color = Rgba8(1, 246, 35, 255); //green

	m_localVerts[3].m_position = Vec3(-1.5f, -1.5f, 0.f);
	m_localVerts[4].m_position = Vec3(1.5f, 1.5f, 0.f);
	m_localVerts[5].m_position = Vec3(-1.5f, 1.5f, 0.f);

	m_localVerts[3].m_color = Rgba8(1, 246, 243, 255); //blue
	m_localVerts[4].m_color = Rgba8(1, 246, 35, 255); //green
	m_localVerts[5].m_color = Rgba8(245, 40, 145, 255); //pink
}

void PowerUp::ChooseRandomPowerUp()
{
	int randNum = g_rng->RollRandomIntInRange(0, static_cast<int>(PowerUpTypes::NUM_POWERUP_TYPES) - 1);
	m_powerupType = static_cast<PowerUpTypes>(randNum);
}
