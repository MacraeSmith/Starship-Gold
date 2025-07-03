#include "Game/Game.hpp"

#include <Engine/Core/ErrorWarningAssert.hpp>

#include "Engine/Renderer/RendererDX11.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"

#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Bullet.hpp"
#include "Game/Asteroid.hpp"
#include "Game/Debris.hpp"
#include "Game/Debris.hpp"
#include "Game/Beetle.hpp"
#include "Game/Wasp.hpp"
#include "Game/Star.hpp"
#include "Game/PowerUp.hpp"

RandomNumberGenerator* g_rng;
extern Game* m_game;

Game::Game()
{
 	m_worldCamera = new Camera();
	m_screenCamera = new Camera();
	g_rng = new RandomNumberGenerator();

	m_worldCamBottomLeft = Vec2(0.f, 0.f);
	m_worldCamTopRight = Vec2(WORLD_SIZE_X, WORLD_SIZE_Y);
	m_standardBodyTextColor = Rgba8(164, 164, 164, 255);

	LoadAllAudioAssets();
	InitPlayerData();
	InitEnemyWaveData();
	InitStarLocations();
	InitAttractScreen();
	PlayGameMusic(StarShipMusic::ATTRACT_SCREEN_MUSIC, true);
	m_clock = new Clock(Clock::GetSystemClock());

	g_eventSystem->SubscribeEventCallbackFunction("Controls", Game::Event_ShowGameControls);
	Strings timeScaleArguments;
	timeScaleArguments.push_back("Scale=");
	timeScaleArguments.push_back("Scale=1.0");
	g_eventSystem->SubscribeEventCallbackFunction("TimeScale", timeScaleArguments, Game::Event_TimeScale);
	g_eventSystem->SubscribeEventCallbackFunction("DebugDraw", Game::Event_DebugDraw);
	g_eventSystem->SubscribeEventCallbackFunction("Restart", Game::Event_Restart);
	PrintControlsToDevConsole();
	
}

Game::~Game()
{
	g_audioSystem->StopSound(m_gameMusic);
	g_audioSystem->StopSound(m_attractScreenMusic);

	delete m_clock;
	m_clock = nullptr;
	delete m_worldCamera;
	m_worldCamera = nullptr;
	delete m_screenCamera;
	m_screenCamera = nullptr;
	delete g_rng;
	g_rng = nullptr;

	
	//delete all entities
	for (int bulletNum = 0; bulletNum < MAX_BULLETS; ++bulletNum)
	{
		if (m_bullets[bulletNum] == nullptr) //skip index if element is a nullptr
			continue;

		delete(m_bullets[bulletNum]);
		m_bullets[bulletNum] = nullptr;
	}

	for (int asteroidNum = 0; asteroidNum < MAX_ASTEROIDS; ++asteroidNum)
	{
		if (m_asteroids[asteroidNum] == nullptr)
			continue;

		delete(m_asteroids[asteroidNum]);
		m_asteroids[asteroidNum] = nullptr;
	}

	for (int beetleNum = 0; beetleNum < MAX_BEETLES; ++beetleNum)
	{
		if (m_beetles[beetleNum] == nullptr)
			continue;

		delete(m_beetles[beetleNum]);
		m_beetles[beetleNum] = nullptr;
	}

	for (int waspNum = 0; waspNum < MAX_WASPS; ++waspNum)
	{
		if (m_wasps[waspNum] == nullptr)
			continue;

		delete(m_wasps[waspNum]);
		m_wasps[waspNum] = nullptr;
	}

	for (int playerNum = 0; playerNum < MAX_NUM_PLAYERS; ++playerNum)
	{
		if (m_playerShips[playerNum] == nullptr)
			continue;

		delete(m_playerShips[playerNum]);
		m_playerShips[playerNum] = nullptr;
	}

	for (int starNum = 0; starNum < MAX_STARS; ++starNum)
	{
		if (m_stars[starNum] == nullptr)
			continue;

		delete(m_stars[starNum]);
		m_stars[starNum] = nullptr;
	}

	for (int powerUpNum = 0; powerUpNum < MAX_POWERUPS; ++powerUpNum)
	{
		if (m_powerUps[powerUpNum] == nullptr)
			continue;

		delete(m_powerUps[powerUpNum]);
		m_powerUps[powerUpNum] = nullptr;
	}
}

//Game management
//--------------------------------------------------------------------
void Game::Startup()
{
	
}

void Game::Shutdown()
{

}

//Frame Flow
//--------------------------------------------------------------------
void Game::BeginFrame()
{
	m_worldCamera->SetOrthoView(m_worldCamBottomLeft, m_worldCamTopRight);
	m_screenCamera->SetOrthoView(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
}

void Game::Update()
{
	CheckKeyboardInputs();
	CheckControllerInputs();
	ManageConditionalGameStateUpdates();
}

void Game::Render() const
{
	g_renderer->ClearScreen(Rgba8::BLACK);

	//World Camera
	g_renderer->BeginCamera(*m_worldCamera);
	ManageConditionalGameStateWorldRenders();
	g_renderer->EndCamera(*m_worldCamera);

	//Screen Camera
	g_renderer->BeginCamera(*m_screenCamera);
	ManageConditionalGameStateScreenRenders();
	g_devConsole->Render(m_screenCamera);
	g_renderer->EndCamera(*m_screenCamera);
}

void Game::EndFrame()
{
	DeleteGarbageEntities();
}

//Input
//--------------------------------------------------------------------
void Game::CheckKeyboardInputs()
{
	if (m_inGameOverSequence)
		return;

	//Spacebar
	if (g_inputSystem->WasKeyJustPressed(' '))
	{
		if (m_inAttractMode)
		{
			switch (m_selectedAttractScreenButton)
			{
			case AttractScreenButtons::SINGLE_PLAYER:
				m_inMultiplayerMode = false;
				ConnectNewPlayer(-1);
				StartGame();
				break;

			case AttractScreenButtons::CO_OP:
				m_inMultiplayerMode = true;
				m_inCoOpMode = true;
				GoToPlayerConnectionLobby();
				break;

			case AttractScreenButtons::VERSUS:
				m_inMultiplayerMode = true;
				m_inCoOpMode = false;
				GoToPlayerConnectionLobby();
				break;

			case AttractScreenButtons::INSTRUCTIONS:
				m_inAttractMode = false;
				m_inPlayerConnectionLobby = false;
				m_inInstructionsScreen = true;
				break;
			}
		}

		else if (m_inPlayerConnectionLobby)
		{
			if (m_numConnectedPlayers > 0)
			{
				int playerNum = GetPlayerNumFromPlayerID(-1);
				if (playerNum >= 0)
				{
					m_readyPlayers[GetPlayerNumFromPlayerID(-1)] = true;
					CheckIfAllPlayersReady();
				}
			}
		}
	}

	//N button
	if (g_inputSystem->WasKeyJustPressed('N'))
	{
		if (m_inAttractMode)
		{
			switch (m_selectedAttractScreenButton)
			{
			case AttractScreenButtons::SINGLE_PLAYER:
				m_inMultiplayerMode = false;
				ConnectNewPlayer(-1);
				StartGame();
				break;

			case AttractScreenButtons::CO_OP:
				m_inMultiplayerMode = true;
				m_inCoOpMode = true;
				GoToPlayerConnectionLobby();
				break;

			case AttractScreenButtons::VERSUS:
				m_inMultiplayerMode = true;
				m_inCoOpMode = false;
				GoToPlayerConnectionLobby();
				break;

			case AttractScreenButtons::INSTRUCTIONS:
				m_inAttractMode = false;
				m_inPlayerConnectionLobby = false;
				m_inInstructionsScreen = true;
				break;
			}
		}

		else if (m_inPlayerConnectionLobby)
		{
			ConnectNewPlayer(-1);
		}
	}

	//Up arrow
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_UPARROW))
	{
		if (m_inAttractMode)
		{
			RotateThroughAttractScreenButtons(false);
		}
	}

	//down arrow
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_DOWNARROW))
	{
		if (m_inAttractMode)
		{
			RotateThroughAttractScreenButtons(true);
		}
	}

	//Escape
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_ESC))
	{
		if (m_inAttractMode)
		{
			g_app->HandleQuitRequested();
		}

		//Return to attract screen
		else
		{
			m_shouldRestart = true;
		}
	}

	//Pause
	if (g_inputSystem->WasKeyJustPressed('P') )
	{
		m_isPaused = !m_isPaused;
		m_clock->TogglePause();
	}

	//SloMo
	if (g_inputSystem->WasKeyJustPressed('T'))
	{
		m_clock->SetTimeScale(1.f);
		m_isSlowMo = !m_isSlowMo;
		if (m_isSlowMo)
			m_clock->SetTimeScale(0.1f);
	}

	//Move one Frame
	if (g_inputSystem->WasKeyJustPressed('O') )
	{
		m_clock->StepSingleFrame();
	}

	//Toggle Debug
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F1) ) //F1 key
	{
		ToggleEntityDebugDraw();
	}

	//Restart Game
	if (g_inputSystem->WasKeyJustPressed(KEYCODE_F8)) //F8 key
	{
		m_shouldRestart = true;
	}

	//Skip the rest of the controls unless you are in gameplay
	if (m_inAttractMode || m_inPlayerConnectionLobby)
		return;
		 
	//Spawn Asteroid
	if (g_inputSystem->WasKeyJustPressed('I'))
	{
		SpawnAsteroid();
	}

	//Spawn Beetle
	if (g_inputSystem->WasKeyJustPressed('B'))
	{
		SpawnBeetle();
	}

	//Spawn Wasp
	if (g_inputSystem->WasKeyJustPressed('V'))
	{
		SpawnWasp();
	}

	//Clear Wave
	if (g_inputSystem->WasKeyJustPressed('C'))
	{
		ClearEnemyWave();
	}

}

void Game::CheckControllerInputs()
{
	if (m_inGameOverSequence)
		return;

	if (!m_inAttractMode && !m_inPlayerConnectionLobby && !m_inInstructionsScreen)
		return;

	for (int controllerNum = 0; controllerNum < MAX_NUM_PLAYERS; ++controllerNum)
	{
		XboxController currentController = g_inputSystem->GetController(controllerNum);

		//A button
		if (currentController.WasButtonJustPressed(XboxButtonID::BUTTON_A))
		{
			if (m_inAttractMode)
			{
				switch (m_selectedAttractScreenButton)
				{
				case AttractScreenButtons::SINGLE_PLAYER:
					m_inMultiplayerMode = false;
					ConnectNewPlayer(controllerNum);
					StartGame();
					break;

				case AttractScreenButtons::CO_OP:
					m_inMultiplayerMode = true;
					m_inCoOpMode = true;
					GoToPlayerConnectionLobby();
					break;

				case AttractScreenButtons::VERSUS:
					m_inMultiplayerMode = true;
					m_inCoOpMode = false;
					GoToPlayerConnectionLobby();
					break;

				case AttractScreenButtons::INSTRUCTIONS:
					m_inAttractMode = false;
					m_inPlayerConnectionLobby = false;
					m_inInstructionsScreen = true;
					break;
				}
				
			}

			else if (m_inPlayerConnectionLobby)
			{
				if (m_numConnectedPlayers > 0)
				{
					int playerNum = GetPlayerNumFromPlayerID(controllerNum);
					if (playerNum >= 0)
					{
						m_readyPlayers[GetPlayerNumFromPlayerID(controllerNum)] = true;
						CheckIfAllPlayersReady();
					}
				}
			}
		}

		//Start Button
		if (currentController.WasButtonJustPressed(XboxButtonID::BUTTON_START))
		{
			if (m_inPlayerConnectionLobby)
			{
				ConnectNewPlayer(controllerNum);
			}
		}

		//B Button
		if (currentController.WasButtonJustPressed(XboxButtonID::BUTTON_B))
		{
			if (m_inPlayerConnectionLobby || m_inInstructionsScreen)
			{
				m_shouldRestart = true;
			}
		}

		//Up arrow
		if (currentController.WasButtonJustPressed(XboxButtonID::BUTTON_DPAD_UP))
		{
			if (m_inAttractMode)
			{
				RotateThroughAttractScreenButtons(false);
			}
		}

		//down arrow
		if (currentController.WasButtonJustPressed(XboxButtonID::BUTTON_DPAD_DOWN))
		{
			if (m_inAttractMode)
			{
				RotateThroughAttractScreenButtons(true);
			}
		}
	}
}

void Game::RotateThroughAttractScreenButtons(bool const& downDirection)
{
	if (downDirection)
	{
		switch (m_selectedAttractScreenButton)
		{
		case AttractScreenButtons::SINGLE_PLAYER:
			m_selectedAttractScreenButton = AttractScreenButtons::CO_OP;
			break;

		case AttractScreenButtons::CO_OP:
			m_selectedAttractScreenButton = AttractScreenButtons::VERSUS;
			break;

		case AttractScreenButtons::VERSUS:
			m_selectedAttractScreenButton = AttractScreenButtons::INSTRUCTIONS;
			break;

		case AttractScreenButtons::INSTRUCTIONS:
			m_selectedAttractScreenButton = AttractScreenButtons::SINGLE_PLAYER;
			break;

		default:
			m_selectedAttractScreenButton = AttractScreenButtons::SINGLE_PLAYER;
			break;
		}
	}

	else
	{
		switch (m_selectedAttractScreenButton)
		{
		case AttractScreenButtons::SINGLE_PLAYER:
			m_selectedAttractScreenButton = AttractScreenButtons::INSTRUCTIONS;
			break;

		case AttractScreenButtons::CO_OP:
			m_selectedAttractScreenButton = AttractScreenButtons::SINGLE_PLAYER;
			break;

		case AttractScreenButtons::VERSUS:
			m_selectedAttractScreenButton = AttractScreenButtons::CO_OP;
			break;

		case AttractScreenButtons::INSTRUCTIONS:
			m_selectedAttractScreenButton = AttractScreenButtons::VERSUS;
			break;

		default:
			m_selectedAttractScreenButton = AttractScreenButtons::SINGLE_PLAYER;
			break;
		}
	}

}

void Game::PrintControlsToDevConsole()
{
	g_devConsole->AddLine(Rgba8::YELLOW, "--Star Ship Controls--", 1.f, true);
	g_devConsole->AddLine(DevConsole::INFO_MINOR, "Navigate Menu - Arrow Keys", 0.75f, true);
	g_devConsole->AddLine(DevConsole::INFO_MINOR, "Select - SPACEBAR", 0.75f, true);
	g_devConsole->AddLine(DevConsole::INFO_MINOR, "Move - W and A", 0.75f, true);
	g_devConsole->AddLine(DevConsole::INFO_MINOR, "Rotate - S and D", 0.75f, true);
	g_devConsole->AddLine(DevConsole::INFO_MINOR, "Fire - SPACEBAR", 0.75f, true);
	g_devConsole->AddLine(DevConsole::INFO_MINOR, "Pause - P", 0.75f, true);
	g_devConsole->AddLine(DevConsole::INFO_MINOR, "SlowMo - T", 0.75f, true);
	g_devConsole->AddLine(DevConsole::INFO_MINOR, "Step - O", 0.75f, true);
	g_devConsole->AddLine(DevConsole::INFO_MINOR, "Quit - ESC", 0.75f, true);
}

void Game::AdjustTimeScale(float scale)
{
	if (m_clock == nullptr)
		return;

	m_clock->SetTimeScale(scale);
}

bool Game::Event_ShowGameControls(EventArgs& args)
{
	UNUSED(args);
	if (m_game != nullptr)
	{
		m_game->PrintControlsToDevConsole();
		return true;
	}
	return false;
}

bool Game::Event_TimeScale(EventArgs& args)
{
	if (!args.HasKey("Scale") && g_devConsole)
	{
		g_devConsole->AddLine(DevConsole::WARNING, "WARNING: Missing or Incorrect Argument 'Scale=___'", 0.5f, true);
		return true;
	}
	float scale = args.GetValue("Scale", -1.f);
	if (m_game != nullptr)
	{
		m_game->AdjustTimeScale(scale);
		return true;
	}
	return false;
}

bool Game::Event_DebugDraw(EventArgs& args)
{
	UNUSED(args);
	if (m_game)
	{
		m_game->m_shouldDrawDebug = !m_game->m_shouldDrawDebug;
		return true;
	}
	return false;
}

bool Game::Event_Restart(EventArgs& args)
{
	UNUSED(args);
	if (m_game)
	{
		m_game->m_shouldRestart = true;
		return true;
	}
	return false;
}

//Debug
//--------------------------------------------------------------------
void Game::ToggleEntityDebugDraw()
{
	m_shouldDrawDebug = !m_shouldDrawDebug;
}

//Gameplay management
//--------------------------------------------------------------------
void Game::GoToPlayerConnectionLobby()
{
	m_inAttractMode = false;
	m_inPlayerConnectionLobby = true;

	m_worldCamBottomLeft = Vec2(60.f, 30.f);
	m_worldCamTopRight = Vec2(140.f, 70.f);
}

void Game::ConnectNewPlayer(int playerID)
{
	for (int playerNum = 0; playerNum < MAX_NUM_PLAYERS; ++playerNum)
	{
		if (m_playerShips[playerNum] == nullptr)
			continue;

		if (m_playerShips[playerNum]->m_playerID == playerID) //if player has already been added, return from function
			return;
	}

	//Create new ship and correctly assign it to array
	m_playerShips[m_numConnectedPlayers] = 
		new PlayerShip(this, m_playerSpawnLocations[m_numConnectedPlayers], m_playerSpawnRotation[m_numConnectedPlayers],
		m_playerColors[m_numConnectedPlayers], m_numConnectedPlayers, playerID);
	
	m_numExtraLives[m_numConnectedPlayers] = PLAYER_SHIP_NUM_STARTING_LIVES - 1;

	if (m_numConnectedPlayers == 0)
	{
		m_firstPlayerShip = m_playerShips[0];
	}

	m_numConnectedPlayers++;
}

void Game::CheckIfAllPlayersReady()
{
	if (m_numConnectedPlayers <= 1)
		return;

	int numReadyPlayers = 0;

	for (int playerNum = 0; playerNum < MAX_NUM_PLAYERS; ++playerNum)
	{
		if (m_readyPlayers[playerNum])
		{
			numReadyPlayers++;
		}
	}

	if (numReadyPlayers >= m_numConnectedPlayers)
	{
		StartGame();
	}
}

void Game::StartGame()
{
	m_inGameplay = true;
	m_inAttractMode = false;
	m_inPlayerConnectionLobby = false;

	m_worldCamBottomLeft = Vec2(0.f, 0.f);
	m_worldCamTopRight = Vec2(WORLD_SIZE_X, WORLD_SIZE_Y);

	PlayGameSFX(StarShipSFX::ENTER_GAME);
	StopGameMusic(StarShipMusic::ATTRACT_SCREEN_MUSIC);
	PlayGameMusic(StarShipMusic::GAME_MUSIC, true);

	SpawnNextEnemyWave();

	for (int playerNum = 0; playerNum < MAX_NUM_PLAYERS; ++playerNum)
	{
		if (m_playerShips[playerNum] == nullptr)
			continue;

		m_playerShips[playerNum]->ToggleShield(true, 5.f);
	}
}

void Game::UpdateAttractScreen(float deltaSeconds)
{
	m_attractScreenInfo.rightShipPos.y -= m_attractScreenInfo.rightShipSpeed * deltaSeconds;
	m_attractScreenInfo.leftShipPos.y -= m_attractScreenInfo.leftShipSpeed * deltaSeconds;

	if (m_attractScreenInfo.rightShipPos.y < 0.f - m_attractScreenInfo.shipRadius)
	{
		m_attractScreenInfo.rightShipPos.y = SCREEN_SIZE_Y + m_attractScreenInfo.shipRadius;
		m_attractScreenInfo.rightShipColor = m_playerColors[g_rng->RollRandomIntInRange(0, MAX_NUM_PLAYERS - 1)];
		m_attractScreenInfo.rightShipSpeed = g_rng->RollRandomFloatInRange(100.f, 300.f);
	}

	if (m_attractScreenInfo.leftShipPos.y < 0.f - m_attractScreenInfo.shipRadius)
	{
		m_attractScreenInfo.leftShipPos.y = SCREEN_SIZE_Y + m_attractScreenInfo.shipRadius;
		m_attractScreenInfo.leftShipColor = m_playerColors[g_rng->RollRandomIntInRange(0, MAX_NUM_PLAYERS - 1)];
		m_attractScreenInfo.rightShipSpeed = g_rng->RollRandomFloatInRange(100.f, 300.f);
	}
}

void Game::GameOver(int const& playerNum, bool const& gameWon)
{
	InitGameOverScreen(playerNum, gameWon);
	m_inGameOverSequence = true;

	//#TODO: fix bug where time reverses
	m_gameOverTimer = new Timer(GAME_OVER_SEQUENCE_DURATION, &Clock::GetSystemClock());
	m_gameOverTimer->Start();

	StopGameMusic(m_gameMusic);
	StopGameMusic(m_attractScreenMusic);
}

//Camera Management
//--------------------------------------------------------------------
void Game::UpdateCameras(float deltaSeconds)
{
	if (!m_inScreenShake)
	{
		m_worldCamera->SetOrthoView(m_worldCamBottomLeft, m_worldCamTopRight);
		return;
	}
	
	float currentTrauma = Lerp(m_screenShakeTrauma, 0.f, GetFractionWithinRange(m_screenShakeElapsedTime, 0.f, m_screenShakeDuration));
	float screenShake = currentTrauma * currentTrauma;
	float screenOffset = g_rng->RollRandomFloatInRange(-screenShake, screenShake);
	
	m_screenShakeElapsedTime += deltaSeconds;
	if (m_screenShakeElapsedTime >= m_screenShakeDuration)
	{
		m_inScreenShake = false;
		m_screenShakeElapsedTime = 0.f;
	}

	m_worldCamera->SetOrthoView(Vec2(0.f, 0.f), Vec2((WORLD_SIZE_X + screenOffset), (WORLD_SIZE_Y + screenOffset)));
}

void Game::StartScreenShake(float duration, float shakeTrauma)
{
	m_screenShakeTrauma = GetClamped(shakeTrauma, -MAX_SCREENSHAKE_TRAUMA, MAX_SCREENSHAKE_TRAUMA);
	m_screenShakeElapsedTime = 0.f;
	m_screenShakeDuration = duration;
	m_inScreenShake = true;
}

//Initialization
//--------------------------------------------------------------------
void Game::InitPlayerData()
{
	m_playerSpawnLocations[0] = Vec2(80.f, 60.f);
	m_playerSpawnLocations[1] = Vec2(120.f, 40.f);
	m_playerSpawnLocations[2] = Vec2(80.f, 40.f);
	m_playerSpawnLocations[3] = Vec2(120.f, 60.f);

	m_playerColors[0] = Rgba8(102, 153, 204, 255);
	m_playerColors[1] = Rgba8(255, 0, 0, 255);
	m_playerColors[2] = Rgba8(128, 255, 0, 255);
	m_playerColors[3] = Rgba8(204, 0, 204, 255);

	m_playerLivesScreenLocation[0] = Vec2(50.f, SCREEN_SIZE_Y - 50.f); //top left
	m_playerLivesScreenLocation[1] = Vec2(SCREEN_SIZE_X - 50.f, 50.f); //bottom right
	m_playerLivesScreenLocation[2] = Vec2(50.f, 50.f); //bottom left
	m_playerLivesScreenLocation[3] = Vec2(SCREEN_SIZE_X - 50.f, SCREEN_SIZE_Y - 50.f); //top right
}

void Game::InitEnemyWaveData()
{
	//Wave 1
	m_enemyWavesInfo[0].numBeetles = 1;
	m_enemyWavesInfo[0].numWasps = 0;
	m_enemyWavesInfo[0].numAsteroids = 4;

	//Wave 2
	m_enemyWavesInfo[1].numBeetles = 2;
	m_enemyWavesInfo[1].numWasps = 0;
	m_enemyWavesInfo[1].numAsteroids = 1;

	//Wave 3
	m_enemyWavesInfo[2].numBeetles = 2;
	m_enemyWavesInfo[2].numWasps = 1;
	m_enemyWavesInfo[2].numAsteroids = 2;

	//Wave 4
	m_enemyWavesInfo[3].numBeetles = 4;
	m_enemyWavesInfo[3].numWasps = 1;
	m_enemyWavesInfo[3].numAsteroids = 2;

	//Wave 5
	m_enemyWavesInfo[4].numBeetles = 4;
	m_enemyWavesInfo[4].numWasps = 2;
	m_enemyWavesInfo[4].numAsteroids = 3;
}

void Game::InitStarLocations()
{
	for (int starNum = 0; starNum < MAX_STARS; ++starNum)
	{
		Vec2 randPos;
		randPos.x = g_rng->RollRandomFloatInRange(0.f, WORLD_SIZE_X);
		randPos.y = g_rng->RollRandomFloatInRange(0.f, WORLD_SIZE_Y);

		float randScale = g_rng->RollRandomFloatInRange(.25f, 1.5f);
		m_stars[starNum] = new Star(this, randPos, 0.f, randScale);
	}
}

void Game::InitAttractScreen()
{
	m_attractScreenInfo.defaultButtonColor = Rgba8(102, 153, 204, 255);
	m_attractScreenInfo.selectedButtonColor = Rgba8(255, 0, 0, 255);

	m_attractScreenInfo.buttons[0].buttonBounds = AABB2(600.f, 350.f, 1000.f, 400.f);
	m_attractScreenInfo.buttons[1].buttonBounds = AABB2(600.f, 250.f, 1000.f, 300.f);
	m_attractScreenInfo.buttons[2].buttonBounds = AABB2(600.f, 150.f, 1000.f, 200.f);
	m_attractScreenInfo.buttons[3].buttonBounds = AABB2(600.f, 50.f, 1000.f, 100.f);

	m_attractScreenInfo.buttons[0].buttonText = "Single Player";
	m_attractScreenInfo.buttons[1].buttonText = "Co-Op";
	m_attractScreenInfo.buttons[2].buttonText = "Versus";
	m_attractScreenInfo.buttons[3].buttonText = "How To Play";

	m_attractScreenInfo.buttons[0].buttonType = AttractScreenButtons::SINGLE_PLAYER;
	m_attractScreenInfo.buttons[1].buttonType = AttractScreenButtons::CO_OP;
	m_attractScreenInfo.buttons[2].buttonType = AttractScreenButtons::VERSUS;
	m_attractScreenInfo.buttons[3].buttonType = AttractScreenButtons::INSTRUCTIONS;

	for (int buttonNum = 0; buttonNum < NUM_ATTRACT_BUTTONS; ++buttonNum)
	{
		Button& currentButton = m_attractScreenInfo.buttons[buttonNum];
		currentButton.textColor = m_standardBodyTextColor;

		float textOffset = GetSimpleTriangleStringWidth(currentButton.buttonText, currentButton.textCellSize) * 0.5f;
		float textPosX = Lerp(currentButton.buttonBounds.m_mins.x, currentButton.buttonBounds.m_maxs.x, 0.5f) - textOffset;
		float textPosY = Lerp(currentButton.buttonBounds.m_mins.y, currentButton.buttonBounds.m_maxs.y, 0.2f);
		currentButton.textPos = Vec2(textPosX, textPosY);
	}

	m_attractScreenInfo.rightShipPos = Vec2(SCREEN_CENTER_X + 650.f, SCREEN_CENTER_Y);
	m_attractScreenInfo.leftShipPos = Vec2(SCREEN_CENTER_X - 650.f, SCREEN_CENTER_Y);

	m_attractScreenInfo.rightShipColor = m_playerColors[g_rng->RollRandomIntInRange(0, MAX_NUM_PLAYERS)];
	m_attractScreenInfo.leftShipColor = m_playerColors[g_rng->RollRandomIntInRange(0, MAX_NUM_PLAYERS)];
}

void Game::InitGameOverScreen(int const& winningPlayerNum, bool const& gameWon)
{
	if (gameWon)
	{
		m_gameOverInfo.text = "Player " + std::to_string(winningPlayerNum + 1) + " Wins!";
		m_gameOverInfo.titleColor = m_playerColors[winningPlayerNum];
		m_gameOverInfo.titleColor.a = static_cast<unsigned char>(m_gameOverInfo.titleOpacity);
	}

	else
	{
		m_gameOverInfo.text = "You Died!";
		m_gameOverInfo.titleColor = Rgba8(255, 0, 0, static_cast<unsigned char>(m_gameOverInfo.titleOpacity));
	}

	float textOffset = GetSimpleTriangleStringWidth(m_gameOverInfo.text, m_gameOverInfo.textCellSize) * 0.5f;
	m_gameOverInfo.titlePos = Vec2(SCREEN_CENTER_X - textOffset, SCREEN_CENTER_Y + 200.f);
}

void Game::LoadAllAudioAssets() const
{
	//Music tracks
	g_audioSystem->CreateOrGetSound("Data/Audio/Music/AttractScreenMusic.mp3");
	g_audioSystem->CreateOrGetSound("Data/Audio/Music/GameMusic.mp3");
	g_audioSystem->CreateOrGetSound("Data/Audio/Music/PlayerShipEngineThrust.wav");

	//SFX
	g_audioSystem->CreateOrGetSound("Data/Audio/SFX/EnterGame.mp3");
	g_audioSystem->CreateOrGetSound("Data/Audio/SFX/FireBullet.wav");
	g_audioSystem->CreateOrGetSound("Data/Audio/SFX/PlayerDamaged.wav");
	g_audioSystem->CreateOrGetSound("Data/Audio/SFX/PlayerDeath.wav");
	g_audioSystem->CreateOrGetSound("Data/Audio/SFX/PlayerRespawn.mp3");
	g_audioSystem->CreateOrGetSound("Data/Audio/SFX/EnemyDeath.wav");
	g_audioSystem->CreateOrGetSound("Data/Audio/SFX/NewEnemyWave.mp3");
	g_audioSystem->CreateOrGetSound("Data/Audio/SFX/AsteroidHit.mp3");
	g_audioSystem->CreateOrGetSound("Data/Audio/SFX/AsteroidBroken.wav");
	g_audioSystem->CreateOrGetSound("Data/Audio/SFX/PowerUp.wav");


}

//Update functions
//--------------------------------------------------------------------
void Game::ManageConditionalGameStateUpdates()
{
	float deltaSeconds = m_clock->GetDeltaSeconds(); //#TODO handle gameOverCountdown with timer
	if (m_inGameOverSequence)
	{
		m_gameOverInfo.titleOpacity = RangeMapClamped((float)m_gameOverTimer->GetElapsedTime(), 0.f, GAME_OVER_SEQUENCE_DURATION, 0.f, 255.f);
		m_gameOverInfo.titleColor.a = static_cast<unsigned char>(m_gameOverInfo.titleOpacity);
		if (m_gameOverTimer->DecrementPeriodIfElapsed())
		{
			m_shouldRestart = true;
		}
	}

	AdjustTimeDistortion();// changes speed of simulation

	deltaSeconds = m_clock->GetDeltaSeconds();
	if (!m_inAttractMode)
	{
		UpdatePlayers(deltaSeconds);

		if (!m_inPlayerConnectionLobby)
		{
			UpdateNonPlayerEntities(deltaSeconds);
			CheckAllEntityCollisions();
		}
	}

	else
	{
		UpdateAttractScreen(deltaSeconds);
	}


	if (deltaSeconds > 0.f)
	{
		UpdateCameras(deltaSeconds);
	}

	if (m_shouldRestart)
	{
		g_app->RestartGame();
		return;
	}
}

void Game::UpdatePlayers(float deltaSeconds)
{
	//Player Ships
	for (int playerNum = 0; playerNum < MAX_NUM_PLAYERS; ++playerNum)
	{
		if (m_playerShips[playerNum] != nullptr)
		{
			m_playerShips[playerNum]->Update(deltaSeconds);
		}
	}
}

void Game::UpdateNonPlayerEntities(float deltaSeconds)
{
	//Stars
	for (int starNum = 0; starNum < MAX_STARS; ++starNum)
	{
		if (m_stars[starNum] != nullptr)
		{
			m_stars[starNum]->Update(deltaSeconds);
		}
	}

	//PowerUps
	for (int powerUpNum = 0; powerUpNum < MAX_POWERUPS; ++powerUpNum)
	{
		if (m_powerUps[powerUpNum] == nullptr)
			continue;

		m_powerUps[powerUpNum]->Update(deltaSeconds);
	}

	//Bullets
	for (int bulletNum = 0; bulletNum < MAX_BULLETS; ++bulletNum)
	{
		if (m_bullets[bulletNum] != nullptr)
		{
			m_bullets[bulletNum]->Update(deltaSeconds);
		}
	}

	//Asteroids
	for (int asteroidNum = 0; asteroidNum < MAX_ASTEROIDS; ++asteroidNum)
	{
		if (m_asteroids[asteroidNum] != nullptr)
		{
			m_asteroids[asteroidNum]->Update(deltaSeconds);
		}
	}

	//Debris
	for (int debrisNum = 0; debrisNum < MAX_DEBRIS; ++debrisNum)
	{
		if (m_debris[debrisNum] != nullptr)
		{
			m_debris[debrisNum]->Update(deltaSeconds);
		}
	}

	int numActiveEnemies = 0;

	//Beatles
	for (int beatleNum = 0; beatleNum < MAX_BEETLES; ++beatleNum)
	{
		if (m_beetles[beatleNum] != nullptr)
		{
			m_beetles[beatleNum]->Update(deltaSeconds);
			numActiveEnemies++;
		}
	}
	//Wasps
	for (int waspNum = 0; waspNum < MAX_WASPS; ++waspNum)
	{
		if (m_wasps[waspNum] != nullptr)
		{
			m_wasps[waspNum]->Update(deltaSeconds);
			numActiveEnemies++;
		}
	}

	//Tells game to spawn new wave if no enemies are alive
	if (numActiveEnemies <= 0)
	{
		SpawnNextEnemyWave();
	}
}

//Render functions
//--------------------------------------------------------------------
void Game::ManageConditionalGameStateWorldRenders() const
{
	if (m_inGameplay)
	{
		RenderAllEntities();
	}

	else if (m_inPlayerConnectionLobby)
	{
		RenderPlayers();
	}
}

void Game::ManageConditionalGameStateScreenRenders() const
{
	if (m_inAttractMode)
	{
		RenderAttractScreen();
	}

	else if (m_inPlayerConnectionLobby)
	{
		RenderPlayerConnectionLobby();
		RenderPlayerLives();
	}

	else if (m_inInstructionsScreen)
	{
		RenderInstructionsScreen();
	}

	else if (m_inGameplay)
	{
		RenderPlayerLives();
		RenderPlayerHealth();
		RenderPlayerPowerUpTimer();
		RenderEnemyWaveData();
	}

	if (m_inGameOverSequence)
	{
		RenderGameOverScreen();
	}
}

void Game::RenderAttractScreen() const
{
	g_renderer->BindTexture(nullptr);
	//Left ship
	Vertex_PCU leftShipVerts[NUM_PLAYERSHIP_VERTS];
	PlayerShip::InitializeLocalVerts(&leftShipVerts[0], m_attractScreenInfo.leftShipColor);

	TransformVertexArrayXY3D(NUM_PLAYERSHIP_VERTS, leftShipVerts, 40.f, -90.f, m_attractScreenInfo.leftShipPos);
	g_renderer->DrawVertexArray(NUM_PLAYERSHIP_VERTS, &leftShipVerts[0]);

	//right ship
	Vertex_PCU rightShipVerts[NUM_PLAYERSHIP_VERTS];
	PlayerShip::InitializeLocalVerts(&rightShipVerts[0], m_attractScreenInfo.rightShipColor);

	TransformVertexArrayXY3D(NUM_PLAYERSHIP_VERTS, rightShipVerts, 40.f, -90.f, m_attractScreenInfo.rightShipPos);
	g_renderer->DrawVertexArray(NUM_PLAYERSHIP_VERTS, &rightShipVerts[0]);

	std::vector<Vertex_PCU> textVerts;
	float textOffset = GetSimpleTriangleStringWidth("Star Ship Gold", 100.f);
	textOffset *= 0.5f;
	AddVertsForTextTriangles2D(textVerts, "Star Ship Gold", Vec2(SCREEN_CENTER_X - textOffset, SCREEN_SIZE_Y - 200.f), 100.f, Rgba8(243, 246, 0, 255));


	//Render all Buttons
	Rgba8 color;
	AABB2 bounds;
	Vec2 startPos;
	Vec2 endPos;
	std::string text;
	for (int buttonNum = 0; buttonNum < NUM_ATTRACT_BUTTONS; ++buttonNum)
	{
		Button currentButton = m_attractScreenInfo.buttons[buttonNum];
		bounds = currentButton.buttonBounds;
		startPos = Vec2(bounds.m_mins.x, Lerp(bounds.m_mins.y, bounds.m_maxs.y, 0.5f));
		endPos = Vec2(bounds.m_maxs.x, startPos.y);

		color = m_attractScreenInfo.defaultButtonColor;
		if (currentButton.buttonType == m_selectedAttractScreenButton)
		{
			color = m_attractScreenInfo.selectedButtonColor;
		}
		DebugDrawLine2D(startPos, endPos, bounds.m_maxs.y - bounds.m_mins.y, color);
		text = currentButton.buttonText;

		AddVertsForTextTriangles2D(textVerts, text, currentButton.textPos, currentButton.textCellSize, currentButton.textColor);

	}
	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(textVerts);

}

void Game::RenderPlayerConnectionLobby() const
{
	//Vertical line
	DebugDrawLine2D(Vec2(SCREEN_CENTER_X, 0.f), Vec2(SCREEN_CENTER_X, SCREEN_SIZE_Y), 5.f, Rgba8(255, 255, 255, 50));
	//horizontal line
	DebugDrawLine2D(Vec2(0.f, SCREEN_CENTER_Y), Vec2(SCREEN_SIZE_X, SCREEN_CENTER_Y), 5.f, Rgba8(255, 255, 255, 50));

	std::vector<Vertex_PCU> textVerts;

	for (int playerNum = 0; playerNum < MAX_NUM_PLAYERS; ++playerNum)
	{
		Vec2 textPos = Vec2(400.f, 600.f);

		Rgba8 color = Rgba8(255, 0, 255, 255);
		switch (playerNum)
		{
		case 0:
			textPos = Vec2(400.f, 600.f);
			break;
		case 1:
			textPos = Vec2(1200.f, 200.f);
			break;
		case 2:
			textPos = Vec2(400.f, 200.f);
			break;
		case 3:
			textPos = Vec2(1200.f, 600.f);
		}

		std::string text = " ";
		if (m_playerShips[playerNum] == nullptr)
		{
			text = "Press Start or N to Connect";
			color = m_standardBodyTextColor;
		}

		else if (!m_readyPlayers[playerNum])
		{
			text = "Press A or SPACEBAR to Ready Up";
			color = m_standardBodyTextColor;
			textPos.y -= 200.f;
		}

		else
		{
			text = "Ready";
			color = Rgba8(0, 255, 29, 255);
			textPos.y -= 200.f;
		}

		float textOffset = GetSimpleTriangleStringWidth(text, 30.f) * 0.5f;
		textPos.x -= textOffset;
		AddVertsForTextTriangles2D(textVerts, text, textPos, 30.f, color);
	}

	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(textVerts);
}

void Game::RenderInstructionsScreen() const
{
	std::vector<Vertex_PCU> textVerts;
	Rgba8 color = m_standardBodyTextColor;
	float textOffset = GetSimpleTriangleStringWidth("How to Play", 50.f) * 0.5f;

	AddVertsForTextTriangles2D(textVerts, "How To Play", Vec2(SCREEN_CENTER_X - textOffset, SCREEN_SIZE_Y - 75.f), 50.f, Rgba8(243, 246, 0, 255));

	//Keyboard instructions
	textOffset = GetSimpleTriangleStringWidth("Keyboard Controls", 30.f) * 0.5f;
	AddVertsForTextTriangles2D(textVerts, "Keyboard Controls", Vec2(SCREEN_CENTER_X - textOffset, SCREEN_SIZE_Y - 150.f), 30.f, Rgba8(102, 153, 204, 255));
	textOffset = GetSimpleTriangleStringWidth("W to Move", 20.f) * 0.5f;
	AddVertsForTextTriangles2D(textVerts, "W to Move", Vec2(SCREEN_CENTER_X - textOffset, SCREEN_SIZE_Y - 190.f), 20.f, color);
	textOffset = GetSimpleTriangleStringWidth("A and S to Rotate", 20.f) * 0.5f;
	AddVertsForTextTriangles2D(textVerts, "A and S to Rotate", Vec2(SCREEN_CENTER_X - textOffset, SCREEN_SIZE_Y - 230.f), 20.f, color);
	textOffset = GetSimpleTriangleStringWidth("SPACEBAR to Shoot", 20.f) * 0.5f;
	AddVertsForTextTriangles2D(textVerts, "SPACEBAR to Shoot", Vec2(SCREEN_CENTER_X - textOffset, SCREEN_SIZE_Y - 270.f), 20.f, color);
	textOffset = GetSimpleTriangleStringWidth("N to Respawn", 20.f) * 0.5f;
	AddVertsForTextTriangles2D(textVerts, "N to Respawn", Vec2(SCREEN_CENTER_X - textOffset, SCREEN_SIZE_Y - 310.f), 20.f, color);
	textOffset = GetSimpleTriangleStringWidth("P To Pause", 20.f) * 0.5f;
	AddVertsForTextTriangles2D(textVerts, "P To Pause", Vec2(SCREEN_CENTER_X - textOffset, SCREEN_SIZE_Y - 350.f), 20.f, color);
	textOffset = GetSimpleTriangleStringWidth("ESC To Exit", 20.f) * 0.5f;
	AddVertsForTextTriangles2D(textVerts, "ESC To Exit", Vec2(SCREEN_CENTER_X - textOffset, SCREEN_SIZE_Y - 390.f), 20.f, color);

	//controller instructions;
	textOffset = GetSimpleTriangleStringWidth("Xbox Controls", 30.f) * 0.5f;
	AddVertsForTextTriangles2D(textVerts, "Xbox Controls", Vec2(SCREEN_CENTER_X - textOffset, SCREEN_SIZE_Y - 490.f), 30.f, Rgba8(255, 0, 0, 255));
	textOffset = GetSimpleTriangleStringWidth("Right Stick to Move", 20.f) * 0.5f;
	AddVertsForTextTriangles2D(textVerts, "Right Stick to Move", Vec2(SCREEN_CENTER_X - textOffset, SCREEN_SIZE_Y - 530.f), 20.f, color);
	textOffset = GetSimpleTriangleStringWidth("Left Stick to Rotate", 20.f) * 0.5f;
	AddVertsForTextTriangles2D(textVerts, "Left Stick to Rotate", Vec2(SCREEN_CENTER_X - textOffset, SCREEN_SIZE_Y - 570.f), 20.f, color);
	textOffset = GetSimpleTriangleStringWidth("A button or Right Trigger to Shoot", 20.f) * 0.5f;
	AddVertsForTextTriangles2D(textVerts, "A button or Right Trigger to Shoot", Vec2(SCREEN_CENTER_X - textOffset, SCREEN_SIZE_Y - 610.f), 20.f, color);
	textOffset = GetSimpleTriangleStringWidth("START to Respawn", 20.f) * 0.5f;
	AddVertsForTextTriangles2D(textVerts, "START to Respawn", Vec2(SCREEN_CENTER_X - textOffset, SCREEN_SIZE_Y - 650.f), 20.f, color);
	
	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(textVerts);


}

void Game::RenderPlayers() const
{
	//Player Ship
	for (int playerNum = 0; playerNum < MAX_NUM_PLAYERS; ++playerNum)
	{
		if (m_playerShips[playerNum] != nullptr)
		{
			m_playerShips[playerNum]->Render();
		}
	}
}

void Game::RenderAllEntities() const
{
	g_renderer->BindTexture(nullptr);
	//Stars
	for (int starNum = 0; starNum < MAX_STARS; ++starNum)
	{
		if (m_stars[starNum] != nullptr)
		{
			m_stars[starNum]->Render();
		}
	}

	//PowerUps
	for (int powerUpNum = 0; powerUpNum < MAX_POWERUPS; ++powerUpNum)
	{
		if (m_powerUps[powerUpNum] == nullptr)
			continue;

		m_powerUps[powerUpNum]->Render();
	}
	
	RenderPlayers();

	//Debris
	for (int debrisNum = 0; debrisNum < MAX_DEBRIS; ++debrisNum)
	{
		if (m_debris[debrisNum] != nullptr)
		{
			m_debris[debrisNum]->Render();
		}
	}

	//Bullets
	for (int bulletNum = 0; bulletNum < MAX_BULLETS; ++bulletNum)
	{
		if (m_bullets[bulletNum] != nullptr)
		{
			m_bullets[bulletNum]->Render();
		}
	}

	//Asteroids
	for (int asteroidNum = 0; asteroidNum < MAX_ASTEROIDS; ++asteroidNum)
	{
		if (m_asteroids[asteroidNum] != nullptr)
		{
			m_asteroids[asteroidNum]->Render();
		}
	}

	//Beatles
	for (int beatleNum = 0; beatleNum < MAX_BEETLES; ++beatleNum)
	{
		if (m_beetles[beatleNum] != nullptr)
		{
			m_beetles[beatleNum]->Render();
		}
	}

	//Wasps
	for (int waspNum = 0; waspNum < MAX_WASPS; ++waspNum)
	{
		if (m_wasps[waspNum] != nullptr)
		{
			m_wasps[waspNum]->Render();
		}
	}

}

void Game::RenderPlayerLives() const
{
	Vec2 fwrdVector = Vec2::MakeFromPolarDegrees(90.f, 1.f);
	fwrdVector *= 7.5f; //increases scale

	for (int playerNum = 0; playerNum < MAX_NUM_PLAYERS; ++playerNum)
	{
		if (m_playerShips[playerNum] == nullptr)
			continue;

		for (int lifeIndex = 0; lifeIndex < m_numExtraLives[playerNum]; ++lifeIndex)
		{
			Rgba8 playerLifeUIColor = m_playerColors[playerNum];
			playerLifeUIColor.a = 125; //drop opacity of player color for ui element

			//Create vertex array of ship with correct color
			Vertex_PCU playerLifeVerts[NUM_PLAYERSHIP_VERTS];
			PlayerShip::InitializeLocalVerts(&playerLifeVerts[0], playerLifeUIColor);
			
			Vec2 playerLifeUILocation = Vec2(m_playerLivesScreenLocation[playerNum].x + (35 * lifeIndex), m_playerLivesScreenLocation[playerNum].y);
			if (playerNum == 1 || playerNum == 3)
			{
				playerLifeUILocation = Vec2(m_playerLivesScreenLocation[playerNum].x - (35 * lifeIndex), m_playerLivesScreenLocation[playerNum].y);
			}
			
			TransformVertexArrayXY3D(NUM_PLAYERSHIP_VERTS, &playerLifeVerts[0], fwrdVector, fwrdVector.GetRotated90Degrees(), playerLifeUILocation);
			g_renderer->BindTexture(nullptr);
			g_renderer->DrawVertexArray(NUM_PLAYERSHIP_VERTS, &playerLifeVerts[0]);
		}
	}
}

void Game::RenderPlayerHealth() const
{
	for (int playerNum = 0; playerNum < MAX_NUM_PLAYERS; ++playerNum)
	{
		if (m_playerShips[playerNum] == nullptr)
			continue;

		//start pos
		Vec2 healthBarStartPos = Vec2(m_playerLivesScreenLocation[playerNum].x, m_playerLivesScreenLocation[playerNum].y - 50.f);
		if (playerNum == 1 || playerNum == 2)
		{
			healthBarStartPos = Vec2(m_playerLivesScreenLocation[playerNum].x, m_playerLivesScreenLocation[playerNum].y + 50.f);
		}

		//end pos
		Vec2 healthBarEndPos = Vec2(healthBarStartPos.x + (35 * (PLAYER_SHIP_NUM_STARTING_LIVES - 2)), healthBarStartPos.y);
		if (playerNum == 1 || playerNum == 3)
		{
			healthBarEndPos = Vec2(healthBarStartPos.x - (35 * (PLAYER_SHIP_NUM_STARTING_LIVES - 2)), healthBarStartPos.y);
		}
		
		DebugDrawLine2D(healthBarStartPos, healthBarEndPos, 15.f, Rgba8(133, 136, 134, 100)); //background bar

		float healthPosX = RangeMapClamped(static_cast<float>(m_playerShips[playerNum]->m_health), 0.f, static_cast<float>(PLAYER_SHIP_STARTING_HEALTH), healthBarStartPos.x, healthBarEndPos.x);
		DebugDrawLine2D(healthBarStartPos, Vec2(healthPosX, healthBarEndPos.y), 15.f, Rgba8(24, 223, 31, 255)); //health bar
	}
}

void Game::RenderPlayerPowerUpTimer() const
{
	for (int playerNum = 0; playerNum < MAX_NUM_PLAYERS; ++playerNum)
	{
		if (m_playerShips[playerNum] == nullptr || !m_playerShips[playerNum]->m_hasPowerUp)
			continue;

		//start pos
		Vec2 powerUpBarStartPos = Vec2(m_playerLivesScreenLocation[playerNum].x, m_playerLivesScreenLocation[playerNum].y - 75.f);
		if (playerNum == 1 || playerNum == 2)
		{
			powerUpBarStartPos = Vec2(m_playerLivesScreenLocation[playerNum].x, m_playerLivesScreenLocation[playerNum].y + 75.f);
		}

		//end pos
		Vec2 powerUpBarEndPos = Vec2(powerUpBarStartPos.x + (35 * (PLAYER_SHIP_NUM_STARTING_LIVES - 2)), powerUpBarStartPos.y);
		if (playerNum == 1 || playerNum == 3)
		{
			powerUpBarEndPos = Vec2(powerUpBarStartPos.x - (35 * (PLAYER_SHIP_NUM_STARTING_LIVES - 2)), powerUpBarStartPos.y);
		}

		DebugDrawLine2D(powerUpBarStartPos, powerUpBarEndPos, 15.f, Rgba8(133, 136, 134, 100)); //background bar

		float healthPosX = RangeMapClamped(static_cast<float>(m_playerShips[playerNum]->m_powerUpAge), 0.f, m_playerShips[playerNum]->m_powerUpMaxAge, powerUpBarEndPos.x, powerUpBarStartPos.x);
		DebugDrawLine2D(powerUpBarStartPos, Vec2(healthPosX, powerUpBarEndPos.y), 15.f, Rgba8(102, 153, 204, 255)); //power up bar
	}
}

void Game::RenderEnemyWaveData() const
{
	Vec2 enemyBarStartPos = Vec2(SCREEN_CENTER_X - 500.f, SCREEN_SIZE_Y - 50.f);
	Vec2 enemyBarEndPos = Vec2(SCREEN_CENTER_X + 500.f, SCREEN_SIZE_Y - 50.f);

	DebugDrawLine2D(enemyBarStartPos, enemyBarEndPos, 10.f, Rgba8(133, 136, 134, 100)); //background bar
	float completedPosX = RangeMapClamped(static_cast<float>(m_numEnemies), 0.f, static_cast<float>(m_numEnemiesInCurrentWave), enemyBarStartPos.x, enemyBarEndPos.x);
	DebugDrawLine2D(enemyBarStartPos,Vec2(completedPosX, enemyBarEndPos.y), 10.f, Rgba8(164, 16, 26, 100)); //wave bar

	std::vector<Vertex_PCU> textVerts;
	std::string waveTitle = "Wave: ";
	std::string waveNumber = std::to_string(m_currentWave);
	float textOffset = GetSimpleTriangleStringWidth(waveTitle + waveNumber, 30.f) * 0.5f;
	AddVertsForTextTriangles2D(textVerts, waveTitle + waveNumber, Vec2(SCREEN_CENTER_X - textOffset, SCREEN_SIZE_Y - 40.f), 30.f, Rgba8(164, 16, 26, 255));
	g_renderer->DrawVertexArray(textVerts);
}

void Game::RenderGameOverScreen() const
{
	std::vector<Vertex_PCU> textVerts;
	AddVertsForTextTriangles2D(textVerts, m_gameOverInfo.text, m_gameOverInfo.titlePos, m_gameOverInfo.textCellSize, m_gameOverInfo.titleColor);
	g_renderer->DrawVertexArray(textVerts);
}

//Spawn Functions
//--------------------------------------------------------------------
void Game::SpawnNewBullet(Vec2 const& position, float const& orientationDegrees, int const& playerID)
{
	for (int i = 0; i < MAX_BULLETS; ++i) // loop through m_bullets array, find first empty address, spawn bullet and assign it to that address
	{
		if (m_bullets[i] == nullptr)
		{
			m_bullets[i] = new Bullet(this, position, orientationDegrees, playerID, GetPlayerShipByID(playerID));
			PlayGameSFX(StarShipSFX::FIRE_BULLET);
			return;
		}
	}

	//ERROR_RECOVERABLE("Cannot spawn new bullet, all slots are full");
}

void Game::SpawnNewSpecialBullet(Vec2 const& position, float const& orientationDegrees, int const& playerID, PowerUpTypes const& bulletType)
{
	int numBulletsFired = 0;
	float bulletOrientation = orientationDegrees;
	int numBulletsToFire = 1;

	switch (bulletType)
	{
	case PowerUpTypes::TRI_BULLET:
		bulletOrientation -= 30.f;
		numBulletsToFire = 3;
		for (int bulletNum = 0; bulletNum < numBulletsToFire; ++bulletNum)
		{
			for (int i = 0; i < MAX_BULLETS; ++i) // loop through m_bullets array, find first empty address, spawn bullet and assign it to that address
			{
				if (m_bullets[i] == nullptr)
				{
					m_bullets[i] = new Bullet(this, position, bulletOrientation, playerID, GetPlayerShipByID(playerID));
					bulletOrientation += 30.f;
					numBulletsFired++;
					break;
				}
			}
		}
		break;

	case PowerUpTypes::FIVE_BULLET:
		bulletOrientation -= 30.f;
		numBulletsToFire = 5;
		for (int bulletNum = 0; bulletNum < numBulletsToFire; ++bulletNum)
		{
			for (int i = 0; i < MAX_BULLETS; ++i) // loop through m_bullets array, find first empty address, spawn bullet and assign it to that address
			{
				if (m_bullets[i] == nullptr)
				{
					m_bullets[i] = new Bullet(this, position, bulletOrientation, playerID, GetPlayerShipByID(playerID));
					bulletOrientation += 15.f;
					numBulletsFired++;
					break;
				}
			}
		}
		break;

	case PowerUpTypes::BURST_BULLET:
		numBulletsToFire = 29;
		for (int bulletNum = 0; bulletNum < numBulletsToFire; ++bulletNum)
		{
			for (int i = 0; i < MAX_BULLETS; ++i) // loop through m_bullets array, find first empty address, spawn bullet and assign it to that address
			{
				if (m_bullets[i] == nullptr)
				{
					m_bullets[i] = new Bullet(this, position, bulletOrientation, playerID, GetPlayerShipByID(playerID));
					bulletOrientation += 30.f;
					numBulletsFired++;
					break;
				}
			}
		}
		break;

	case PowerUpTypes::SNIPER_BULLET:
		for (int i = 0; i < MAX_BULLETS; ++i) // loop through m_bullets array, find first empty address, spawn bullet and assign it to that address
		{
			if (m_bullets[i] == nullptr)
			{
				m_bullets[i] = new Bullet(this, position, orientationDegrees, playerID, GetPlayerShipByID(playerID), bulletType);
				numBulletsFired++;
				break;
			}
		}
		break;

	case PowerUpTypes::SHIELD:
		SpawnNewBullet(position, orientationDegrees, playerID);
		break;

	default:
		break;
	}

	if (numBulletsFired > 0)
	{
		PlayGameSFX(StarShipSFX::FIRE_BULLET);
	}
}

void Game::SpawnAsteroid()
{
	for (int i = 0; i < MAX_ASTEROIDS; ++i) // loop through m_asteroids array, find first empty address, spawn asteroid and assign it to that address
	{
		if (m_asteroids[i] == nullptr)
		{ 
			Vec2 randomScreenPos = GetRandomPointOutsideScreen(ASTEROID_COSMETIC_RADIUS);
			float randomOrientation = g_rng->RollRandomFloatInRange(0.f, 360.f);

			m_asteroids[i] = new Asteroid(this, randomScreenPos, randomOrientation);
			return;
		}
	}

	//ERROR_RECOVERABLE("Cannot spawn new asteroid, all slots are full");
}

void Game::SpawnBeetle()
{
	for (int i = 0; i < MAX_BEETLES; ++i)
	{
		if (m_beetles[i] == nullptr)
		{
			Vec2 randomScreenPos = GetRandomPointOutsideScreen(BEETLE_COSMETIC_RADIUS);
			m_beetles[i] = new Beetle(this, randomScreenPos, 0.f);
			return;
		}
	}
}

void Game::SpawnWasp()
{
	for (int i = 0; i < MAX_WASPS; ++i)
	{
		if (m_wasps[i] == nullptr)
		{
			Vec2 randomScreenPos = GetRandomPointOutsideScreen(WASP_COSMETIC_RADIUS);
			m_wasps[i] = new Wasp(this, randomScreenPos, 0.f);
			return;
		}
	}
}

void Game::SpawnNewDebris(Vec2 const& position, Vec2 const& velocity, float const& averageRadius, Rgba8 const& color)
{
	for (int i = 0; i < MAX_DEBRIS; ++i) // loop through m_asteroids array, find first empty address, spawn asteroid and assign it to that address
	{
		if (m_debris[i] == nullptr)
		{
			float orientationDeg = g_rng->RollRandomFloatInRange(0.f, 360.f);
			float radiusMax = averageRadius * 1.5f;
			float radiusMin = averageRadius * .05f;
			m_debris[i] = new Debris(this, position, orientationDeg, velocity, color, radiusMax, radiusMin);
			return;
		}
	}
}

void Game::SpawnNewDebrisCluster(Vec2 const& position, int numDebris, Vec2 const& averageVelocity, float maxScatterSpeed, float averageRadius, Rgba8 const& color)
{
	for (int i = 0; i < numDebris; ++i)
	{
		float thetaDegrees = g_rng->RollRandomFloatInRange(0.f, 360.f);
		float speed = g_rng->RollRandomFloatZeroToOne() * maxScatterSpeed;
		Vec2 scatterVelocity = Vec2::MakeFromPolarDegrees(thetaDegrees, speed);
		Vec2 velocity = averageVelocity + scatterVelocity;
		SpawnNewDebris(position, velocity, averageRadius, color);
	}
}

void Game::SpawnNewPowerUp(Vec2 const& position)
{
	//PowerUps
	for (int powerUpNum = 0; powerUpNum < MAX_POWERUPS; ++powerUpNum)
	{
		if (m_powerUps[powerUpNum] == nullptr)
		{
			m_powerUps[powerUpNum] = new PowerUp(this, position, g_rng->RollRandomFloatInRange(0.f, 360.f));
			return;
		}
			
	}
}

void Game::SpawnNextEnemyWave()
{
	m_numEnemiesInCurrentWave = 0;
	if (m_currentWave >= NUM_PLANNED_ENEMY_WAVES || m_currentWave < 0)
	{
		if (!m_inGameOverSequence && !m_inMultiplayerMode)
		{
			GameOver(m_playerShips[0]->m_playerNum, true);
		}

		else if( m_inMultiplayerMode)
		{
			SpawnRandomizedEnemyWave();
		}

		return;
	}

	EnemyWaveInfo currentWaveInfo = m_enemyWavesInfo[m_currentWave];

	for (int i = 0; i < currentWaveInfo.numBeetles; ++i)
	{
		SpawnBeetle();
		m_numEnemiesInCurrentWave++;
	}

	for (int i = 0; i < currentWaveInfo.numWasps; ++i)
	{
		SpawnWasp();
		m_numEnemiesInCurrentWave++;
	}

	int numActiveAsteroids = 0;
	for (int i = 0; i < MAX_ASTEROIDS; ++i)
	{
		if (m_asteroids[i] != nullptr)
		{
			numActiveAsteroids++;
		}
	}

	int numAsteroidsToSpawn = GetClampedInt(currentWaveInfo.numAsteroids, 0, MAX_ASTEROIDS - numActiveAsteroids);

	for (int i = 0; i < numAsteroidsToSpawn; ++i)
	{
		SpawnAsteroid();
	}

	if (m_currentWave > 0)
	{
		PlayGameSFX(StarShipSFX::NEW_ENEMY_WAVE);
	}

	m_currentWave++; //happens at end to account for array index starting at 0. So wave 0 = first wave, wave 1 = second wave etc
	m_numEnemies = m_numEnemiesInCurrentWave;
}

void Game::SpawnRandomizedEnemyWave()
{
	m_currentWave++;
	PlayGameSFX(StarShipSFX::NEW_ENEMY_WAVE);

	for (int numEnemies = 0; numEnemies < m_currentWave; ++numEnemies)
	{
		int randRoll = g_rng->RollRandomIntInRange(0, WASP_BIAS);
		if (randRoll == 0)
		{
			SpawnBeetle();
			m_numEnemiesInCurrentWave++;
		}
		else
		{
			SpawnWasp();
			m_numEnemiesInCurrentWave++;
		}
		SpawnAsteroid();
	}

	m_numEnemies = m_numEnemiesInCurrentWave;
}

//Deletion Functions
//--------------------------------------------------------------------
void Game::DeleteGarbageEntities()
{
	//Bullets
	for (int bulletNum = 0; bulletNum < MAX_BULLETS; ++bulletNum)
	{
		if (m_bullets[bulletNum] == nullptr) //skip index if element is a nullptr
			continue;

		else if (m_bullets[bulletNum]->IsGarbage())
		{
			delete(m_bullets[bulletNum]);
			m_bullets[bulletNum] = nullptr;
		}
	}

	//Asteroids
	for (int asteroidNum = 0; asteroidNum < MAX_ASTEROIDS; ++asteroidNum)
	{
		if (m_asteroids[asteroidNum] == nullptr)
			continue;

		else if (m_asteroids[asteroidNum]->IsGarbage())
		{
			delete(m_asteroids[asteroidNum]);
			m_asteroids[asteroidNum] = nullptr;
		}
	}

	//Debris
	for (int debrisNum = 0; debrisNum < MAX_DEBRIS; ++debrisNum)
	{
		if (m_debris[debrisNum] == nullptr)
			continue;

		else if (m_debris[debrisNum]->IsGarbage())
		{
			delete(m_debris[debrisNum]);
			m_debris[debrisNum] = nullptr;
		}
	}

	//Beetles
	for (int beetleNum = 0; beetleNum < MAX_BEETLES; ++beetleNum)
	{
		if (m_beetles[beetleNum] == nullptr)
			continue;

		else if (m_beetles[beetleNum]->IsGarbage())
		{
			delete(m_beetles[beetleNum]);
			m_beetles[beetleNum] = nullptr;
		}
	}

	//Wasps
	for (int waspNum = 0; waspNum < MAX_WASPS; ++waspNum)
	{
		if (m_wasps[waspNum] == nullptr)
			continue;

		else if (m_wasps[waspNum]->IsGarbage())
		{
			delete(m_wasps[waspNum]);
			m_wasps[waspNum] = nullptr;
		}
	}

	for (int powerUpNum = 0; powerUpNum < MAX_POWERUPS; ++powerUpNum)
	{
		if (m_powerUps[powerUpNum] == nullptr)
			continue;

		else if (m_powerUps[powerUpNum]->IsGarbage())
		{
			delete(m_powerUps[powerUpNum]);
			m_powerUps[powerUpNum] = nullptr;
		}
	}
}

void Game::ClearEnemyWave()
{
	//Asteroids
	for (int asteroidNum = 0; asteroidNum < MAX_ASTEROIDS; ++asteroidNum)
	{
		if (m_asteroids[asteroidNum] == nullptr)
			continue;

		m_asteroids[asteroidNum]->Die();
	}

	//Debris
	for (int debrisNum = 0; debrisNum < MAX_DEBRIS; ++debrisNum)
	{
		if (m_debris[debrisNum] == nullptr)
			continue;

		m_debris[debrisNum]->Die();
	}

	//Beetles
	for (int beetleNum = 0; beetleNum < MAX_BEETLES; ++beetleNum)
	{
		if (m_beetles[beetleNum] == nullptr)
			continue;

		m_beetles[beetleNum]->Die();
	}

	//Wasps
	for (int waspNum = 0; waspNum < MAX_WASPS; ++waspNum)
	{
		if (m_wasps[waspNum] == nullptr)
			continue;

		m_wasps[waspNum]->Die();
	}
}

//Collision Functions
//--------------------------------------------------------------------
void Game::CheckAllEntityCollisions()
{
	CheckBulletCollisions();
	CheckPlayerCollisions();
	CheckEnemyCollisions();
}

void Game::CheckBulletCollisions()
{
	for (int bulletNum = 0; bulletNum < MAX_BULLETS; ++bulletNum)
	{
		Bullet*& currentBullet = m_bullets[bulletNum];
		if (currentBullet == nullptr) //skip index if element is a nullptr
			continue;

		if (!currentBullet->IsAlive()) //skip index if bullet is already dead
			continue;

		Vec2 bulletPos = currentBullet->m_position;

		//Bullet vs Asteroids
		for (int asteroidNum = 0; asteroidNum < MAX_ASTEROIDS; ++asteroidNum)
		{
			Asteroid*& currentAsteroid = m_asteroids[asteroidNum];
			if (currentAsteroid == nullptr) //skips index if element is a nullptr
				continue;

			if (!currentAsteroid->IsAlive())//skips index if asteroid is already dead
				continue;

			else if (DoDiscsOverlap(currentBullet->m_position, BULLET_PHYSICS_RADIUS, currentAsteroid->m_position, ASTEROID_PHYSICS_RADIUS))
			{
				currentAsteroid->LoseHealth();
				currentBullet->Die();

				//spawn small debris
				int debrisAmount = g_rng->RollRandomIntInRange(m_smallDebrisAmountRange.x, m_smallDebrisAmountRange.y);
				Vec2 velocity = currentBullet->m_velocity * m_smallDebrisVelocityScale;
				SpawnNewDebrisCluster(bulletPos, debrisAmount, velocity, DEBRIS_MAX_SCATTER_SPEED, .25f, currentAsteroid->m_color);

			}
		}

		//Bullet vs Beetles
		for (int beetleNum = 0; beetleNum < MAX_BEETLES; ++beetleNum)
		{
			Beetle*& currentBeetle = m_beetles[beetleNum];
			if (currentBeetle == nullptr)
				continue;

			if (!currentBeetle->IsAlive())
				continue;

			else if (DoDiscsOverlap(bulletPos, BULLET_PHYSICS_RADIUS, currentBeetle->m_position, BEETLE_PHYSICS_RADIUS))
			{
				currentBullet->Die();
				currentBeetle->LoseHealth();

				//spawn small debris
				int debrisAmount = g_rng->RollRandomIntInRange(m_smallDebrisAmountRange.x, m_smallDebrisAmountRange.y);
				Vec2 velocity = currentBullet->m_velocity * m_smallDebrisVelocityScale;
				SpawnNewDebrisCluster(bulletPos, debrisAmount, velocity, DEBRIS_MAX_SCATTER_SPEED, .25f, currentBeetle->m_color);
			}
		}

		//Bullet vs Wasps
		for (int waspNum = 0; waspNum < MAX_WASPS; ++waspNum)
		{
			Wasp*& currentWasp = m_wasps[waspNum];
			if (currentWasp == nullptr)
				continue;

			if (!currentWasp->IsAlive())
				continue;
			
			else if (DoDiscsOverlap(bulletPos, BULLET_PHYSICS_RADIUS, currentWasp->m_position, WASP_PHYSICS_RADIUS))
			{
				currentBullet->Die();
				currentWasp->LoseHealth();

				//spawn small debris
				int debrisAmount = g_rng->RollRandomIntInRange(m_smallDebrisAmountRange.x, m_smallDebrisAmountRange.y);
				Vec2 velocity = currentBullet->m_velocity * m_smallDebrisVelocityScale;
				SpawnNewDebrisCluster(bulletPos, debrisAmount, velocity, DEBRIS_MAX_SCATTER_SPEED, .25f, currentWasp->m_color);
			}
		}
	}
}

void Game::CheckPlayerCollisions()
{
	for (int playerNum = 0; playerNum < MAX_NUM_PLAYERS; ++playerNum)
	{
		PlayerShip*& currentPlayerShip = m_playerShips[playerNum];

		if (currentPlayerShip == nullptr)
			continue;

		if (!currentPlayerShip->IsAlive()) //skips tracking player collision if m_playerShip is dead
			continue;

		Vec2 playerShipPos = currentPlayerShip->m_position;

		//Player vs Asteroids
		for (int asteroidNum = 0; asteroidNum < MAX_ASTEROIDS; ++asteroidNum)
		{
			Asteroid*& currentAsteroid = m_asteroids[asteroidNum];
			if (currentAsteroid == nullptr) //skips index if element is a nullptr
				continue;

			if (!currentAsteroid->IsAlive()) //skip index if asteroid is already dead
				continue;

			if (currentPlayerShip->HasShield())
			{
				if (DoDiscsOverlap(playerShipPos, PLAYER_SHIP_SHIELD_RADIUS, currentAsteroid->m_position, ASTEROID_PHYSICS_RADIUS))
				{
					currentAsteroid->Die();
				}
			}

			else if (DoDiscsOverlap(currentAsteroid->m_position, ASTEROID_PHYSICS_RADIUS, playerShipPos, PLAYER_SHIP_PHYSICS_RADIUS))
			{
				currentPlayerShip->LoseHealth();
				currentAsteroid->Die();
				SpawnAsteroid();
			}
		}

		//Player vs Beetles
		for (int beetleNum = 0; beetleNum < MAX_BEETLES; ++beetleNum)
		{
			Beetle*& currentBeetle = m_beetles[beetleNum];
			if (currentBeetle == nullptr)
				continue;

			if (!currentBeetle->IsAlive())
				continue;

			if (currentPlayerShip->HasShield())
			{
				if (DoDiscsOverlap(playerShipPos, PLAYER_SHIP_SHIELD_RADIUS, currentBeetle->m_position, BEETLE_PHYSICS_RADIUS))
				{
					PushDiscOutOfFixedDisc2D(currentBeetle->m_position, BEETLE_PHYSICS_RADIUS, playerShipPos, PLAYER_SHIP_SHIELD_RADIUS);
				}
			}

			else if (DoDiscsOverlap(playerShipPos, PLAYER_SHIP_PHYSICS_RADIUS, currentBeetle->m_position, BEETLE_PHYSICS_RADIUS))
			{
				currentPlayerShip->LoseHealth();
				PushDiscOutOfFixedDisc2D(currentBeetle->m_position, BEETLE_PHYSICS_RADIUS, playerShipPos, PLAYER_SHIP_PHYSICS_RADIUS);
			}
		}

		//Player vs Wasps
		for (int waspNum = 0; waspNum < MAX_WASPS; ++waspNum)
		{
			Wasp*& currentWasp = m_wasps[waspNum];
			if (currentWasp == nullptr)
				continue;

			if (!currentWasp->IsAlive())
				continue;

			if (currentPlayerShip->HasShield())
			{
				if (DoDiscsOverlap(playerShipPos, PLAYER_SHIP_SHIELD_RADIUS, currentWasp->m_position, WASP_PHYSICS_RADIUS))
				{
					PushDiscOutOfFixedDisc2D(currentWasp->m_position, BEETLE_PHYSICS_RADIUS, playerShipPos, PLAYER_SHIP_SHIELD_RADIUS);
				}
			}

			else if (DoDiscsOverlap(playerShipPos, PLAYER_SHIP_PHYSICS_RADIUS,currentWasp->m_position, WASP_PHYSICS_RADIUS))
			{
				currentPlayerShip->LoseHealth();
				PushDiscOutOfFixedDisc2D(currentWasp->m_position, BEETLE_PHYSICS_RADIUS, playerShipPos, PLAYER_SHIP_PHYSICS_RADIUS);
			}
		}

		//Player vs other players
		for (int otherPlayerNum = 0; otherPlayerNum < MAX_NUM_PLAYERS; ++otherPlayerNum)
		{
			if (otherPlayerNum == playerNum)
				continue;
			PlayerShip*& otherPlayer = m_playerShips[otherPlayerNum];

			if (otherPlayer == nullptr)
				continue;

			if (!otherPlayer->IsAlive())
				continue;

			if (currentPlayerShip->HasShield())
			{
				//Shield vs shield
				if (otherPlayer->HasShield())
				{
					if (DoDiscsOverlap(playerShipPos, PLAYER_SHIP_SHIELD_RADIUS, otherPlayer->m_position, PLAYER_SHIP_SHIELD_RADIUS))
					{
						PushDiscsOutOfEachOther2D(playerShipPos, PLAYER_SHIP_SHIELD_RADIUS, otherPlayer->m_position, PLAYER_SHIP_SHIELD_RADIUS);
					}
				}

				//Shield vs no shield
				else
				{
					if (DoDiscsOverlap(playerShipPos, PLAYER_SHIP_SHIELD_RADIUS, otherPlayer->m_position, PLAYER_SHIP_PHYSICS_RADIUS))
					{
						PushDiscOutOfFixedDisc2D(otherPlayer->m_position, PLAYER_SHIP_PHYSICS_RADIUS, playerShipPos, PLAYER_SHIP_SHIELD_RADIUS);
					}
				}

			}

			//no shield vs no shield
			else if (DoDiscsOverlap(playerShipPos, PLAYER_SHIP_PHYSICS_RADIUS, otherPlayer->m_position, PLAYER_SHIP_PHYSICS_RADIUS))
			{
				PushDiscsOutOfEachOther2D(playerShipPos, PLAYER_SHIP_PHYSICS_RADIUS, otherPlayer->m_position, PLAYER_SHIP_PHYSICS_RADIUS);
			}
		}
		//Player vs PowerUps
		for (int powerUpNum = 0; powerUpNum < MAX_POWERUPS; ++powerUpNum)
		{
			PowerUp*& currentPowerUp = m_powerUps[powerUpNum];
			if (currentPowerUp == nullptr)
				continue;

			if (!currentPowerUp->IsAlive())
				continue;

			if (DoDiscsOverlap(playerShipPos, PLAYER_SHIP_PHYSICS_RADIUS, currentPowerUp->m_position, POWERUP_PHYSICS_RADIUS))
			{
				currentPlayerShip->PickUpPowerUp(currentPowerUp->m_powerupType);
				currentPowerUp->Die();
			}
		}

		//Player vs Bullets of other players
		if (m_inCoOpMode)
			continue;

		for (int bulletNum = 0; bulletNum < MAX_BULLETS; ++bulletNum)
		{
			Bullet*& currentBullet = m_bullets[bulletNum];
			if (currentBullet == nullptr)
				continue;

			if (!currentBullet->IsAlive())
				continue;

			if (currentBullet->GetOwningPlayerID() == currentPlayerShip->m_playerID)
				continue;

			else if (DoDiscsOverlap(playerShipPos, PLAYER_SHIP_PHYSICS_RADIUS, currentBullet->m_position, BULLET_PHYSICS_RADIUS))
			{
				currentPlayerShip->LoseHealth();
				currentBullet->Die();
			}
		}

		
	}
}

void Game::CheckEnemyCollisions()
{
	//Beetles
	for (int beetleNum = 0; beetleNum < MAX_BEETLES; ++beetleNum)
	{
		Beetle*& currentBeetle = m_beetles[beetleNum];
		if (currentBeetle == nullptr)
			continue;

		if (!currentBeetle->IsAlive())
			continue;

		//Asteroids
		for (int asteroidNum = 0; asteroidNum < MAX_ASTEROIDS; ++asteroidNum)
		{
			Asteroid*& currentAsteroid = m_asteroids[asteroidNum];
			if (currentAsteroid == nullptr) //skips index if element is a nullptr
				continue;

			if (!currentAsteroid->IsAlive()) //skip index if asteroid is already dead
				continue;

			else if (DoDiscsOverlap(currentAsteroid->m_position, ASTEROID_PHYSICS_RADIUS, currentBeetle->m_position, BEETLE_PHYSICS_RADIUS))
			{
				currentAsteroid->Die();
				currentBeetle->LoseHealth();
			}
		}

		//Checking against other enemies to push away from each other
		for (int otherBeetleNum = 0; otherBeetleNum < MAX_BEETLES; ++otherBeetleNum)
		{
			if (otherBeetleNum == beetleNum) // skip if same beetle
				continue;

			Beetle*& otherBeetle = m_beetles[otherBeetleNum];
			if (otherBeetle == nullptr)
				continue;

			if (!otherBeetle->IsAlive())
				continue;

			if (DoDiscsOverlap(currentBeetle->m_position, BEETLE_PHYSICS_RADIUS, otherBeetle->m_position, BEETLE_PHYSICS_RADIUS))
			{
				PushDiscsOutOfEachOther2D(currentBeetle->m_position, BEETLE_PHYSICS_RADIUS, otherBeetle->m_position, BEETLE_PHYSICS_RADIUS);
			}
		}

		for (int waspNum = 0; waspNum < MAX_WASPS; ++waspNum)
		{
			Wasp*& currentWasp = m_wasps[waspNum];
			if (currentWasp == nullptr)
				continue;

			if (!currentWasp->IsAlive())
				continue;

			if (DoDiscsOverlap(currentBeetle->m_position, BEETLE_PHYSICS_RADIUS, currentWasp->m_position, WASP_PHYSICS_RADIUS))
			{
				PushDiscsOutOfEachOther2D(currentBeetle->m_position, BEETLE_PHYSICS_RADIUS, currentWasp->m_position, WASP_PHYSICS_RADIUS);
			}
		}
	}

	//Wasps
	for (int waspNum = 0; waspNum < MAX_WASPS; ++waspNum)
	{
		Wasp*& currentWasp = m_wasps[waspNum];
		if (currentWasp == nullptr)
			continue;

		if (!currentWasp->IsAlive())
			continue;

		//Asteroids
		for (int asteroidNum = 0; asteroidNum < MAX_ASTEROIDS; ++asteroidNum)
		{
			Asteroid*& currentAsteroid = m_asteroids[asteroidNum];
			if (currentAsteroid == nullptr) //skips index if element is a nullptr
				continue;

			if (!currentAsteroid->IsAlive()) //skip index if asteroid is already dead
				continue;

			else if (DoDiscsOverlap(currentAsteroid->m_position, ASTEROID_PHYSICS_RADIUS, currentWasp->m_position, WASP_PHYSICS_RADIUS))
			{
				currentAsteroid->Die();
				currentWasp->LoseHealth();
			}
		}

		//Other Wasps
		for (int otherWaspNum = 0; otherWaspNum < MAX_WASPS; ++otherWaspNum)
		{
			if (otherWaspNum == waspNum) //Skip if same wasp
				continue;

			Wasp*& otherWasp = m_wasps[otherWaspNum];
			if (otherWasp == nullptr)
				continue;

			if (!otherWasp->IsAlive())
				continue;

			if (DoDiscsOverlap(otherWasp->m_position, WASP_PHYSICS_RADIUS, currentWasp->m_position, WASP_PHYSICS_RADIUS))
			{
				PushDiscsOutOfEachOther2D(otherWasp->m_position, WASP_PHYSICS_RADIUS, currentWasp->m_position, WASP_PHYSICS_RADIUS);
			}
		}
	}

	//Asteroids
	for (int asteroidNum = 0; asteroidNum < MAX_ASTEROIDS; ++asteroidNum)
	{
		Asteroid*& currentAsteroid = m_asteroids[asteroidNum];
		if (currentAsteroid == nullptr) //skips index if element is a nullptr
			continue;

		if (!currentAsteroid->IsAlive()) //skip index if asteroid is already dead
			continue;

		for (int otherAsteroidNum = 0; otherAsteroidNum < MAX_ASTEROIDS; ++otherAsteroidNum)
		{
			if (otherAsteroidNum == asteroidNum)
				continue;

			Asteroid*& otherAsteroid = m_asteroids[otherAsteroidNum];
			if (otherAsteroid == nullptr) //skips index if element is a nullptr
				continue;

			if (!otherAsteroid->IsAlive()) //skip index if asteroid is already dead
				continue;

			else if (DoDiscsOverlap(currentAsteroid->m_position, ASTEROID_PHYSICS_RADIUS, otherAsteroid->m_position, ASTEROID_PHYSICS_RADIUS))
			{
				currentAsteroid->Die();
				otherAsteroid->Die();
			}
		}
	}
}

//Game Audio
//-----------------------------------------------------------------------------------------------
void const Game::PlayGameSFX(StarShipSFX soundEffect) const
{
	//default sound initialized with non-existant sound
	SoundID newSound = g_audioSystem->CreateOrGetSound("NonExistantSound");
	bool isLooped = false;
	float volume = 1.f;
	float balance = 0.f;
	float speed = 1.f;
	bool isPaused = false;

	switch (soundEffect)
	{
	case StarShipSFX::ENTER_GAME:
		newSound = g_audioSystem->CreateOrGetSound("Data/Audio/SFX/EnterGame.mp3");
		speed = 1.5f;
		break;

	case StarShipSFX::FIRE_BULLET:
		newSound = g_audioSystem->CreateOrGetSound("Data/Audio/SFX/FireBullet.wav");
		volume = 0.5f;
		break;

	case StarShipSFX::PLAYER_DAMAGED:
		newSound = g_audioSystem->CreateOrGetSound("Data/Audio/SFX/PlayerDamaged.wav");
		break;

	case StarShipSFX::PLAYER_DEATH:
		newSound = g_audioSystem->CreateOrGetSound("Data/Audio/SFX/PlayerDeath.wav");
		break;

	case StarShipSFX::PLAYER_RESPAWN:
		newSound = g_audioSystem->CreateOrGetSound("Data/Audio/SFX/PlayerRespawn.mp3");
		break;

	case StarShipSFX::ENEMY_DEATH:
		newSound = g_audioSystem->CreateOrGetSound("Data/Audio/SFX/EnemyDeath.wav");
		volume = .75f;
		speed = 1.5f;
		break;

	case StarShipSFX::NEW_ENEMY_WAVE:
		newSound = g_audioSystem->CreateOrGetSound("Data/Audio/SFX/NewEnemyWave.mp3");
		break;

	case StarShipSFX::POWERUP:
		newSound = g_audioSystem->CreateOrGetSound("Data/Audio/SFX/PowerUp.wav");
		break;

	case StarShipSFX::ASTEROID_HIT:
		newSound = g_audioSystem->CreateOrGetSound("Data/Audio/SFX/AsteroidHit.mp3");
		break;

	case StarShipSFX::ASTEROID_BROKEN:
		newSound = g_audioSystem->CreateOrGetSound("Data/Audio/SFX/AsteroidBroken.wav");
		break;

	default:
		break;
	}

	g_audioSystem->StartSound(newSound, isLooped, volume, balance, speed, isPaused);
}

void const Game::PlayGameSFX(StarShipSFX soundEffect, Vec2 const& worldPosition)
{
	//default sound initialized with non-existant sound
	SoundID newSound = g_audioSystem->CreateOrGetSound("NonExistantSound");
	bool isLooped = false;
	float volume = 1.f;
	float balance = 0.f;
	float speed = 1.f;
	bool isPaused = false;

	switch (soundEffect)
	{
	case StarShipSFX::ENTER_GAME:
		newSound = g_audioSystem->CreateOrGetSound("Data/Audio/SFX/EnterGame.mp3");
		speed = 1.5f;
		break;

	case StarShipSFX::FIRE_BULLET:
		newSound = g_audioSystem->CreateOrGetSound("Data/Audio/SFX/FireBullet.wav");
		volume = 0.5f;
		break;

	case StarShipSFX::PLAYER_DAMAGED:
		newSound = g_audioSystem->CreateOrGetSound("Data/Audio/SFX/PlayerDamaged.wav");
		break;

	case StarShipSFX::PLAYER_DEATH:
		newSound = g_audioSystem->CreateOrGetSound("Data/Audio/SFX/PlayerDeath.wav");
		break;

	case StarShipSFX::PLAYER_RESPAWN:
		newSound = g_audioSystem->CreateOrGetSound("Data/Audio/SFX/PlayerRespawn.mp3");
		break;

	case StarShipSFX::ENEMY_DEATH:
		newSound = g_audioSystem->CreateOrGetSound("Data/Audio/SFX/EnemyDeath.wav");
		volume = .75f;
		speed = 1.5f;
		break;

	case StarShipSFX::NEW_ENEMY_WAVE:
		newSound = g_audioSystem->CreateOrGetSound("Data/Audio/SFX/NewEnemyWave.mp3");
		break;

	case StarShipSFX::POWERUP:
		newSound = g_audioSystem->CreateOrGetSound("Data/Audio/SFX/PowerUp.wav");
		break;

	case StarShipSFX::ASTEROID_HIT:
		newSound = g_audioSystem->CreateOrGetSound("Data/Audio/SFX/AsteroidHit.mp3");
		break;

	case StarShipSFX::ASTEROID_BROKEN:
		newSound = g_audioSystem->CreateOrGetSound("Data/Audio/SFX/AsteroidBroken.wav");
		break;

	default:
		break;
	}

	SoundPlaybackID newPlayback = g_audioSystem->StartSound(newSound, isLooped, volume, balance, speed, isPaused);
	SetAudioBalanceAndVolumeFromWorldPosition(newPlayback, worldPosition);
}

void const Game::PlayGameMusic(StarShipMusic musicTrack, bool loop)
{
	SoundID newMusic = g_audioSystem->CreateOrGetSound("NonExistantSound");

	switch (musicTrack)
	{
	case StarShipMusic::ATTRACT_SCREEN_MUSIC:
		StopGameMusic(m_attractScreenMusic);
		newMusic = g_audioSystem->CreateOrGetSound("Data/Audio/Music/AttractScreenMusic.mp3");
		m_attractScreenMusic = g_audioSystem->StartSound(newMusic, loop, 1.f, 0.f, 1.f, false);
		break;

	case StarShipMusic::GAME_MUSIC:
		StopGameMusic(m_gameMusic);
		newMusic = g_audioSystem->CreateOrGetSound("Data/Audio/Music/GameMusic.mp3");
		m_gameMusic = g_audioSystem->StartSound(newMusic, loop, .5f, 0.f, 1.f, false);
		break;

	case StarShipMusic::PLAYER_SHIP_ENGINE_THRUST:
		newMusic = g_audioSystem->CreateOrGetSound("Data/Audio/Music/PlayerShipEngineThrust.wav");
		g_audioSystem->StartSound(newMusic, false, .5f, 0.f, 1.f, false);

	}

}

void const Game::PlayGameMusic(SoundPlaybackID& soundPlayBackID, StarShipMusic musicTrack, bool loop)
{
	SoundID newMusic = g_audioSystem->CreateOrGetSound("NonExistantSound");

	switch (musicTrack)
	{
	case StarShipMusic::ATTRACT_SCREEN_MUSIC:
		StopGameMusic(m_attractScreenMusic);
		newMusic = g_audioSystem->CreateOrGetSound("Data/Audio/Music/AttractScreenMusic.mp3");
		soundPlayBackID = g_audioSystem->StartSound(newMusic, loop, 1.f, 0.f, 1.f, false);
		break;

	case StarShipMusic::GAME_MUSIC:
		StopGameMusic(m_gameMusic);
		newMusic = g_audioSystem->CreateOrGetSound("Data/Audio/Music/GameMusic.mp3");
		soundPlayBackID = g_audioSystem->StartSound(newMusic, loop, .5f, 0.f, 1.f, false);
		break;

	case StarShipMusic::PLAYER_SHIP_ENGINE_THRUST:
		newMusic = g_audioSystem->CreateOrGetSound("Data/Audio/Music/PlayerShipEngineThrust.wav");
		soundPlayBackID = g_audioSystem->StartSound(newMusic, loop, .5f, 0.f, 1.f, false);

	default:
		break;
	}
}

void const Game::StopGameMusic(StarShipMusic musicTrack)
{
	switch (musicTrack)
	{
	case StarShipMusic::ATTRACT_SCREEN_MUSIC:
		g_audioSystem->StopSound(m_attractScreenMusic);
		break;

	case StarShipMusic::GAME_MUSIC:
		g_audioSystem->StopSound(m_gameMusic);
		break;

	default:
		break;
	}
}

void const Game::StopGameMusic(SoundPlaybackID soundPlaybackID)
{
	g_audioSystem->StopSound(soundPlaybackID);
}

float Game::GetAudioBalanceFromWorldPosition(Vec2 const& inPosition) const
{
	return RangeMapClamped(inPosition.x, m_worldCamBottomLeft.x, m_worldCamTopRight.x, -1.f, 1.f);;
}

void const Game::SetAudioBalanceAndVolumeFromWorldPosition(SoundPlaybackID& sound, Vec2 const& worldPos)
{
	AABB2 cameraView = AABB2(m_worldCamBottomLeft, m_worldCamTopRight);
	if (!cameraView.IsPointInside(worldPos)) // volume = 0 if position is not in camera view
	{
		g_audioSystem->SetSoundPlaybackVolume(sound, 0.f);
	}
	g_audioSystem->SetSoundPlaybackBalance(sound, RangeMapClamped(worldPos.x, m_worldCamBottomLeft.x, m_worldCamTopRight.x, -1.f, 1.f));
}

//Helper Functions
//-----------------------------------------------------------------------------------------------
Vec2 const Game::GetRandomPointOutsideScreen(float const& offset) const
{
	Vec2 randomScreenPos;

	int side = g_rng->RollRandomIntInRange(0, 3);
	switch (side)
	{
	case 0: //bottom
		randomScreenPos.x = g_rng->RollRandomFloatInRange(0.f, WORLD_SIZE_X);
		randomScreenPos.y = -offset;
		break;

	case 1: //right
		randomScreenPos.x = WORLD_SIZE_X + offset;
		randomScreenPos.y = g_rng->RollRandomFloatInRange(0.f, WORLD_SIZE_Y);
		break;

	case 2: //top
		randomScreenPos.x = g_rng->RollRandomFloatInRange(0.f, WORLD_SIZE_X);
		randomScreenPos.y = WORLD_SIZE_Y + offset;
		break;

	case 3: //left
		randomScreenPos.x = -offset;
		randomScreenPos.y = g_rng->RollRandomFloatInRange(0.f, WORLD_SIZE_Y);
		break;
	}
	return randomScreenPos;
}

void Game::AdjustTimeDistortion()
{
	//#TODO: fix time issue with game over sequence
	if (m_inGameOverSequence) //gradually slows down game during end game sequence
	{
		float timeScale = Lerp(.75f, 0.1f, GetFractionWithinRange((float)m_gameOverTimer->GetElapsedTime(), 0, GAME_OVER_SEQUENCE_DURATION));
		m_clock->SetTimeScale(timeScale);
	}

}

Vec2 const Game::GetNearestPlayerPosition(Vec2 const& inPosition)
{
	Vec2 closestPlayerPos = m_playerShips[0]->m_position;
	float shortestDistance = 9999999999999.f;

	for (int playerNum = 0; playerNum < MAX_NUM_PLAYERS; ++playerNum)
	{
		if (m_playerShips[playerNum] == nullptr || !m_playerShips[playerNum]->IsAlive())
			continue;

		Vec2 currentPlayerPos = m_playerShips[playerNum]->m_position;
		float currentDistance = GetDistance2D(inPosition, currentPlayerPos);

		if (currentDistance < shortestDistance)
		{
			shortestDistance = currentDistance;
			closestPlayerPos = currentPlayerPos;
		}
	}

	return closestPlayerPos;
}

bool const Game::AllPlayersDead() const
{
	for (int playerNum = 0; playerNum < MAX_NUM_PLAYERS; ++playerNum)
	{
		if (m_playerShips[playerNum] == nullptr)
			continue;

		if (m_playerShips[playerNum]->IsAlive())
			return false;
	}

	return true;
}

PlayerShip* Game::GetPlayerShipByID(int idNum) const
{
	for (int playerNum = 0; playerNum < MAX_NUM_PLAYERS; ++playerNum)
	{
		if (m_playerShips[playerNum]->m_playerID == idNum)
		{
			return m_playerShips[playerNum];
		}
	}
	return nullptr;
}

int Game::GetPlayerNumFromPlayerID(int idNum) const
{
	for (int playerNum = 0; playerNum < MAX_NUM_PLAYERS; ++playerNum)
	{
		if (m_playerShips[playerNum]!= nullptr && m_playerShips[playerNum]->m_playerID == idNum)
		{
			return playerNum;
		}
	}
	return -1;
}

void Game::CheckNumRemainingPlayersForGameOver()
{
	int numPlayersAlive = 0;
	int winningPlayerNum = -1;
	
	for (int playerNum = 0; playerNum < m_numConnectedPlayers; ++playerNum)
	{
		if (m_numExtraLives[playerNum] > 0)
		{
			numPlayersAlive++;
			winningPlayerNum = playerNum;
		}

		else if (m_playerShips[playerNum] != nullptr && m_playerShips[playerNum]->IsAlive())
		{
			numPlayersAlive++;
			winningPlayerNum = playerNum;
		}

		if (numPlayersAlive >= 2)
			return;
	}

	if (numPlayersAlive <= 1)
	{
		GameOver(winningPlayerNum, true);
	}
}




