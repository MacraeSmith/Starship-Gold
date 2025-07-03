#include "Game/App.hpp"

#include "Engine/Renderer/RendererDX11.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine//Window/Window.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DevConsole.hpp"


#include "Game/Game.hpp"

App* g_app = nullptr;
RendererDX11* g_renderer = nullptr;
AudioSystem* g_audioSystem = nullptr;
Window* g_window = nullptr;
Game* m_game = nullptr;

//App Management
//-----------------------------------------------------------------------------------------------
void App::Startup()
{
	InputConfig inputConfig;
	g_inputSystem = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_aspectRatio = 2.0f;
	windowConfig.m_inputSystem = g_inputSystem;
	windowConfig.m_windowTitle = "SD1-A4: Starship Gold";
	g_window = new Window(windowConfig);

	RendererConfig rendererConfig;
	rendererConfig.m_window = g_window;
	g_renderer = new RendererDX11(rendererConfig);

	EventSystemConfig eventSystemConfig;
	g_eventSystem = new EventSystem(eventSystemConfig);

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_defaultFontName = "Data/Font/SquirrelFixedFont";
	devConsoleConfig.m_defaultLinesOnScreen = 29.5f;
	devConsoleConfig.m_defaultRenderer = g_renderer;
	devConsoleConfig.m_defaultFontAspect = 1.f;
	g_devConsole = new DevConsole(devConsoleConfig);

	AudioConfig audioConfig;
	g_audioSystem = new AudioSystem(audioConfig);

	g_window->Startup();
	g_renderer->Startup();
	g_renderer->BindTexture(nullptr);
	g_eventSystem->Startup();
	g_devConsole->Startup();
	g_inputSystem->Startup();
	g_audioSystem->Startup();

	m_game = new Game();
	m_game->Startup();

	SubscribeEventCallbackFunction("Quit", QuitEvent);
}

void App::Shutdown()
{
	m_game->Shutdown();
	delete m_game;
	m_game = nullptr;


	g_audioSystem->Shutdown();
	g_eventSystem->ShutDown();
	g_devConsole->ShutDown();
	g_renderer->Shutdown();
	g_window->Shutdown();
	g_inputSystem->Shutdown();

	delete g_audioSystem;
	g_audioSystem = nullptr;

	delete g_eventSystem;
	g_eventSystem = nullptr;

	delete g_devConsole;
	g_devConsole = nullptr;

	delete g_renderer;
	g_renderer = nullptr;

	delete g_window;
	g_window = nullptr;

	delete g_inputSystem;
	g_inputSystem = nullptr;
}

//Frame Flow
//-----------------------------------------------------------------------------------------------
void App::RunFrame()
{
	BeginFrame();
	Update();
	Render();
	EndFrame();
}

void App::BeginFrame()
{
	g_inputSystem->BeginFrame();
	g_window->BeginFrame();
	g_renderer->BeginFrame();
	g_eventSystem->BeginFrame();
	g_devConsole->BeginFrame();
	g_audioSystem->BeginFrame();
	m_game->BeginFrame();
	Clock::TickSystemClock();
}

void App::Update()
{
	m_game->Update();
}

void App::Render() const
{
	m_game->Render();
}

void App::EndFrame()
{
	m_game->EndFrame();
	g_audioSystem->EndFrame();
	g_eventSystem->EndFrame();
	g_devConsole->EndFrame();
	g_renderer->EndFrame();
	g_inputSystem->EndFrame();
}

bool App::QuitEvent(EventArgs& args)
{
	UNUSED(args);
	if (g_app != nullptr)
	{
		g_app->HandleQuitRequested();
	}

	return true;
}

//Input
//-----------------------------------------------------------------------------------------------
void App::HandleQuitRequested()
{
	m_isQuitting = true;
}

//Game Management
//-----------------------------------------------------------------------------------------------
void App::RestartGame()
{
	m_game->Shutdown();
	delete m_game;
	m_game = nullptr;

	m_game = new Game();
	m_game->Startup();
}
