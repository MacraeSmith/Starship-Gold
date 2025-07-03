#pragma once
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#define UNUSED(x) (void)(x);

class EventSystem;
class DevConsole;
class InputSystem;

extern NamedStrings g_gameConfigBlackboard;
extern EventSystem* g_eventSystem;
extern DevConsole* g_devConsole;
extern InputSystem* g_inputSystem;


