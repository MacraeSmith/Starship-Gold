#pragma once
#include "Entity.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/PowerUp.hpp"

enum class PowerUpTypes;

constexpr int NUM_PLAYERSHIP_TRIS = 5;
constexpr int NUM_PLAYERSHIP_VERTS = 3 * NUM_PLAYERSHIP_TRIS;
constexpr int NUM_ENGINE_FLAME_VERTS = 3;
class PlayerShip : public Entity
{
public:
	explicit PlayerShip(Game* owner, Vec2 const& startPos, float orientationDeg, Rgba8 color, int playerNum, int playerID);
	~PlayerShip();

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;

	//Health
	virtual void LoseHealth() override;
	virtual void Die() override;

	static void InitializeLocalVerts(Vertex_PCU* vertsToFillIn, Rgba8 color);

	//Power Ups
	void ToggleShield(bool const& turnOn, float const& duration);
	bool HasShield();
	void PickUpPowerUp(PowerUpTypes const& powerUpType);

private:
	void InitializeEngineFlameVerts();

	//Input
	void CheckInput(float deltaSeconds);
	void CheckKeyboardInput(float deltaSeconds);
	void CheckControllerInput(float deltaSeconds);

	void RunTimers(float deltaSeconds);

	void RespawnShip();
	void BounceOffWalls();
	void ToggleDamageColor(bool start);

public:
	Vertex_PCU m_localVerts[NUM_PLAYERSHIP_VERTS];
	Vertex_PCU m_engineFlameVerts[NUM_ENGINE_FLAME_VERTS];

	int m_playerID = -2;
	int m_playerNum = -2;

	bool m_hasPowerUp = false;
	float m_powerUpAge = 0.f;
	float m_powerUpMaxAge = 0.f;

private:
	//Engine
	Vec2 m_engineFlameFlickerRange = Vec2(2.f, 5.f);
	float m_engineFlameMaxLength = 4.f;
	float m_thrustFraction = 0.f;
	SoundPlaybackID m_engineThrustSound;

	//Damage
	Rgba8 m_DamagedColor;
	bool m_inDamageAnim;
	float m_damageAnimMaxAge = 0.1f;
	float m_damageAnimAge = 0.f;

	//PowerUps
	PowerUpTypes m_powerUpType;
	bool m_hasShield;
	float m_shieldMaxAge = 5.f;
	float m_shieldAge = 0.f;
	float m_shieldOpacity;

	//Respawn
	int m_remainingLives;
	Vec2 m_spawnLocation;
	bool m_canTakeDamage = true;
	float m_respawnProtectionMaxAge = 1.f;
	float m_respawnProtectionAge = 0.f;


	
};

