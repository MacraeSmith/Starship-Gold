#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/EventSystem.hpp"

class PlayerShip;
class Asteroid;
class Bullet;
class RandomNumberGenerator;
class Camera;
struct Vec2;
class Entity;
class Debris;
class Beetle;
class Wasp;
class Star;
class PowerUp;
enum class PowerUpTypes;
class Clock;
class Timer;

struct EnemyWaveInfo
{
	int numBeetles = 1;
	int numWasps = 0;
	int numAsteroids = 3;
};

enum class AttractScreenButtons
{
	SINGLE_PLAYER,
	CO_OP,
	VERSUS,
	INSTRUCTIONS,
	NUM_BUTTONS,
};

struct Button
{
	AttractScreenButtons buttonType;
	AABB2 buttonBounds;
	std::string buttonText = "Text";
	float textCellSize = 30.f;
	Vec2 textPos;
	Rgba8 textColor;
};

constexpr int NUM_ATTRACT_BUTTONS = 4;
struct AttractScreenInfo
{
	Button buttons[NUM_ATTRACT_BUTTONS] = {};
	std::string titleText = "Star Ship Gold";
	Vec2 titlePos;
	Rgba8 defaultButtonColor;
	Rgba8 selectedButtonColor;

	Vec2 rightShipPos;
	Vec2 leftShipPos;
	float rightShipSpeed = 200.f;
	float leftShipSpeed = 200.f;
	Rgba8 rightShipColor;
	Rgba8 leftShipColor;
	float shipRadius = 70.f;
};

struct GameOverScreenInfo
{
	bool gameWon = false;
	std::string text = "You died";
	Vec2 titlePos;
	Rgba8 titleColor;
	float textCellSize = 50.f;
	float titleOpacity = 0.f;
};

enum class StarShipSFX
{
	ENTER_GAME,
	FIRE_BULLET,
	ENGINE_THRUST,
	PLAYER_DAMAGED,
	PLAYER_DEATH,
	PLAYER_RESPAWN,
	ENEMY_DEATH,
	NEW_ENEMY_WAVE,
	ASTEROID_HIT,
	ASTEROID_BROKEN,
	POWERUP,
	GAME_OVER_VICTORY,
	GAME_OVER_DEFEAT,
	NUM_SFX,
};

enum class StarShipMusic
{
	ATTRACT_SCREEN_MUSIC,
	GAME_MUSIC,
	PLAYER_SHIP_ENGINE_THRUST,
	NUM_MUSIC,
};

class Game
{
public:
	Game();
	~Game();

	//Game Flow Management
	void Startup();
	void Shutdown();
	void BeginFrame();
	void Update();
	void Render() const;
	void EndFrame();

	void GameOver(int const& playerNum, bool const& gameWon);

	//Input and Debug
	void CheckKeyboardInputs();
	void CheckControllerInputs();
	void ToggleEntityDebugDraw();
	static bool Event_ShowGameControls(EventArgs& args);
	static bool Event_TimeScale(EventArgs& args);
	static bool Event_DebugDraw(EventArgs& args);
	static bool Event_Restart(EventArgs& args);

	//Public Spawn Functions
	void SpawnNewBullet(Vec2 const& position, float const& orientationDegrees, int const& playerID);
	void SpawnNewSpecialBullet(Vec2 const& position, float const& orientationDegrees, int const& playerID, PowerUpTypes const& bulletType);
	void SpawnNewDebrisCluster(Vec2 const& position, int numDebris, Vec2 const& averageVelocity, float spraySpeed, float averageRadius, Rgba8 const& color);
	void SpawnNewPowerUp(Vec2 const& position);

	//Music and SFX
	void const PlayGameSFX(StarShipSFX soundEffect) const;
	void const PlayGameSFX(StarShipSFX soundEffect, Vec2 const& worldPosition);
	void const PlayGameMusic(StarShipMusic musicTrack, bool loop);
	void const PlayGameMusic(SoundPlaybackID& soundPlayBackID, StarShipMusic musicTrack, bool loop);
	void const StopGameMusic(StarShipMusic musicTrack);
	void const StopGameMusic(SoundPlaybackID soundPlaybackID);
	float GetAudioBalanceFromWorldPosition(Vec2 const& inPosition) const;
	void const SetAudioBalanceAndVolumeFromWorldPosition(SoundPlaybackID& sound, Vec2 const& worldPos); // mutes sound if it is off screen

	//Screen Shake
	void StartScreenShake(float duration, float shakeTrauma);

	//Player Data
	Vec2 const GetNearestPlayerPosition(Vec2 const& inPosition);
	bool const AllPlayersDead() const;
	PlayerShip* GetPlayerShipByID(int idNum) const;
	int GetPlayerNumFromPlayerID(int idNum) const;
	void CheckNumRemainingPlayersForGameOver();

private:
	//Initialization
	void InitPlayerData();
	void InitEnemyWaveData();
	void InitStarLocations();
	void InitAttractScreen();
	void InitGameOverScreen(int const& winningPlayerNum, bool const& gameWon);
	void LoadAllAudioAssets() const;

	//Game Start Flow
	void UpdateAttractScreen(float deltaSeconds);
	void GoToPlayerConnectionLobby();
	void ConnectNewPlayer(int playerID);
	void CheckIfAllPlayersReady();
	void StartGame();

	//Update
	void ManageConditionalGameStateUpdates();

	//Render Functions
	void ManageConditionalGameStateWorldRenders() const;
	void ManageConditionalGameStateScreenRenders() const;
	void RenderAttractScreen() const;
	void RenderInstructionsScreen() const;
	void RenderPlayerConnectionLobby() const;
	void RenderPlayers() const;
	void RenderAllEntities() const;
	void RenderPlayerLives() const;
	void RenderPlayerHealth() const;
	void RenderPlayerPowerUpTimer() const;
	void RenderEnemyWaveData() const;
	void RenderGameOverScreen() const;

	//Spawn Functions
	void SpawnNextEnemyWave();
	void SpawnRandomizedEnemyWave();
	void SpawnAsteroid();
	void SpawnBeetle();
	void SpawnWasp();
	void SpawnNewDebris(Vec2 const& position, Vec2 const& velocity, float const& averageRadius, Rgba8 const& color);

	//Camera Management
	void UpdateCameras(float deltaSeconds);

	//Entity Management
	void UpdatePlayers(float deltaSeconds);
	void UpdateNonPlayerEntities(float deltaSeconds);
	void DeleteGarbageEntities();

	//Enemy Waves Management
	void ClearEnemyWave();

	//Collision
	void CheckAllEntityCollisions();
	void CheckBulletCollisions();
	void CheckPlayerCollisions();
	void CheckEnemyCollisions();

	//Input
	void RotateThroughAttractScreenButtons(bool const& downDirection = true);
	void PrintControlsToDevConsole();
	void AdjustTimeScale(float scale);

	//Helpers
	void AdjustTimeDistortion();
	Vec2 const GetRandomPointOutsideScreen(float const& offset) const;

public:
	//Player management
	PlayerShip* m_playerShips[MAX_NUM_PLAYERS] = {};
	PlayerShip* m_firstPlayerShip;
	int m_numExtraLives[MAX_NUM_PLAYERS] = {};
	
	//Debug
	bool m_shouldDrawDebug = false;

	//Game States
	bool m_inGameplay = false;
	bool m_inGameOverSequence = false;
	bool m_inMultiplayerMode = true;
	bool m_inCoOpMode = false;

	int m_numEnemies = 0;

private:

	//Camera
	Camera* m_worldCamera;
	Camera* m_screenCamera;
	Vec2 m_worldCamBottomLeft;
	Vec2 m_worldCamTopRight;

	//Entities
	Asteroid* m_asteroids[MAX_ASTEROIDS] = {};
	Bullet* m_bullets[MAX_BULLETS] = {};
	Debris* m_debris[MAX_DEBRIS] = {};
	Beetle* m_beetles[MAX_BEETLES] = {};
	Wasp* m_wasps[MAX_WASPS] = {};
	Star* m_stars[MAX_STARS] = {};
	PowerUp* m_powerUps[MAX_POWERUPS] = {};

	//Game States
	bool m_isPaused = false;
	bool m_isSlowMo = false;
	bool m_inAttractMode = true;
	bool m_inPlayerConnectionLobby = false;
	bool m_inInstructionsScreen = false;

	//Debug
	bool m_shouldRestart = false;

	//Debris
	IntVec2 m_smallDebrisAmountRange = IntVec2(1, 3);
	float m_smallDebrisVelocityScale = -0.15f;

	//Enemy waves
	int m_currentWave = 0;
	EnemyWaveInfo m_enemyWavesInfo[NUM_PLANNED_ENEMY_WAVES] = {};
	int m_numEnemiesInCurrentWave = 0;

	//Player Initialization
	Vec2 m_playerSpawnLocations[MAX_NUM_PLAYERS] = {};
	float m_playerSpawnRotation[MAX_NUM_PLAYERS] = { -45.f, 135.f, 45.f, 225.f };
	Vec2 m_playerLivesScreenLocation[MAX_NUM_PLAYERS] = {};
	Rgba8 m_playerColors[MAX_NUM_PLAYERS] = {};

	//Screen Shake
	bool m_inScreenShake = false;
	float m_screenShakeElapsedTime = 0.f;
	float m_screenShakeDuration = 0.f;
	float m_screenShakeTrauma = 0.f;

	//Sound IDs
	SoundPlaybackID m_attractScreenMusic;
	SoundPlaybackID m_gameMusic;

	//Player Connection
	int m_numConnectedPlayers = 0;
	bool m_readyPlayers[MAX_NUM_PLAYERS] = { false, false, false, false };

	//Attract screen
	AttractScreenInfo m_attractScreenInfo;
	AttractScreenButtons m_selectedAttractScreenButton = AttractScreenButtons::SINGLE_PLAYER;

	//Game Over screen
	GameOverScreenInfo m_gameOverInfo;

	Rgba8 m_standardBodyTextColor;

	Clock* m_clock = nullptr;
	Timer* m_gameOverTimer = nullptr;

};

