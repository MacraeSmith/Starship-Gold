#pragma once
#include "Engine/Math/Vec2.hpp"
#include"Engine/Core/Vertex_PCU.hpp"

class Game;
struct Rgba8;

class Entity
{
	
public:
	explicit Entity(Game* gameInstance, Vec2 const& startingPosition, float orientationDeg, Rgba8 color);
	explicit Entity(Game* gameInstance, Vec2 const& startingPosition, float orientationDeg);
	virtual ~Entity() {};

	//Frame Flow
	virtual void Update(float deltaSeconds)=0; //"=0" says Entity does not provide these functions but the classes that inherit from Entity must have them
	virtual void Render() const=0;
	virtual void DebugRender(Vec2 const& shipPos) const;
	virtual void DebugRender() const; //Debug render if you do not want to draw a line to shipPos

	//Health
	virtual void LoseHealth();
	virtual void Die();

	//Helper Mutators
	virtual void WrapToOppositeSide();
	virtual void TryToDropPowerUp(int percentageSuccess = 0) const;
	virtual void RotateToFacePosition(Vec2 const& position);
	virtual void ToggleDebugDraw();
	
	//Accessors
	bool const IsOffScreen() const;
	bool const IsAlive() const;
	bool const IsGarbage() const { return m_isGarbage; };
	Vec2 const GetForwardNormal() const;
	float const GetPhysicsRadius() const;

	
public:
	Vec2 m_position;
	Vec2 m_velocity;
	Rgba8 m_color;
	int m_health = 0;

protected:
	Game* m_game = nullptr;

	float m_orientationDegrees = 0.f;
	float m_angularVelocity = 0.f;
	float m_physicsRadius = 0.f;
	float m_cosmeticRadius = 1.f;
	float m_age = 0;
	bool m_isDead = false;
	bool m_isGarbage = false;
	bool m_shouldDrawDebug = false;
	

};

