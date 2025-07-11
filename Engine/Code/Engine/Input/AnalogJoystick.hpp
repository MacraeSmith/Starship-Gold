#pragma once
#include "Engine/Math/Vec2.hpp"

class AnalogJoystick
{
public:
	Vec2 GetPosition() const;
	float GetMagnitude() const;
	float GetOrientationDegrees() const;

	Vec2 GetRawUncorrectedPosition() const;
	float GetInnerDeadZoneFraction() const;
	float GetOuterDeadZoneFraction() const;

	//for use by XboxController, et al.
	void Reset();
	void SetDeadZoneThresholds(float normalizedInnerDeadzoneThreshold, float normalizedOuterDeadzoneThreshold);
	void UpdatePosition(float rawNormalizedX, float rawNormalizedY);

protected:
	Vec2 m_rawPosition;
	Vec2 m_correctedPosition;
	float m_innerDeadZoneFraction = 0.3f;
	float m_outerDeadZoneFraction = 0.95f;

};

