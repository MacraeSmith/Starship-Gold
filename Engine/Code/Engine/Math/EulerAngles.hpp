#pragma once

struct Vec3;
struct Mat44;

struct EulerAngles
{
public:

	static const EulerAngles ZERO;

	float m_yawDegrees = 0.f;
	float m_pitchDegrees = 0.f;
	float m_rollDegrees = 0.f;

	EulerAngles() = default;
	EulerAngles(float yawDegrees, float pitchDegrees, float rollDegrees);
	void GetAsVectors_IFwd_JLeft_KUp(Vec3& out_forwardIBasis, Vec3& out_leftJBasis, Vec3& out_upKBasis) const;
	Mat44 GetAsMatrix_IFwd_JLeft_KUp() const;

	Vec3 Get_IFwd() const;
	Vec3 Get_JLeft() const;
	Vec3 Get_KUp() const;

	void SetFromText(char const* text);

	//Operators (const)
	const EulerAngles operator*(float uniformScale) const;

	//Operators (self - mutating)
	void operator+=(EulerAngles const& eulerAnglesToAdd);
	void operator*=(const float uniformScale);
};

