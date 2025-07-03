#include "Game/Debris.hpp"

#include "Engine/Renderer/RendererDX11.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"


Debris::Debris(Game* owner, Vec2 const& startPos, float orientationDeg, Vec2 const& velocity, Rgba8 color, float cosmeticRadius, float physicsRadius)
	:Entity(owner, startPos, orientationDeg, color)
{
	m_physicsRadius = physicsRadius;
	m_cosmeticRadius = cosmeticRadius;

	m_velocity = velocity;
	m_angularVelocity = g_rng->RollRandomFloatInRange(-200, 200);
	m_age = 0.f;
	InitializeLocalVerts();

}

void Debris::Update(float deltaSeconds)
{
	m_position += m_velocity * deltaSeconds;
	m_orientationDegrees += m_angularVelocity * deltaSeconds;
	m_age += deltaSeconds;

	UpdateOpacity(); //gradual fade of debris

	if (IsOffScreen() || m_age >= DEBRIS_LIFETIME)
	{
		Die();
	}
	
}

void Debris::Render() const
{
	Vertex_PCU worldSpaceVerts[NUM_DEBRIS_VERTS];
	for (int vertexIndex = 0; vertexIndex < NUM_DEBRIS_VERTS; vertexIndex++)
	{
		worldSpaceVerts[vertexIndex] = m_localVerts[vertexIndex];
	}

	TransformVertexArrayXY3D(NUM_DEBRIS_VERTS, worldSpaceVerts, 1.f, m_orientationDegrees, m_position);

	g_renderer->DrawVertexArray(NUM_DEBRIS_VERTS, &worldSpaceVerts[0]);

	if (m_game->m_shouldDrawDebug)
		DebugRender(m_game->m_firstPlayerShip->m_position);
}

void Debris::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}


void Debris::UpdateOpacity()
{
	float opacity = RangeMapClamped(m_age, 0.f, DEBRIS_LIFETIME, 127.f, 0.f); //Decrease opacity as age increases
	m_color.a = static_cast<unsigned char> (opacity);
	for (int vertex = 0; vertex < NUM_DEBRIS_VERTS; ++vertex)
	{
		m_localVerts[vertex].m_color = m_color;
	}
}

void Debris::InitializeLocalVerts()
{
	float asteroidRadii[NUM_DEBRIS_SIDES] = {};
	for (int sideNum = 0; sideNum < NUM_DEBRIS_SIDES; ++sideNum)
	{
		asteroidRadii[sideNum] = g_rng->RollRandomFloatInRange(m_physicsRadius, m_cosmeticRadius);
	}

	//compute 2d vertex offsets
	constexpr float degreesPerAstroidSide = 360.f / static_cast<float>(NUM_DEBRIS_SIDES);
	Vec2 asteroidLocalVertPositions[NUM_DEBRIS_SIDES] = {};
	for (int sideNum = 0; sideNum < NUM_DEBRIS_SIDES; ++sideNum)
	{
		float degrees = degreesPerAstroidSide * static_cast<float>(sideNum);
		float radius = asteroidRadii[sideNum];
		asteroidLocalVertPositions[sideNum].x = radius * CosDegrees(degrees);
		asteroidLocalVertPositions[sideNum].y = radius * SinDegrees(degrees);
	}

	//build triangles
	for (int triNum = 0; triNum < NUM_DEBRIS_TRIS; ++triNum)
	{
		int startRadiusIndex = triNum;
		int endRadiusIndex = (triNum + 1) % NUM_DEBRIS_SIDES;
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
	for (int vertIndex = 0; vertIndex < NUM_DEBRIS_VERTS; ++vertIndex)
	{
		m_localVerts[vertIndex].m_color = Rgba8(100, 100, 100);
	}
}
