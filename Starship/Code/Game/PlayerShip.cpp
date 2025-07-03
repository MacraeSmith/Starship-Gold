#include "Game/PlayerShip.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/RendererDX11.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Input/InputSystem.hpp"

#include "Game/Bullet.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


PlayerShip::PlayerShip(Game* owner, Vec2 const& startPos, float orientationDeg, Rgba8 color, int playerNum, int playerID)
	:Entity(owner, startPos, orientationDeg, color)
{
	m_physicsRadius = PLAYER_SHIP_PHYSICS_RADIUS;
	m_cosmeticRadius = PLAYER_SHIP_COSMETIC_RADIUS;
	m_health = PLAYER_SHIP_STARTING_HEALTH;
	m_remainingLives = PLAYER_SHIP_NUM_STARTING_LIVES;

	m_playerID = playerID;
	m_playerNum = playerNum;

	m_DamagedColor = Rgba8(255, 255, 255, 255);
	m_spawnLocation = startPos;

	InitializeLocalVerts(&m_localVerts[0], color);
	InitializeEngineFlameVerts();
	m_game->PlayGameMusic(m_engineThrustSound, StarShipMusic::PLAYER_SHIP_ENGINE_THRUST, true);
}

PlayerShip::~PlayerShip()
{
	g_audioSystem->StopSound(m_engineThrustSound);
}
																					
void PlayerShip::Update(float deltaSeconds)											
{	
	m_thrustFraction = 0.f;

	CheckInput(deltaSeconds);
	
	//Move the ship
	m_position += m_velocity * deltaSeconds;
	BounceOffWalls();

	//Change flame length
	float flameVariation = g_rng->RollRandomFloatZeroToOne();
	float flamePointPos = Lerp(-2.f, -2.f - (m_thrustFraction * m_engineFlameMaxLength), flameVariation);
	m_engineFlameVerts[2].m_position = Vec3(flamePointPos, 0.f, 0.f);

	//Change Engine Audio
	g_audioSystem->SetSoundPlaybackVolume(m_engineThrustSound, m_thrustFraction);
	g_audioSystem->SetSoundPlaybackBalance(m_engineThrustSound, m_game->GetAudioBalanceFromWorldPosition(m_position));

	RunTimers(deltaSeconds);
	
}																					
																					
void PlayerShip::Render() const														
{	
	//Do not render when ship is dead or inactive
	if (m_isDead)
		return;

	//Player Ship
	//---------------------------------------------------------------------------------
	Vertex_PCU worldSpaceVerts[NUM_PLAYERSHIP_VERTS];
	for (int vertexIndex = 0; vertexIndex < NUM_PLAYERSHIP_VERTS; vertexIndex++)
	{
		worldSpaceVerts[vertexIndex] = m_localVerts[vertexIndex];
	}

	Vec2 fwrdNormal = GetForwardNormal();
	TransformVertexArrayXY3D(NUM_PLAYERSHIP_VERTS, worldSpaceVerts, fwrdNormal, fwrdNormal.GetRotated90Degrees(), m_position);
	g_renderer->DrawVertexArray(NUM_PLAYERSHIP_VERTS, &worldSpaceVerts[0]);
	
	//Engine flame
	//---------------------------------------------------------------------------------
	Vertex_PCU worldSpaceFlameVerts[NUM_ENGINE_FLAME_VERTS];
	for (int vertexIndex = 0; vertexIndex < NUM_ENGINE_FLAME_VERTS; vertexIndex++)
	{
		worldSpaceFlameVerts[vertexIndex] = m_engineFlameVerts[vertexIndex];
	}

	TransformVertexArrayXY3D(NUM_ENGINE_FLAME_VERTS, worldSpaceFlameVerts, fwrdNormal, fwrdNormal.GetRotated90Degrees(), m_position);
	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(NUM_ENGINE_FLAME_VERTS, &worldSpaceFlameVerts[0]);

	//Respawn Shield
	//---------------------------------------------------------------------------------
	if (m_hasShield)
	{
		DebugDrawRing(m_position, PLAYER_SHIP_SHIELD_RADIUS, PLAYER_SHIP_SHIELD_RADIUS - PLAYER_SHIP_COSMETIC_RADIUS, Rgba8(182, 234, 246, static_cast<unsigned char> (m_shieldOpacity)));
	}

	if(m_game->m_shouldDrawDebug)
		DebugRender();
}

void PlayerShip::Die()
{
	m_game->PlayGameSFX(StarShipSFX::PLAYER_DEATH, m_position);

	m_isDead = true;
	m_remainingLives--;

	m_game->StartScreenShake(1.f, 2.f);
	int debrisAmount = g_rng->RollRandomIntInRange(5, 30);
	m_game->SpawnNewDebrisCluster(m_position, debrisAmount, m_velocity, DEBRIS_MAX_SCATTER_SPEED, m_physicsRadius * 0.85f, m_color);

	if (m_remainingLives <= 0)
	{
		if (m_game->m_inMultiplayerMode)
		{
			m_game->CheckNumRemainingPlayersForGameOver();
		}
		else
		{
			m_game->GameOver(m_playerNum, false);
		}
	}
}

void PlayerShip::LoseHealth()
{
	if (m_hasShield || !m_canTakeDamage)
		return;
	m_game->PlayGameSFX(StarShipSFX::PLAYER_DAMAGED, m_position);
	m_game->StartScreenShake(.5f, .75f);

	m_health--;
	m_canTakeDamage = false;
	ToggleDamageColor(true);

	if (m_health <= 0)
	{
		Die();
	}
}

void PlayerShip::CheckInput(float deltaSeconds)
{
	CheckKeyboardInput(deltaSeconds);
	CheckControllerInput(deltaSeconds);
}

void PlayerShip::CheckKeyboardInput(float deltaSeconds)
{
	//Player one controls on keyboard
	if (m_playerID != -1)
		return;

	//When ship is dead check for respawn button but ignore all other controls
	if (m_isDead)
	{
		if (m_game->m_inGameOverSequence) //don't allow respawn when game is playing its end sequence
			return;

		if (g_inputSystem->WasKeyJustPressed('N') && m_game->m_inGameplay)
		{
			RespawnShip();
		}

		return;
	}

	//Turn ship
	//------------------------------------------------------------------------------
	if (g_inputSystem->IsKeyDown('A'))
	{
		m_orientationDegrees += (PLAYER_SHIP_TURN_SPEED * deltaSeconds); //Spin ship left
	}

	if (g_inputSystem->IsKeyDown('D'))
	{
		m_orientationDegrees -= (PLAYER_SHIP_TURN_SPEED * deltaSeconds); //Spin ship right
	}

	//Thrust forward
	//------------------------------------------------------------------------------
	Vec2 fwdNormal = GetForwardNormal();
	Vec2 acceleration = fwdNormal * PLAYER_SHIP_ACCELERATION;

	if (g_inputSystem->IsKeyDown('W'))
	{
		if (m_game->m_inGameplay)
		{
			m_velocity += acceleration * deltaSeconds;
		}
		m_thrustFraction = 1.f;
	}

	//Fire Bullets
	//------------------------------------------------------------------------------
	if (g_inputSystem->WasKeyJustPressed(' ') && m_game->m_inGameplay)
	{
		Vec2 shipNosePos = m_position;
		shipNosePos += GetForwardNormal() * 2.f; // adds 2.f offset from ship position in direction ship is facing

		if (m_hasPowerUp)
		{
			if (m_powerUpType == PowerUpTypes::BURST_BULLET)
			{
				m_game->SpawnNewSpecialBullet(m_position, m_orientationDegrees, m_playerID, m_powerUpType);
				return;
			}

			m_game->SpawnNewSpecialBullet(shipNosePos, m_orientationDegrees, m_playerID, m_powerUpType);
			return;
		}

		m_game->SpawnNewBullet(shipNosePos, m_orientationDegrees, m_playerID);
	}
}

void PlayerShip::CheckControllerInput(float deltaSeconds)
{
	XboxController playerController = g_inputSystem->GetController(m_playerID);
	if (m_isDead)
	{
		if (m_game->m_inGameOverSequence) //don't allow respawn when game is playing its end sequence
			return;

		if (playerController.WasButtonJustPressed(XboxButtonID::BUTTON_START) && m_game->m_inGameplay)
		{
			RespawnShip();
		}

		return;
	}

	//Turn ship
	//------------------------------------------------------------------------------
	AnalogJoystick const& rightStick = playerController.GetRightStick();
	if (rightStick.GetMagnitude() > 0.f)
	{
		m_orientationDegrees = rightStick.GetOrientationDegrees();
	}

	//Thrust forward
	//------------------------------------------------------------------------------
	Vec2 fwdNormal = GetForwardNormal();
	Vec2 acceleration = fwdNormal * PLAYER_SHIP_ACCELERATION;

	AnalogJoystick const& leftStick = playerController.GetLeftStick();
	Vec2 leftStickPos = leftStick.GetPosition();
	if (leftStick.GetMagnitude() > 0.f && leftStickPos.y > 0.f)
	{
		m_thrustFraction = leftStick.GetMagnitude();
		if (m_game->m_inGameplay)
		{
			m_velocity += m_thrustFraction * acceleration * deltaSeconds;
		}
	}

	//Fire Bullets
	//------------------------------------------------------------------------------
	if (!m_game->m_inGameplay)
		return;

	if (playerController.WasButtonJustPressed(XboxButtonID::BUTTON_A) || playerController.WasButtonJustPressed(XboxButtonID::BUTTON_VIRTUAL_RIGHT_TRIGGER_BUTTON))
	{
		Vec2 shipNosePos = m_position;
		shipNosePos += GetForwardNormal() * 2.f; // adds 2.f offset from ship position in direction ship is facing

		if (m_hasPowerUp)
		{
			if (m_powerUpType == PowerUpTypes::BURST_BULLET)
			{
				m_game->SpawnNewSpecialBullet(m_position, m_orientationDegrees, m_playerID, m_powerUpType);
				return;
			}

			m_game->SpawnNewSpecialBullet(shipNosePos, m_orientationDegrees, m_playerID, m_powerUpType);
			return;
		}

		m_game->SpawnNewBullet(shipNosePos, m_orientationDegrees, m_playerID);
	}
}

void PlayerShip::RunTimers(float deltaSeconds)
{
	if (m_inDamageAnim)
	{
		m_damageAnimAge += deltaSeconds;
		if (m_damageAnimAge >= m_damageAnimMaxAge)
		{
			ToggleDamageColor(false);
		}
	}

	if (m_hasShield)
	{
		m_shieldAge += deltaSeconds;
		float fraction = GetFractionWithinRange(m_shieldAge, 0.f, m_shieldMaxAge);

		//Flicker effect as shield is running out
		if ((fraction > .75f && fraction < .8f) || (fraction > .9f && fraction < .95f))
		{
			m_shieldOpacity = 50.f;
		}
		else 
		{
			m_shieldOpacity = 200.f;
		}

		if (m_shieldAge >= m_shieldMaxAge)
		{
			ToggleShield(false, 0.f);
			m_shieldAge = 0.f;
		}
	}

	if (!m_canTakeDamage)
	{
		m_respawnProtectionAge += deltaSeconds;
		if (m_respawnProtectionAge >= m_respawnProtectionMaxAge)
		{
			m_canTakeDamage = true;
			m_respawnProtectionAge = 0.f;
		}
	}

	if (m_hasPowerUp)
	{
		m_powerUpAge += deltaSeconds;
		if (m_powerUpAge >= m_powerUpMaxAge)
		{
			m_hasPowerUp = false;
			m_powerUpAge = 0.f;
		}
	}
}

void PlayerShip::InitializeLocalVerts(Vertex_PCU* vertsToFillIn, Rgba8 color)
{
	//Left Wing
	vertsToFillIn[0].m_position = Vec3(2.f, 1.f, 0.f);
	vertsToFillIn[1].m_position = Vec3(0.f, 2.f, 0.f);
	vertsToFillIn[2].m_position = Vec3(-2.f, 1.f, 0.f);

	//body
	vertsToFillIn[3].m_position = Vec3(0.f, 1.f, 0.f);
	vertsToFillIn[4].m_position = Vec3(-2.f, 1.f, 0.f);
	vertsToFillIn[5].m_position = Vec3(-2.f, -1.f, 0.f);
	vertsToFillIn[6].m_position = Vec3(0.f, -1.f, 0.f);
	vertsToFillIn[7].m_position = Vec3(0.f, 1.f, 0.f);
	vertsToFillIn[8].m_position = Vec3(-2.f, -1.f, 0.f);

	//right wing
	vertsToFillIn[9].m_position = Vec3(2.f, -1.f, 0.f);
	vertsToFillIn[10].m_position = Vec3(-2.f, -1.f, 0.f);
	vertsToFillIn[11].m_position = Vec3(0.f, -2.f, 0.f);

	//nose
	vertsToFillIn[12].m_position = Vec3(1.f, 0.f, 0.f);
	vertsToFillIn[13].m_position = Vec3(0.f, 1.f, 0.f);
	vertsToFillIn[14].m_position = Vec3(0.f, -1.f, 0.f);

	for (int vertIndex = 0; vertIndex < NUM_PLAYERSHIP_VERTS; ++vertIndex)
	{
		vertsToFillIn[vertIndex].m_color = color;
	}
}

void PlayerShip::InitializeEngineFlameVerts()
{
	m_engineFlameVerts[0].m_position = Vec3(-2.f, -.75f, 0.f);
	m_engineFlameVerts[1].m_position = Vec3(-2.f, .75f, 0.f);
	m_engineFlameVerts[2].m_position = Vec3(-3.f, 0.f, 0.f);

	m_engineFlameVerts[0].m_color = Rgba8(255, 200, 0, 255);
	m_engineFlameVerts[1].m_color = Rgba8(255, 200, 0, 255);
	m_engineFlameVerts[2].m_color = Rgba8(255, 0, 0, 50);
}

void PlayerShip::RespawnShip()
{
	if (m_remainingLives <= 0.f)
		return;
	m_game->PlayGameSFX(StarShipSFX::PLAYER_RESPAWN, m_position);

	m_isDead = false;
	m_health = PLAYER_SHIP_STARTING_HEALTH;
	m_game->m_numExtraLives[m_playerNum] = m_remainingLives - 1;

	m_position = m_spawnLocation;
	m_orientationDegrees = 0.f;
	m_velocity = Vec2(0.f, 0.f);

	ToggleShield(true, 5.f);
}

void PlayerShip::BounceOffWalls()
{
	//west wall
	if (m_position.x < m_physicsRadius)
	{
		m_velocity.Reflect(Vec2(1.f, 0.f));
		m_position.x = 0.f + m_physicsRadius;
	}

	//east wall
	else if (m_position.x > WORLD_SIZE_X - m_physicsRadius)
	{
		m_velocity.Reflect(Vec2(-1.f, 0.f));
		m_position.x = WORLD_SIZE_X - m_physicsRadius;
	}

	//north wall
	if (m_position.y > WORLD_SIZE_Y - m_physicsRadius)
	{
		m_velocity.Reflect(Vec2(0.f, -1.f));
		m_position.y = WORLD_SIZE_Y - m_physicsRadius;
	}

	//south wall
	if (m_position.y < m_physicsRadius)
	{
		m_velocity.Reflect(Vec2(0.f, 1.f));
		m_position.y = 0.f + m_physicsRadius;
	}
}

void PlayerShip::ToggleDamageColor(bool start)
{
	m_inDamageAnim = start;
	m_damageAnimAge = 0.f;

	if (start)
	{
		ChangeColorsOfVertexArray(NUM_PLAYERSHIP_VERTS, &m_localVerts[0], m_DamagedColor);
		return;
	}

	ChangeColorsOfVertexArray(NUM_PLAYERSHIP_VERTS, &m_localVerts[0], m_color);
}

void PlayerShip::PickUpPowerUp(PowerUpTypes const& powerUpType)
{
	m_game->PlayGameSFX(StarShipSFX::POWERUP, m_position);
	m_powerUpType = powerUpType;
	m_powerUpAge = 0;
	m_hasPowerUp = true;
	m_hasShield = false;
	
	switch (powerUpType)
	{
	case PowerUpTypes::TRI_BULLET:
		m_powerUpMaxAge = 60.f;
		break;

	case PowerUpTypes::FIVE_BULLET:
		m_powerUpMaxAge = 30.f;
		break;

	case PowerUpTypes::SNIPER_BULLET:
		m_powerUpMaxAge = 20.f;
		break;

	case PowerUpTypes::BURST_BULLET:
		m_powerUpMaxAge = 5.f;
		break;

	case PowerUpTypes::SHIELD:
		m_powerUpMaxAge = 10.f;
		ToggleShield(true, m_powerUpMaxAge);
		break;

	case PowerUpTypes::HEALTH:
		m_health = PLAYER_SHIP_STARTING_HEALTH;
		m_powerUpMaxAge = .25f;
		ToggleShield(true, m_powerUpMaxAge);
		m_hasPowerUp = false;
		break;

	default:
		break;
	}
}

void PlayerShip::ToggleShield(bool const& turnOn, float const& duration)
{
	m_hasShield = turnOn;
	m_shieldAge = 0.f;
	m_shieldMaxAge = duration;
}

bool PlayerShip::HasShield()
{
	return m_hasShield;
}


																					