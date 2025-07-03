#pragma once
#include "Engine/Math/Vec2.hpp"

class RendererDX11;
class App;
class RandomNumberGenerator;
class InputSystem;
struct Rgba8;
class AudioSystem;
class Window;

extern RendererDX11* g_renderer;
extern App* g_app;
extern RandomNumberGenerator* g_rng;
extern InputSystem* g_inputSystem;
extern AudioSystem* g_audioSystem;
extern Window* g_window;

//Max Number Entities
constexpr int MAX_NUM_PLAYERS = 4;
constexpr int MAX_ASTEROIDS = 50;
constexpr int MAX_BULLETS = 100;
constexpr int MAX_DEBRIS = 100;
constexpr int MAX_BEETLES = 100;
constexpr int MAX_WASPS = 100;
constexpr int MAX_POWERUPS = 10;

//World Size
constexpr float WORLD_SIZE_X = 200.f;
constexpr float WORLD_SIZE_Y = 100.f;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;

//Screen Size
constexpr float SCREEN_SIZE_X = 1600.f;
constexpr float SCREEN_SIZE_Y = 800.f;
constexpr float SCREEN_CENTER_X = SCREEN_SIZE_X / 2.f;
constexpr float SCREEN_CENTER_Y = SCREEN_SIZE_Y / 2.f;

//Screen Shake
constexpr float MAX_SCREENSHAKE_TRAUMA = 10.f;

//Asteroids
constexpr float ASTEROID_MIN_SPEED = 3.f;
constexpr float ASTEROID_MAX_SPEED = 10.f;
constexpr float ASTEROID_PHYSICS_RADIUS = 1.6f;
constexpr float ASTEROID_COSMETIC_RADIUS = 2.0f;
constexpr int ASTEROID_STARTING_HEALTH = 3;

//Bullets
constexpr float BULLET_LIFETIME_SECONDS = 2.0f;
constexpr float BULLET_SPEED = 50.f;
constexpr float BULLET_PHYSICS_RADIUS = 0.5f;
constexpr float BULLET_COSMETIC_RADIUS = 2.0f;

//Player Ship
constexpr float PLAYER_SHIP_ACCELERATION = 30.f;
constexpr float PLAYER_SHIP_TURN_SPEED = 300.f;
constexpr float PLAYER_SHIP_PHYSICS_RADIUS = 1.75f;
constexpr float PLAYER_SHIP_COSMETIC_RADIUS = 2.25f;
constexpr int PLAYER_SHIP_NUM_STARTING_LIVES = 5;
constexpr int PLAYER_SHIP_STARTING_HEALTH = 3;
constexpr float PLAYER_SHIP_SHIELD_RADIUS = 2.75f;

//Debris
constexpr float DEBRIS_PHYSICS_RADIUS = 0.5f;
constexpr float DEBRIS_COSMETIC_RADIUS = 1.f;
constexpr float DEBRIS_LIFETIME = 2.f;
constexpr float DEBRIS_MAX_SCATTER_SPEED = 8.f;

//Stars
constexpr int MAX_STARS = 200;

//Beetles
constexpr float BEETLE_SPEED = 10.f;
constexpr float BEETLE_PHYSICS_RADIUS = 2.5f;
constexpr float BEETLE_COSMETIC_RADIUS = 3.2f;
constexpr int BEETLE_STARTING_HEALTH = 3;

//Wasps
constexpr float WASP_ACCELERATION = 10.f;
constexpr float WASP_MAX_SPEED = 25.f;
constexpr float WASP_PHYSICS_RADIUS = 1.5f;
constexpr float WASP_COSMETIC_RADIUS = 2.f;
constexpr int WASP_STARTING_HEALTH = 2;

//Power Ups
constexpr float POWERUP_PHYSICS_RADIUS = 1.5f;
constexpr float POWERUP_COSMETIC_RADIUS = 2.2f;
constexpr float POWERUP_MAX_SPEED = 7.f;
constexpr float POWERUP_MIN_SPEED = 0.75f;

//Enemy Waves
constexpr int NUM_PLANNED_ENEMY_WAVES = 5;
constexpr int WASP_BIAS = 2;

//End of Game
constexpr float GAME_OVER_SEQUENCE_DURATION = 5.f;

//Debug
constexpr float DEBUG_LINE_THICKNESS = .2f;

void DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color);
void DebugDrawLine2D(Vec2 const& start, Vec2 const& end, float thickness, Rgba8 color);


