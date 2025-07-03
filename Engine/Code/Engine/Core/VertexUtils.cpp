#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Disc2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Triangle2.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <vector>

void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float uniformScaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY)
{
	// for loop through points in array
	for (int vertIndex = 0; vertIndex < numVerts; ++vertIndex)
	{
		Vec3& pos = verts[vertIndex].m_position;
		TransformPositionXY3D(pos, uniformScaleXY, rotationDegreesAboutZ, translationXY);
	}
}

void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, Vec2 const& vectorFwrd, Vec2 const& vectorLeft, Vec2 const& translationXY)
{
	// for loop through points in array
	for (int vertIndex = 0; vertIndex < numVerts; ++vertIndex)
	{
		Vec3& pos = verts[vertIndex].m_position;
		TransformPositionXY3D(pos, vectorFwrd, vectorLeft, translationXY);
	}
}

void TransformVertexArrayXY3D(Verts& verts, Vec2 const& vectorFwrd, Vec2 const& vectorLeft, Vec2 const& translationXY)
{
	for (int vertIndex = 0; vertIndex < (int)(verts.size()); ++vertIndex)
	{
		Vec3& pos = verts[vertIndex].m_position;
		TransformPositionXY3D(pos, vectorFwrd, vectorLeft, translationXY);
	}
}

void TransformVertexArrayXY3D(Verts& verts, Vec2 const& vectorFwrd, Vec2 const& vectorLeft, Vec2 const& translationXY, IntRange const& vertsToChangeIndexRange)
{
	for (int vertIndex = vertsToChangeIndexRange.m_min; vertIndex < vertsToChangeIndexRange.m_max + 1; ++vertIndex)
	{
		Vec3& pos = verts[vertIndex].m_position;
		TransformPositionXY3D(pos, vectorFwrd, vectorLeft, translationXY);
	}
}

void TransformVertexArray3D(Verts& verts, Mat44 const& transform)
{
	for (int vertIndex = 0; vertIndex < (int)verts.size(); ++vertIndex)
	{
		Vec3& pos = verts[vertIndex].m_position;
		TransformPosition3D(pos, transform);
	}
}

void TransformVertexArray3D(Verts& verts, Mat44 const& transform, IntRange const& vertsToChangeIndexRange)
{
	for (int vertIndex = vertsToChangeIndexRange.m_min; vertIndex < vertsToChangeIndexRange.m_max + 1; ++vertIndex)
	{
		Vec3& pos = verts[vertIndex].m_position;
		TransformPosition3D(pos, transform);
	}
}

void TransformVertexArray3D(VertTBNs& verts, Mat44 const& transform, IntRange const& vertsToChangeIndexRange)
{
	for (int vertIndex = vertsToChangeIndexRange.m_min; vertIndex < vertsToChangeIndexRange.m_max + 1; ++vertIndex)
	{
		Vec3& pos = verts[vertIndex].m_position;
		TransformPosition3D(pos, transform);
		verts[vertIndex].m_normal = transform.TransformVectorQuantity3D(verts[vertIndex].m_normal);
		verts[vertIndex].m_tangent = transform.TransformVectorQuantity3D(verts[vertIndex].m_tangent);
		verts[vertIndex].m_biTangent = transform.TransformVectorQuantity3D(verts[vertIndex].m_biTangent);
	}
}


void ChangeColorsOfVertexArray(int numVerts, Vertex_PCU* verts, Rgba8 const& color)
{
	for (int vertIndex = 0; vertIndex < numVerts; ++vertIndex)
	{
		verts[vertIndex].m_color = color;
	}
}

void AddVertsForDisc2D(Verts& verts, Vec2 const& discCenter, float const& discRadius, Rgba8 const& color, int numSlices)
{
	const float DEGREES_PER_SLICE = 360.f / numSlices;

	float sliceEndTheta = 0.f;
	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		float sliceStartTheta = sliceEndTheta;
		sliceEndTheta = sliceStartTheta + DEGREES_PER_SLICE;

		Vec2 sliceStartDisp = Vec2::MakeFromPolarDegrees(sliceStartTheta, discRadius);
		Vec2 sliceEndDisp = Vec2::MakeFromPolarDegrees(sliceEndTheta, discRadius);
		Vec2 sliceStartOuterPos = discCenter + sliceStartDisp;
		Vec2 sliceEndOuterPos = discCenter + sliceEndDisp;

		//TODO: ADD UV COORDS
		verts.push_back(Vertex_PCU(discCenter, color));
		verts.push_back(Vertex_PCU(sliceStartOuterPos, color));
		verts.push_back(Vertex_PCU(sliceEndOuterPos, color));
	}
}

void AddVertsForDisc2D(Verts& verts, Disc2 const& disc, Rgba8 const& color)
{
	constexpr int NUM_SLICES = 32;
	constexpr float DEGREES_PER_SLICE = 360.f / (float)(NUM_SLICES);
	float sliceEndTheta = 0.f;
	for (int sliceNum = 0; sliceNum < NUM_SLICES; ++sliceNum)
	{
		float sliceStartTheta = sliceEndTheta;
		sliceEndTheta = sliceStartTheta + DEGREES_PER_SLICE;

		Vec2 sliceStartDisp = Vec2::MakeFromPolarDegrees(sliceStartTheta, disc.m_radius);
		Vec2 sliceEndDisp = Vec2::MakeFromPolarDegrees(sliceEndTheta, disc.m_radius);
		Vec2 sliceStartOuterPos = disc.m_center + sliceStartDisp;
		Vec2 sliceEndOuterPos = disc.m_center + sliceEndDisp;

		//#TODO: ADD UV COORDS
		verts.push_back(Vertex_PCU(disc.m_center, color));
		verts.push_back(Vertex_PCU(sliceStartOuterPos, color));
		verts.push_back(Vertex_PCU(sliceEndOuterPos, color));
	}
}

void AddVertsForAABB2D(Verts& verts, AABB2 const& alignedBox, Rgba8 const& color, Vec2 const& uvMins, Vec2 const& uvMaxs)
{
	Vec2 BL = alignedBox.m_mins;
	Vec2 BR = Vec2(alignedBox.m_maxs.x, alignedBox.m_mins.y);
	Vec2 TL = Vec2(alignedBox.m_mins.x, alignedBox.m_maxs.y);
	Vec2 TR = alignedBox.m_maxs;

	verts.push_back(Vertex_PCU(BL, color, uvMins));
	verts.push_back(Vertex_PCU(BR, color, Vec2(uvMaxs.x, uvMins.y)));
	verts.push_back(Vertex_PCU(TR, color, uvMaxs));
	verts.push_back(Vertex_PCU(BL, color, uvMins));
	verts.push_back(Vertex_PCU(TR, color,uvMaxs));
	verts.push_back(Vertex_PCU(TL, color,Vec2(uvMins.x, uvMaxs.y)));

}

void AddVertsForAABB2D(Verts& verts, AABB2 const& alignedBox, Rgba8 const& color, AABB2 const& uvs)
{
	Vec2 BL = alignedBox.m_mins;
	Vec2 BR = Vec2(alignedBox.m_maxs.x, alignedBox.m_mins.y);
	Vec2 TL = Vec2(alignedBox.m_mins.x, alignedBox.m_maxs.y);
	Vec2 TR = alignedBox.m_maxs;

	verts.push_back(Vertex_PCU(BL, color, uvs.m_mins));
	verts.push_back(Vertex_PCU(BR, color, Vec2(uvs.m_maxs.x, uvs.m_mins.y)));
	verts.push_back(Vertex_PCU(TR, color, uvs.m_maxs));
	verts.push_back(Vertex_PCU(BL, color, uvs.m_mins));
	verts.push_back(Vertex_PCU(TR, color, uvs.m_maxs));
	verts.push_back(Vertex_PCU(TL, color, Vec2(uvs.m_mins.x, uvs.m_maxs.y)));
}

void AddVertsForOBB2D(Verts& verts, OBB2 const& orientedBox, Rgba8 const& color)
{
	Vec2 cornerPoints[4];
	orientedBox.GetCornerPoints(cornerPoints);
	Vec2 BL = cornerPoints[0];
	Vec2 BR = cornerPoints[1];
	Vec2 TR = cornerPoints[2];
	Vec2 TL = cornerPoints[3];

	verts.push_back(Vertex_PCU(BL, color, Vec2::ZERO));
	verts.push_back(Vertex_PCU(BR, color, Vec2::ONE_TO_ZERO));
	verts.push_back(Vertex_PCU(TR, color, Vec2::ONE));
	verts.push_back(Vertex_PCU(BL, color, Vec2::ZERO));
	verts.push_back(Vertex_PCU(TR, color, Vec2::ONE));
	verts.push_back(Vertex_PCU(TL, color, Vec2::ZERO_TO_ONE));
}

void AddVertsForOBB2D(Verts& verts, Vec2 const& center, Vec2 const& iBasisNormal, Vec2 const& halfDimensionsIJ, Rgba8 const& color)
{
	OBB2 orientedBox(center, iBasisNormal, halfDimensionsIJ);
	Vec2 cornerPoints[4];
	orientedBox.GetCornerPoints(cornerPoints);
	Vec2 BL = cornerPoints[0];
	Vec2 BR = cornerPoints[1];
	Vec2 TR = cornerPoints[2];
	Vec2 TL = cornerPoints[3];

	verts.push_back(Vertex_PCU(BL, color, Vec2::ZERO));
	verts.push_back(Vertex_PCU(BR, color, Vec2::ONE_TO_ZERO));
	verts.push_back(Vertex_PCU(TR, color, Vec2::ONE));
	verts.push_back(Vertex_PCU(BL, color, Vec2::ZERO));
	verts.push_back(Vertex_PCU(TR, color, Vec2::ONE));
	verts.push_back(Vertex_PCU(TL, color, Vec2::ZERO_TO_ONE));
}


void AddVertsForCapsule2D(Verts& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float const& radius, Rgba8 const& color)
{
	//#TODO: ADD UV COORDS

	Vec2 fwrdStep = boneEnd - boneStart;
	fwrdStep.SetLength(radius);
	Vec2 leftStep = fwrdStep.GetRotated90Degrees();

	Vec2 EL = boneEnd + leftStep;
	Vec2 ER = boneEnd - leftStep;
	Vec2 SR = boneStart - leftStep;
	Vec2 SL = boneStart + leftStep;

	//draw 2 triangles to make the middle rectangle
	verts.push_back(Vertex_PCU(SR, color)); 
	verts.push_back(Vertex_PCU(ER, color));
	verts.push_back(Vertex_PCU(EL, color));

	verts.push_back(Vertex_PCU(SL, color));
	verts.push_back(Vertex_PCU(SR, color));
	verts.push_back(Vertex_PCU(EL, color));

	//precalculate for drawing end caps
	constexpr int NUM_SLICES = 16;
	constexpr float DEGREES_PER_SLICE = 180.f / (float)(NUM_SLICES);

	float boneFwrdOrientationTheta = fwrdStep.GetOrientationDegrees();
	float endCapStartTheta = boneFwrdOrientationTheta - 90.f;
	float endCapSliceEndTheta = endCapStartTheta;

	float startCapStartTheta = endCapStartTheta + 180.f;
	float startCapSliceEndTheta = startCapStartTheta;

	//Add verts for both caps
	for (int sliceNum = 0; sliceNum < NUM_SLICES; ++sliceNum)
	{
		//end Cap
		float sliceStartTheta = endCapSliceEndTheta;
		endCapSliceEndTheta = sliceStartTheta + DEGREES_PER_SLICE;
		Vec2 sliceStartDisp = Vec2::MakeFromPolarDegrees(sliceStartTheta, radius);
		Vec2 sliceEndDisp = Vec2::MakeFromPolarDegrees(endCapSliceEndTheta, radius);
		Vec2 sliceStartOuterPos = boneEnd + sliceStartDisp;
		Vec2 sliceEndOuterPos = boneEnd + sliceEndDisp;

		verts.push_back(Vertex_PCU(boneEnd, color));
		verts.push_back(Vertex_PCU(sliceStartOuterPos, color));
		verts.push_back(Vertex_PCU(sliceEndOuterPos, color));

		//start Cap
		sliceStartTheta = startCapSliceEndTheta;
		startCapSliceEndTheta = sliceStartTheta + DEGREES_PER_SLICE;
		sliceStartDisp = Vec2::MakeFromPolarDegrees(sliceStartTheta, radius);
		sliceEndDisp = Vec2::MakeFromPolarDegrees(startCapSliceEndTheta, radius);
		sliceStartOuterPos = boneStart + sliceStartDisp;
		sliceEndOuterPos = boneStart + sliceEndDisp;

		verts.push_back(Vertex_PCU(boneStart, color));
		verts.push_back(Vertex_PCU(sliceStartOuterPos, color));
		verts.push_back(Vertex_PCU(sliceEndOuterPos, color));
	}
}

void AddVertsForAABB2DFrame(Verts& verts, AABB2 const& alignedBox, float thickness, Rgba8 const& color)
{
	const float HALF_THICKNESS = thickness * 0.5f;

	//TopBar
	AddVertsForLineSegment2D(verts, Vec2(alignedBox.m_mins.x, alignedBox.m_maxs.y + HALF_THICKNESS), Vec2(alignedBox.m_maxs.x, alignedBox.m_maxs.y + HALF_THICKNESS), thickness, color);

	//BottomBar
	AddVertsForLineSegment2D(verts, Vec2(alignedBox.m_mins.x, alignedBox.m_mins.y - HALF_THICKNESS), Vec2(alignedBox.m_maxs.x, alignedBox.m_mins.y - HALF_THICKNESS), thickness, color);

	//LeftBar
	AddVertsForLineSegment2D(verts, Vec2(alignedBox.m_mins.x - HALF_THICKNESS, alignedBox.m_mins.y - HALF_THICKNESS), Vec2(alignedBox.m_mins.x - HALF_THICKNESS, alignedBox.m_maxs.y + HALF_THICKNESS), thickness, color);

	//RightBar
	AddVertsForLineSegment2D(verts, Vec2(alignedBox.m_maxs.x + HALF_THICKNESS, alignedBox.m_mins.y - HALF_THICKNESS), Vec2(alignedBox.m_maxs.x + HALF_THICKNESS, alignedBox.m_maxs.y + HALF_THICKNESS), thickness, color);



}

void AddVertsForCapsule2D(Verts& verts, Capsule2 const& capsule, Rgba8 const& color)
{
	//#TODO: ADD UV COORDS

	Vec2 boneStart = capsule.m_start;
	Vec2 boneEnd = capsule.m_end;
	float radius = capsule.m_radius;

	Vec2 fwrdStep = boneEnd - boneStart;
	fwrdStep.SetLength(radius);
	Vec2 leftStep = fwrdStep.GetRotated90Degrees();

	Vec2 EL = boneEnd + leftStep;
	Vec2 ER = boneEnd - leftStep;
	Vec2 SR = boneStart - leftStep;
	Vec2 SL = boneStart + leftStep;

	//draw 2 triangles to make the middle rectangle
	verts.push_back(Vertex_PCU(SR, color));
	verts.push_back(Vertex_PCU(ER, color));
	verts.push_back(Vertex_PCU(EL, color));

	verts.push_back(Vertex_PCU(SL, color));
	verts.push_back(Vertex_PCU(SR, color));
	verts.push_back(Vertex_PCU(EL, color));

	//precalculate for drawing end caps
	constexpr int NUM_SLICES = 16;
	constexpr float DEGREES_PER_SLICE = 180.f / (float)(NUM_SLICES);

	float boneFwrdOrientationTheta = fwrdStep.GetOrientationDegrees();
	float endCapStartTheta = boneFwrdOrientationTheta - 90.f;
	float endCapSliceEndTheta = endCapStartTheta;

	float startCapStartTheta = endCapStartTheta + 180.f;
	float startCapSliceEndTheta = startCapStartTheta;

	//Add verts for both caps
	for (int sliceNum = 0; sliceNum < NUM_SLICES; ++sliceNum)
	{
		//end Cap
		float sliceStartTheta = endCapSliceEndTheta;
		endCapSliceEndTheta = sliceStartTheta + DEGREES_PER_SLICE;
		Vec2 sliceStartDisp = Vec2::MakeFromPolarDegrees(sliceStartTheta, radius);
		Vec2 sliceEndDisp = Vec2::MakeFromPolarDegrees(endCapSliceEndTheta, radius);
		Vec2 sliceStartOuterPos = boneEnd + sliceStartDisp;
		Vec2 sliceEndOuterPos = boneEnd + sliceEndDisp;

		verts.push_back(Vertex_PCU(boneEnd, color));
		verts.push_back(Vertex_PCU(sliceStartOuterPos, color));
		verts.push_back(Vertex_PCU(sliceEndOuterPos, color));

		//start Cap
		sliceStartTheta = startCapSliceEndTheta;
		startCapSliceEndTheta = sliceStartTheta + DEGREES_PER_SLICE;
		sliceStartDisp = Vec2::MakeFromPolarDegrees(sliceStartTheta, radius);
		sliceEndDisp = Vec2::MakeFromPolarDegrees(startCapSliceEndTheta, radius);
		sliceStartOuterPos = boneStart + sliceStartDisp;
		sliceEndOuterPos = boneStart + sliceEndDisp;

		verts.push_back(Vertex_PCU(boneStart, color));
		verts.push_back(Vertex_PCU(sliceStartOuterPos, color));
		verts.push_back(Vertex_PCU(sliceEndOuterPos, color));
	}
}

void AddVertsForTriangle2D(Verts& verts, Vec2 const& triPointACounterClockwise, Vec2 const& triPointBCounterClockwise, Vec2 const& triPointCCounterClockwise, Rgba8 const& color)
{
	//#TODO: ADD UV COORDS
	verts.push_back(Vertex_PCU(triPointACounterClockwise, color));
	verts.push_back(Vertex_PCU(triPointBCounterClockwise, color));
	verts.push_back(Vertex_PCU(triPointCCounterClockwise, color));
}

void AddVertsForTriangle2D(Verts& verts, Triangle2 const& triangle, Rgba8 const& color)
{
	//#TODO: ADD UV COORDS
	verts.push_back(Vertex_PCU(triangle.m_pointsCounterClockwise[0], color));
	verts.push_back(Vertex_PCU(triangle.m_pointsCounterClockwise[1], color));
	verts.push_back(Vertex_PCU(triangle.m_pointsCounterClockwise[2], color));
}

void AddVertsForLineSegment2D(Verts& verts, Vec2 const& start, Vec2 const& end, float const& thickness, Rgba8 const& color)
{
	Vec2 displacement = end - start;
	float halfThickness = thickness * 0.5f;
	Vec2 stepFwrd = halfThickness * displacement.GetNormalized();
	Vec2 stepLeft = stepFwrd.GetRotated90Degrees();
	
	Vec2 endLeftVert = end + stepFwrd + stepLeft;
	Vec2 endRightVert = end + stepFwrd - stepLeft;
	Vec2 startLeftVert = start - stepFwrd + stepLeft;
	Vec2 startRightVert = start - stepFwrd - stepLeft;

	verts.push_back(Vertex_PCU(startRightVert, color, Vec2::ONE_TO_ZERO));
	verts.push_back(Vertex_PCU(endLeftVert, color, Vec2::ZERO_TO_ONE));
	verts.push_back(Vertex_PCU(startLeftVert, color, Vec2::ZERO));
	verts.push_back(Vertex_PCU(endLeftVert, color, Vec2::ZERO_TO_ONE));
	verts.push_back(Vertex_PCU(startRightVert, color, Vec2::ONE_TO_ZERO));
	verts.push_back(Vertex_PCU(endRightVert, color, Vec2::ONE));

}

void AddVertsForLineSegment2D(Verts& verts, Vec2 const& start, Vec2 const& end, float const& thickness, Rgba8 const& startColor, Rgba8 const& endColor)
{
	Vec2 displacement = end - start;
	float halfThickness = thickness * 0.5f;
	Vec2 stepFwrd = halfThickness * displacement.GetNormalized();
	Vec2 stepLeft = stepFwrd.GetRotated90Degrees();

	Vec2 endLeftVert = end + stepFwrd + stepLeft;
	Vec2 endRightVert = end + stepFwrd - stepLeft;
	Vec2 startLeftVert = start - stepFwrd + stepLeft;
	Vec2 startRightVert = start - stepFwrd - stepLeft;

	verts.push_back(Vertex_PCU(startRightVert, startColor, Vec2::ONE_TO_ZERO));
	verts.push_back(Vertex_PCU(endLeftVert, endColor, Vec2::ZERO_TO_ONE));
	verts.push_back(Vertex_PCU(startLeftVert, startColor, Vec2::ZERO));
	verts.push_back(Vertex_PCU(endLeftVert, endColor, Vec2::ZERO_TO_ONE));
	verts.push_back(Vertex_PCU(startRightVert, startColor, Vec2::ONE_TO_ZERO));
	verts.push_back(Vertex_PCU(endRightVert, endColor, Vec2::ONE));
}

void AddVertsForLineSegment2D(Verts& verts, LineSegment2 const& lineSegment, float const& thickness, Rgba8 const& color)
{
	Vec2 start = lineSegment.m_start;
	Vec2 end = lineSegment.m_end;

	Vec2 displacement = end - start;
	float halfThickness = thickness * 0.5f;
	Vec2 stepFwrd = halfThickness * displacement.GetNormalized();
	Vec2 stepLeft = stepFwrd.GetRotated90Degrees();

	Vec2 endLeftVert = end + stepFwrd + stepLeft;
	Vec2 endRightVert = end + stepFwrd - stepLeft;
	Vec2 startLeftVert = start - stepFwrd + stepLeft;
	Vec2 startRightVert = start - stepFwrd - stepLeft;

	verts.push_back(Vertex_PCU(startRightVert, color, Vec2::ONE_TO_ZERO));
	verts.push_back(Vertex_PCU(endLeftVert, color, Vec2::ZERO_TO_ONE));
	verts.push_back(Vertex_PCU(startLeftVert, color, Vec2::ZERO));
	verts.push_back(Vertex_PCU(endLeftVert, color, Vec2::ZERO_TO_ONE));
	verts.push_back(Vertex_PCU(startRightVert, color, Vec2::ONE_TO_ZERO));
	verts.push_back(Vertex_PCU(endRightVert, color, Vec2::ONE));
}

void AddVertsForLineSegment2D(Verts& verts, LineSegment2 const& lineSegment, float const& thickness, Rgba8 const& startColor, Rgba8 const& endColor)
{
	Vec2 start = lineSegment.m_start;
	Vec2 end = lineSegment.m_end;

	Vec2 displacement = end - start;
	float halfThickness = thickness * 0.5f;
	Vec2 stepFwrd = halfThickness * displacement.GetNormalized();
	Vec2 stepLeft = stepFwrd.GetRotated90Degrees();

	Vec2 endLeftVert = end + stepFwrd + stepLeft;
	Vec2 endRightVert = end + stepFwrd - stepLeft;
	Vec2 startLeftVert = start - stepFwrd + stepLeft;
	Vec2 startRightVert = start - stepFwrd - stepLeft;

	verts.push_back(Vertex_PCU(startRightVert, startColor, Vec2::ONE_TO_ZERO));
	verts.push_back(Vertex_PCU(endLeftVert, endColor, Vec2::ZERO_TO_ONE));
	verts.push_back(Vertex_PCU(startLeftVert, startColor, Vec2::ZERO));
	verts.push_back(Vertex_PCU(endLeftVert, endColor, Vec2::ZERO_TO_ONE));
	verts.push_back(Vertex_PCU(startRightVert, startColor, Vec2::ONE_TO_ZERO));
	verts.push_back(Vertex_PCU(endRightVert, endColor, Vec2::ONE));
}

void AddVertsForArrow2D(Verts& verts, Vec2 const& start, Vec2 const& end, float arrowSize, float thickness, Rgba8 const& color)
{
	AddVertsForLineSegment2D(verts, start, end, thickness, color);
	Vec2 fwrdVecWithSizeMagnitude = Vec2(end - start);
	fwrdVecWithSizeMagnitude.SetLength(arrowSize);

	Vec2 jVecWithSizeMagnitude = fwrdVecWithSizeMagnitude.GetRotated90Degrees();
	Vec2 endMinusFwrdVecPos = end - fwrdVecWithSizeMagnitude;

	Vec2 leftWingPos = endMinusFwrdVecPos + jVecWithSizeMagnitude;
	Vec2 rightWingPos = endMinusFwrdVecPos - jVecWithSizeMagnitude;

	AddVertsForLineSegment2D(verts, end, leftWingPos, thickness, color);
	AddVertsForLineSegment2D(verts, end, rightWingPos, thickness, color);

}

void AddVertsForRing2D(Verts& verts, Vec2 const& center, float radius, float thickness, Rgba8 const& color)
{
	//#TODO: add UVs

	float halfThickness = 0.5f * thickness;
	float innerRadius = radius - halfThickness;
	float outerRadius = radius + halfThickness;
	constexpr int NUM_SIDES = 32;
	constexpr float DEGREES_PER_SIDE = 360.f / (float)(NUM_SIDES);

	for (int sideNum = 0; sideNum < NUM_SIDES; ++sideNum)
	{
		//compute angle related terms
		float startDegrees = DEGREES_PER_SIDE * (float)(sideNum);
		float endDegrees = DEGREES_PER_SIDE * (float)(sideNum + 1);
		float cosStart = CosDegrees(startDegrees);
		float sinStart = SinDegrees(startDegrees);
		float cosEnd = CosDegrees(endDegrees);
		float sinEnd = SinDegrees(endDegrees);

		//compute inner and outer positions
		Vec3 innerStartPos(center.x + innerRadius * cosStart, center.y + innerRadius * sinStart, 0.0f);
		Vec3 outerStartPos(center.x + outerRadius * cosStart, center.y + outerRadius * sinStart, 0.f);
		Vec3 outerEndPos(center.x + outerRadius * cosEnd, center.y + outerRadius * sinEnd, 0.f);
		Vec3 innerEndPos(center.x + innerRadius * cosEnd, center.y + innerRadius * sinEnd, 0.f);

		verts.push_back(Vertex_PCU(innerEndPos, color, Vec2::ZERO));
		verts.push_back(Vertex_PCU(innerStartPos, color, Vec2::ZERO));
		verts.push_back(Vertex_PCU(outerStartPos, color, Vec2::ZERO));
		verts.push_back(Vertex_PCU(innerEndPos, color, Vec2::ZERO));
		verts.push_back(Vertex_PCU(outerStartPos, color, Vec2::ZERO));
		verts.push_back(Vertex_PCU(outerEndPos, color, Vec2::ZERO));
	}
}

void AddVertsForQuad3D(Verts& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& uvs)
{
	verts.push_back(Vertex_PCU(bottomLeft, color, uvs.m_mins));
	verts.push_back(Vertex_PCU(bottomRight, color, Vec2(uvs.m_maxs.x, uvs.m_mins.y)));
	verts.push_back(Vertex_PCU(topRight, color, uvs.m_maxs));

	verts.push_back(Vertex_PCU(bottomLeft, color, uvs.m_mins));
	verts.push_back(Vertex_PCU(topRight, color, Vec2(uvs.m_maxs.x, uvs.m_maxs.y)));
	verts.push_back(Vertex_PCU(topLeft, color, Vec2(uvs.m_mins.x, uvs.m_maxs.y)));
}

void AddVertsForIndexedQuad3D(Verts& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& uvs)
{
	const int START_INDEX = (int)verts.size();
	verts.push_back(Vertex_PCU(bottomLeft, color, uvs.m_mins));
	verts.push_back(Vertex_PCU(bottomRight, color, Vec2(uvs.m_maxs.x, uvs.m_mins.y)));

	verts.push_back(Vertex_PCU(topRight, color, Vec2(uvs.m_maxs.x, uvs.m_maxs.y)));
	verts.push_back(Vertex_PCU(topLeft, color, Vec2(uvs.m_mins.x, uvs.m_maxs.y)));

	indexes.push_back(START_INDEX);
	indexes.push_back(START_INDEX + 1);
	indexes.push_back(START_INDEX + 2);
	indexes.push_back(START_INDEX);
	indexes.push_back(START_INDEX + 2);
	indexes.push_back(START_INDEX + 3);
}

void AddVertsForIndexedQuad3D(VertTBNs& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& uvs)
{
	const int START_INDEX = (int)verts.size();
	Vec3 tangent = (bottomRight - bottomLeft).GetNormalized();
	Vec3 biTangent = (topLeft - bottomLeft).GetNormalized();
	Vec3 normal = CrossProduct3D(tangent, biTangent).GetNormalized();
	verts.push_back(Vertex_PCUTBN(bottomLeft, color, tangent, biTangent, normal, uvs.m_mins ));
	verts.push_back(Vertex_PCUTBN(bottomRight, color, tangent, biTangent, normal, Vec2(uvs.m_maxs.x, uvs.m_mins.y)));

	verts.push_back(Vertex_PCUTBN(topRight, color, tangent, biTangent, normal, Vec2(uvs.m_maxs.x, uvs.m_maxs.y)));
	verts.push_back(Vertex_PCUTBN(topLeft, color, tangent, biTangent, normal, Vec2(uvs.m_mins.x, uvs.m_maxs.y)));

	indexes.push_back(START_INDEX);
	indexes.push_back(START_INDEX + 1);
	indexes.push_back(START_INDEX + 2);
	indexes.push_back(START_INDEX);
	indexes.push_back(START_INDEX + 2);
	indexes.push_back(START_INDEX + 3);
}

void AddVertsForRoundedQuad(VertTBNs& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& uvs)
{
	Vec3 bottomDifference = bottomRight - bottomLeft;
	Vec3 bottomMiddle = bottomLeft + (bottomDifference * 0.5f);
	Vec3 topDifference = topRight - topLeft;
	Vec3 topMiddle = topLeft + (topDifference * 0.5f);

	Vec3 tangent(0.f, 0.f, 0.f);
	Vec3 biTangent(0.f, 0.f, 0.f);
	Vec3 middleNormal = CrossProduct3D((bottomRight - bottomLeft), (topLeft - bottomLeft)).GetNormalized();
	Vec3 rightNormal = bottomDifference.GetNormalized();
	Vec3 leftNormal = -rightNormal;

	Vec2 uvDifference = uvs.m_maxs - uvs.m_mins;
	float uvMiddleX = uvs.m_mins.x + (uvDifference.x * 0.5f);


	verts.push_back(Vertex_PCUTBN(bottomLeft, color, tangent, biTangent, leftNormal, uvs.m_mins));
	verts.push_back(Vertex_PCUTBN(bottomMiddle, color, tangent, biTangent, middleNormal, Vec2(uvMiddleX, uvs.m_mins.y)));
	verts.push_back(Vertex_PCUTBN(topMiddle, color, tangent, biTangent, middleNormal, Vec2(uvMiddleX, uvs.m_maxs.y)));

	verts.push_back(Vertex_PCUTBN(bottomLeft, color, tangent, biTangent, leftNormal, uvs.m_mins));
	verts.push_back(Vertex_PCUTBN(topMiddle, color, tangent, biTangent, middleNormal, Vec2(uvMiddleX, uvs.m_maxs.y)));
	verts.push_back(Vertex_PCUTBN(topLeft, color, tangent, biTangent, leftNormal, Vec2(uvs.m_mins.x, uvs.m_maxs.y)));

	verts.push_back(Vertex_PCUTBN(bottomMiddle, color, tangent, biTangent, middleNormal, Vec2(uvMiddleX, uvs.m_mins.y)));
	verts.push_back(Vertex_PCUTBN(bottomRight, color, tangent, biTangent, rightNormal, Vec2(uvs.m_maxs.x, uvs.m_mins.y)));
	verts.push_back(Vertex_PCUTBN(topRight, color, tangent, biTangent, rightNormal, uvs.m_maxs));

	verts.push_back(Vertex_PCUTBN(bottomMiddle, color, tangent, biTangent, middleNormal, Vec2(uvMiddleX, uvs.m_mins.y)));
	verts.push_back(Vertex_PCUTBN(topRight, color, tangent, biTangent, rightNormal, Vec2(uvs.m_maxs.x, uvs.m_maxs.y)));
	verts.push_back(Vertex_PCUTBN(topMiddle, color, tangent, biTangent, middleNormal, Vec2(uvMiddleX, uvs.m_maxs.y)));
}


void AddVertsForAABB3D(Verts& verts, AABB3 const& bounds, Rgba8 const& color, AABB2 const& uvs)
{
	AddVertsForQuad3D(verts, Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z), color, uvs);	// x face
	AddVertsForQuad3D(verts, Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z), Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z), color, uvs);	// -x face
	AddVertsForQuad3D(verts, Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z), color, uvs);	// y face
	AddVertsForQuad3D(verts, Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z), Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z), color, uvs);	// -y face
	AddVertsForQuad3D(verts, Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z), Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z), color, uvs);	// z face
	AddVertsForQuad3D(verts, Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z), color, uvs);	// -z face
}

void AddVertsForAABB3D(Verts& verts, AABB3 const& bounds, Rgba8* faceColors_XPos_XNeg_YPos_YNeg_ZPos_ZNeg, AABB2 const& uvs)
{
	AddVertsForQuad3D(verts, Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z), faceColors_XPos_XNeg_YPos_YNeg_ZPos_ZNeg[0], uvs);	// x face
	AddVertsForQuad3D(verts, Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z), Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z), faceColors_XPos_XNeg_YPos_YNeg_ZPos_ZNeg[1], uvs);	// -x face
	AddVertsForQuad3D(verts, Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z), faceColors_XPos_XNeg_YPos_YNeg_ZPos_ZNeg[2], uvs);	// y face
	AddVertsForQuad3D(verts, Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z), Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z), faceColors_XPos_XNeg_YPos_YNeg_ZPos_ZNeg[3], uvs);	// -y face
	AddVertsForQuad3D(verts, Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z), Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z), faceColors_XPos_XNeg_YPos_YNeg_ZPos_ZNeg[4], uvs);	// z face
	AddVertsForQuad3D(verts, Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z), faceColors_XPos_XNeg_YPos_YNeg_ZPos_ZNeg[5], uvs);	// -z face
}

void AddVertsForIndexedAABB3D(VertTBNs& verts, std::vector<unsigned int>& indexes, AABB3 const& bounds, Rgba8 const& color, AABB2 const& uvs)
{
	AddVertsForIndexedQuad3D(verts, indexes, Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z), color, uvs);	// x face
	AddVertsForIndexedQuad3D(verts, indexes, Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z), Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z), color, uvs);	// -x face
	AddVertsForIndexedQuad3D(verts, indexes, Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z), color, uvs);	// y face
	AddVertsForIndexedQuad3D(verts, indexes, Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z), Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z), color, uvs);	// -y face
	AddVertsForIndexedQuad3D(verts, indexes, Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z), Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z), color, uvs);	// z face
	AddVertsForIndexedQuad3D(verts, indexes, Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z), color, uvs);	// -z face
}

void AddVertsForOBB3D(Verts& verts, OBB3 const& orientedBox, Rgba8 const& color, AABB2 const& uvs)
{
	const int START_INDEX = (int)verts.size();
	AABB3 bounds( -orientedBox.m_halfDimensionsIJK.x,  -orientedBox.m_halfDimensionsIJK.y,  -orientedBox.m_halfDimensionsIJK.z,
		orientedBox.m_halfDimensionsIJK.x, orientedBox.m_halfDimensionsIJK.y, orientedBox.m_halfDimensionsIJK.z);

	AddVertsForQuad3D(verts, Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z), color, uvs);	// x face
	AddVertsForQuad3D(verts, Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z), Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z), color, uvs);	// -x face
	AddVertsForQuad3D(verts, Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z), color, uvs);	// y face
	AddVertsForQuad3D(verts, Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z), Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z), color, uvs);	// -y face
	AddVertsForQuad3D(verts, Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z), Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z), color, uvs);	// z face
	AddVertsForQuad3D(verts, Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z), color, uvs);	// -z face

	Mat44 transform(orientedBox.m_iBasis, orientedBox.m_jBasis, orientedBox.m_kBasis, orientedBox.m_center );
	TransformVertexArray3D(verts, transform, IntRange(START_INDEX, (int)verts.size() - 1));
}

void AddVertsForIndexedOBB3D(VertTBNs& verts, std::vector<unsigned int>& indexes, OBB3 const& orientedBox, Rgba8 const& color, AABB2 const& uvs)
{
	const int START_INDEX = (int)verts.size();
	AABB3 bounds(-orientedBox.m_halfDimensionsIJK.x, -orientedBox.m_halfDimensionsIJK.y, -orientedBox.m_halfDimensionsIJK.z,
		orientedBox.m_halfDimensionsIJK.x, orientedBox.m_halfDimensionsIJK.y, orientedBox.m_halfDimensionsIJK.z);

	AddVertsForIndexedQuad3D(verts, indexes, Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z), color, uvs);	// x face
	AddVertsForIndexedQuad3D(verts, indexes, Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z), Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z), color, uvs);	// -x face
	AddVertsForIndexedQuad3D(verts, indexes, Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z), color, uvs);	// y face
	AddVertsForIndexedQuad3D(verts, indexes, Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z), Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z), color, uvs);	// -y face
	AddVertsForIndexedQuad3D(verts, indexes, Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z), Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z), color, uvs);	// z face
	AddVertsForIndexedQuad3D(verts, indexes, Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z), Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z), Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z), color, uvs);	// -z face

	Mat44 transform(orientedBox.m_iBasis, orientedBox.m_jBasis, orientedBox.m_kBasis, orientedBox.m_center);
	TransformVertexArray3D(verts, transform, IntRange(START_INDEX, (int)verts.size() - 1));
}

void AddVertsForGridPlane3D(Verts& verts, IntVec2 const& xyUnitDimensions, Vec3 const& center, int unitScale, int accentUnitScale, float lineThickness)
{
	int xRows = (int)floorf((float)xyUnitDimensions.x * 0.5f); // rows in either direction starting at center
	int yRows = (int)floorf((float)xyUnitDimensions.y * 0.5f);

	Vec2 offsetXY = Vec2(center.x + (yRows * unitScale), center.y + (xRows * unitScale));
	AABB2 gridBounds(center.x - offsetXY.x, center.y - offsetXY.y, center.x + offsetXY.x, center.y + offsetXY.y);

	float halfLineThick = lineThickness * 0.5f;
	float halfAccentLineThick = lineThickness * 1.5f;
	float halfCenterLineThick = lineThickness * 3.f;

	const Rgba8 X_COLOR = Rgba8::RED;
	const Rgba8 Y_COLOR = Rgba8::GREEN;
	const Rgba8 GRID_COLOR = Rgba8::GREY;

	//center lines

	//x
	AABB3 lineBounds(Vec3(gridBounds.m_mins.x, center.y - halfCenterLineThick, center.z - halfCenterLineThick), 
		Vec3(gridBounds.m_maxs.x, center.y + halfCenterLineThick, center.z + halfCenterLineThick));
	AddVertsForAABB3D(verts, lineBounds, X_COLOR);

	//y
	halfCenterLineThick *= 0.9f;
	lineBounds = AABB3(Vec3(center.x - halfCenterLineThick, gridBounds.m_mins.y, center.z - halfCenterLineThick), 
		Vec3(center.x + halfCenterLineThick, gridBounds.m_maxs.y, center.z + halfCenterLineThick)); 
	AddVertsForAABB3D(verts, lineBounds, Y_COLOR);

	//x rows
	int rowCount = 0;
	for (int xRowNum = 0; xRowNum < xRows; ++xRowNum)
	{
		Rgba8 color = GRID_COLOR;
		float thickness = halfLineThick;

		rowCount++;
		if (rowCount == accentUnitScale)
		{
			color = X_COLOR;
			thickness = halfAccentLineThick;
			color.r = 150;
			rowCount = 0;
		}

		lineBounds = AABB3(Vec3(gridBounds.m_mins.x, (center.y + unitScale + (xRowNum * unitScale)) - thickness, center.z - thickness),
			Vec3(gridBounds.m_maxs.x, (center.y + unitScale + (xRowNum * unitScale)) + thickness, center.z + thickness));
		AddVertsForAABB3D(verts, lineBounds, color);

		lineBounds = AABB3(Vec3(gridBounds.m_mins.x, (center.y - unitScale - (xRowNum * unitScale)) - thickness, center.z - thickness),
			Vec3(gridBounds.m_maxs.x, (center.y - unitScale - (xRowNum * unitScale)) + thickness, center.z + thickness));
		AddVertsForAABB3D(verts, lineBounds, color);
	}

	//yRows
	halfLineThick *= 0.9f;
	halfAccentLineThick *= 0.9f;
	rowCount = 0;
	for (int yRowNum = 0; yRowNum < yRows; ++yRowNum)
	{
		Rgba8 color = GRID_COLOR;
		float thickness = halfLineThick;

		rowCount++;
		if (rowCount == accentUnitScale)
		{
			color = Y_COLOR;
			thickness = halfAccentLineThick;
			color.g = 150;
			rowCount = 0;
		}

		lineBounds = AABB3(Vec3((center.x + unitScale + (yRowNum * unitScale)) - thickness, gridBounds.m_mins.y, center.z - thickness),
			Vec3((center.x + unitScale + (yRowNum * unitScale)) + thickness, gridBounds.m_maxs.y, center.z + thickness));
		AddVertsForAABB3D(verts, lineBounds, color);

		lineBounds = AABB3(Vec3((center.x - unitScale -(yRowNum * unitScale)) - thickness, gridBounds.m_mins.y, center.z - thickness),
			Vec3((center.x - unitScale - (yRowNum * unitScale)) + thickness, gridBounds.m_maxs.y, center.z + thickness));
		AddVertsForAABB3D(verts, lineBounds, color);
	}
}

void AddVertsForGridPlane3D(Verts& verts, Vec3 const& planeNormal, float planeDistanceFromOriginAlongNormal, IntVec2 const& visibleDimensions, int unitScale, int accentUnitScale, float lineThickness)
{
	const int START_INDEX = (int)verts.size();

	AddVertsForGridPlane3D(verts, visibleDimensions, Vec3::ZERO, unitScale, accentUnitScale, lineThickness);
	Vec3 center = planeNormal * planeDistanceFromOriginAlongNormal;

	Mat44 transform = GetLookAtTransform(center, center + (planeNormal * 10.f));
	transform.AppendYRotation(-90.f);

	TransformVertexArray3D(verts, transform, IntRange(START_INDEX, (int)verts.size() - 1));
}

void AddVertsForSingleColoredXYGridPlane3D(Verts& verts, IntVec2 const& xyUnitDimensions, Vec3 const& center, int unitScale, float lineThickness, Rgba8 const& color)
{
	int xRows = (int)floorf((float)xyUnitDimensions.x * 0.5f); // rows in either direction starting at center
	int yRows = (int)floorf((float)xyUnitDimensions.y * 0.5f);

	Vec2 offsetXY = Vec2(center.x + (yRows * unitScale), center.y + (xRows * unitScale));
	AABB2 gridBounds(center.x - offsetXY.x, center.y - offsetXY.y, center.x + offsetXY.x, center.y + offsetXY.y);

	float halfLineThickness = lineThickness * 0.5f;

	//x
	AABB3 lineBounds(Vec3(gridBounds.m_mins.x, center.y - halfLineThickness, center.z - halfLineThickness),
		Vec3(gridBounds.m_maxs.x, center.y + halfLineThickness, center.z + halfLineThickness));
	AddVertsForAABB3D(verts, lineBounds, color);

	//y
	lineBounds = AABB3(Vec3(center.x - halfLineThickness, gridBounds.m_mins.y, center.z - halfLineThickness),
		Vec3(center.x + halfLineThickness, gridBounds.m_maxs.y, center.z + halfLineThickness));
	AddVertsForAABB3D(verts, lineBounds, color);

	//x rows
	for (int xRowNum = 0; xRowNum < xRows; ++xRowNum)
	{
		lineBounds = AABB3(Vec3(gridBounds.m_mins.x, (center.y + unitScale + (xRowNum * unitScale)) - halfLineThickness, center.z - halfLineThickness),
			Vec3(gridBounds.m_maxs.x, (center.y + unitScale + (xRowNum * unitScale)) + halfLineThickness, center.z + halfLineThickness));
		AddVertsForAABB3D(verts, lineBounds, color);

		lineBounds = AABB3(Vec3(gridBounds.m_mins.x, (center.y - unitScale - (xRowNum * unitScale)) - halfLineThickness, center.z - halfLineThickness),
			Vec3(gridBounds.m_maxs.x, (center.y - unitScale - (xRowNum * unitScale)) + halfLineThickness, center.z + halfLineThickness));
		AddVertsForAABB3D(verts, lineBounds, color);
	}

	//yRows
	for (int yRowNum = 0; yRowNum < yRows; ++yRowNum)
	{
		lineBounds = AABB3(Vec3((center.x + unitScale + (yRowNum * unitScale)) - halfLineThickness, gridBounds.m_mins.y, center.z - halfLineThickness),
			Vec3((center.x + unitScale + (yRowNum * unitScale)) + halfLineThickness, gridBounds.m_maxs.y, center.z + halfLineThickness));
		AddVertsForAABB3D(verts, lineBounds, color);

		lineBounds = AABB3(Vec3((center.x - unitScale - (yRowNum * unitScale)) - halfLineThickness, gridBounds.m_mins.y, center.z - halfLineThickness),
			Vec3((center.x - unitScale - (yRowNum * unitScale)) + halfLineThickness, gridBounds.m_maxs.y, center.z + halfLineThickness));
		AddVertsForAABB3D(verts, lineBounds, color);
	}
}

void AddVertsForArrow3D(Verts& verts, Vec3 const& start, Vec3 const& direction, float length, float thickness, int numSlices, Rgba8 const& tint, AABB2 const& uvs)
{
	const int START_INDEX = (int)verts.size();
	const float SLICE_STEP_DEGREES = 360.f / numSlices;
	const float ARROW_LENGTH = GetClamped(.25f * length, 0.f, .25f);
	float arrowLengthFraction = GetClampedFractionWithinRange(length, 0.f, 10.f);
	const float ARROW_THICKNESS = thickness * Lerp(1.5f, 5.f, arrowLengthFraction);

	Rgba8 darkenedTint = tint;
	darkenedTint.r = DenormalizeByte(NormalizeByte(tint.r) - 0.1f);
	darkenedTint.g = DenormalizeByte(NormalizeByte(tint.g) - 0.1f);
	darkenedTint.b = DenormalizeByte(NormalizeByte(tint.b) - 0.1f);

	//divide up uvs by section of arrow
	const float UV_SLICE_STEP = (uvs.m_maxs.x - uvs.m_mins.x) / numSlices;
	const float UV_Y_DIFF = uvs.m_maxs.y - uvs.m_mins.y;
	const float UV_LENGTH = (thickness * 0.5f) + (ARROW_THICKNESS * 0.5f) + length;

	float fraction = GetFractionWithinRange(thickness * 0.5f, 0.f, UV_LENGTH);
	const FloatRange Y_UV_RANGE_BOTTOM = FloatRange(uvs.m_mins.y, uvs.m_mins.y + (fraction * UV_Y_DIFF));
	fraction += GetFractionWithinRange(length - ARROW_LENGTH, 0.f, UV_LENGTH);
	const FloatRange Y_UV_RANGE_SHAFT = FloatRange(Y_UV_RANGE_BOTTOM.m_max, uvs.m_mins.y + (fraction * UV_Y_DIFF));
	fraction += GetFractionWithinRange(ARROW_THICKNESS * 0.5f, 0.f, UV_LENGTH);
	const FloatRange Y_UV_RANGE_ARROW_UNDERSIDE = FloatRange(Y_UV_RANGE_SHAFT.m_max, uvs.m_mins.y + (fraction * UV_Y_DIFF));
	const FloatRange Y_UV_RANGE_ARROW_HEAD = FloatRange(Y_UV_RANGE_ARROW_UNDERSIDE.m_max, uvs.m_maxs.y);

	//Create temporary verts in x direction to be transformed later
	Vec3 endOfCylinder = Vec3::FORWARD * (length - ARROW_LENGTH);
	Vec3 end = Vec3::FORWARD * length;
	const float LATITUDE = -90.f;
 	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		float longitude = sliceNum * SLICE_STEP_DEGREES;
		FloatRange sliceXuvs(uvs.m_mins.x + (UV_SLICE_STEP * sliceNum), uvs.m_mins.x + (UV_SLICE_STEP * (sliceNum + 1)));

		//circle underneath cylinder
		Vec3 leftOffset = Vec3::MakeFromPolarDegrees(LATITUDE, longitude, thickness);
		Vec3 rightOffset = Vec3::MakeFromPolarDegrees(LATITUDE, longitude + SLICE_STEP_DEGREES, thickness);
		Vec3 topLeft = leftOffset;
		Vec3 topRight = rightOffset;

		verts.push_back(Vertex_PCU(Vec3::ZERO, darkenedTint, Vec2(sliceXuvs.m_min + (0.5f * (sliceXuvs.m_max - sliceXuvs.m_min)), uvs.m_mins.y)));
		verts.push_back(Vertex_PCU(topRight, darkenedTint, Vec2(sliceXuvs.m_max, Y_UV_RANGE_BOTTOM.m_max)));
		verts.push_back(Vertex_PCU(topLeft, darkenedTint, Vec2(sliceXuvs.m_min, Y_UV_RANGE_BOTTOM.m_max)));

		//length of cylinder before arrow head
		Vec3 bottomLeft = topLeft;
		Vec3 bottomRight = topRight;
		topRight = endOfCylinder + rightOffset;
		topLeft = endOfCylinder + leftOffset;
		AABB2 cylinderLengthUVS = AABB2(Vec2(sliceXuvs.m_min, Y_UV_RANGE_SHAFT.m_min), Vec2(sliceXuvs.m_max, Y_UV_RANGE_SHAFT.m_max));

		AddVertsForQuad3D(verts, bottomLeft, bottomRight, topRight, topLeft, tint, cylinderLengthUVS);

		//circle under arrow head
		bottomLeft = topLeft;
		bottomRight = topRight;

		leftOffset = Vec3::MakeFromPolarDegrees(LATITUDE, longitude, ARROW_THICKNESS);
		rightOffset = Vec3::MakeFromPolarDegrees(LATITUDE, longitude + SLICE_STEP_DEGREES, ARROW_THICKNESS);
		topRight = endOfCylinder + rightOffset;
		topLeft = endOfCylinder + leftOffset;

		AddVertsForQuad3D(verts, bottomLeft, bottomRight, topRight, topLeft, darkenedTint, AABB2(sliceXuvs.m_min ,Y_UV_RANGE_ARROW_UNDERSIDE.m_min,
			sliceXuvs.m_max, Y_UV_RANGE_ARROW_UNDERSIDE.m_max));

		//point of arrow head
		bottomLeft = topLeft;
		bottomRight = topRight;
		verts.push_back(Vertex_PCU(bottomLeft, tint, Vec2(sliceXuvs.m_min, Y_UV_RANGE_ARROW_HEAD.m_min)));
		verts.push_back(Vertex_PCU(bottomRight, tint, Vec2(sliceXuvs.m_max, Y_UV_RANGE_ARROW_HEAD.m_min)));
		verts.push_back(Vertex_PCU(end, tint, Vec2( sliceXuvs.m_min + (0.5f * (sliceXuvs.m_max - sliceXuvs.m_min)), uvs.m_maxs.y)));
	}

	Mat44 transform = GetLookAtTransform(start, start + (direction * length));

	//Transform all newly added verts to IJK space
	TransformVertexArray3D(verts, transform, IntRange(START_INDEX, (int)(verts.size() - 1)));
}

void AddVertsForUVSphereZ3D(Verts& verts, Vec3 const& center, float radius, int numSlices, int numStacks, Rgba8 const& tint, AABB2 const& uvs)
{
	float yawStep = 360.f / (float)numSlices;
	float pitchStep = 180.f / (float)numStacks;
	Vec2 uvStep((uvs.m_maxs.x - uvs.m_mins.x) / numSlices, (uvs.m_maxs.y - uvs.m_mins.y) / numStacks);

	for (int stackNum = 0; stackNum < numStacks; ++stackNum)
	{
		float longitude = 90.f - (stackNum * pitchStep);
		for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
		{
			float latitude = yawStep * sliceNum;
			AABB2 quadUvs = AABB2(Vec2(uvs.m_mins.x + (uvStep.x * sliceNum), uvs.m_mins.y + (uvStep.y * stackNum)),
				Vec2(uvs.m_mins.x + (uvStep.x * (sliceNum + 1.f)),uvs.m_mins.y + (uvStep.y * (stackNum + 1.f))));

			Vec3 bottomLeft = center + Vec3::MakeFromPolarDegrees(latitude, longitude, radius);
			Vec3 bottomRight = center + Vec3::MakeFromPolarDegrees(latitude + yawStep, longitude, radius);
			Vec3 topRight = center + Vec3::MakeFromPolarDegrees(latitude + yawStep, longitude - pitchStep, radius);
			Vec3 topLeft = center + Vec3::MakeFromPolarDegrees(latitude, longitude - pitchStep, radius);

			//bottom cap
			if (stackNum == 0)
			{
				float uvXMiddle = quadUvs.m_mins.x + (uvStep.x * 0.5f);
				verts.push_back(Vertex_PCU(bottomLeft, tint, Vec2(uvXMiddle, 0.f)));
				verts.push_back(Vertex_PCU(topRight, tint, Vec2(quadUvs.m_maxs.x, quadUvs.m_maxs.y)));
				verts.push_back(Vertex_PCU(topLeft, tint, Vec2(quadUvs.m_mins.x, quadUvs.m_maxs.y)));
			}
			
			//top cap
			else if (stackNum == numStacks - 1)
			{
				float uvXMiddle = quadUvs.m_mins.x + (uvStep.x * 0.5f);
				verts.push_back(Vertex_PCU(bottomLeft, tint, quadUvs.m_mins));
				verts.push_back(Vertex_PCU(bottomRight, tint, Vec2(quadUvs.m_maxs.x, quadUvs.m_mins.y)));
				verts.push_back(Vertex_PCU(topRight, tint, Vec2(uvXMiddle, 1.f)));
			}

			else
			{
				verts.push_back(Vertex_PCU(bottomLeft, tint, quadUvs.m_mins));
				verts.push_back(Vertex_PCU(bottomRight, tint, Vec2(quadUvs.m_maxs.x, quadUvs.m_mins.y)));
				verts.push_back(Vertex_PCU(topRight, tint, quadUvs.m_maxs));

				verts.push_back(Vertex_PCU(bottomLeft, tint, quadUvs.m_mins));
				verts.push_back(Vertex_PCU(topRight, tint, Vec2(quadUvs.m_maxs.x, quadUvs.m_maxs.y)));
				verts.push_back(Vertex_PCU(topLeft, tint, Vec2(quadUvs.m_mins.x, quadUvs.m_maxs.y)));
			}

		}
	}
}


void AddVertsForIndexedZSphere3D(VertTBNs& verts, std::vector<unsigned int>& indexes, Vec3 const& center, float radius, Rgba8 const& tint, AABB2 const& uvs, int numSlices, int numStacks)
{
	const int START_INDEX = (int)verts.size();

	float yawStep = 360.f / (float)numSlices;
	float pitchStep = 180.f / (float)numStacks;
	Vec2 uvStep((uvs.m_maxs.x - uvs.m_mins.x) / numSlices, (uvs.m_maxs.y - uvs.m_mins.y) / numStacks);

	Vec3 tangent = Vec3::ZERO;
	Vec3 biTangent = Vec3::ZERO;
	Vec3 normal = Vec3::DOWN;

	unsigned int bottomLeftIndex = START_INDEX;
	unsigned int bottomRightIndex = START_INDEX;
	unsigned int topLeftIndex = START_INDEX;
	unsigned int topRightIndex = START_INDEX;
	std::vector<unsigned int> bottomIndexes;
	bottomIndexes.reserve(numSlices);

	Vec3 bottomRight;
	Vec3 topRight;
	Vec3 topLeft;

	topLeftIndex = (unsigned int)verts.size();
	bottomIndexes.push_back(topLeftIndex);
	topLeft = center + Vec3::MakeFromPolarDegrees(0.f, 90.f - pitchStep, radius);
	normal = (topLeft - center).GetNormalized();
	tangent = CrossProduct3D(Vec3::UP, normal).GetNormalized();
	biTangent = CrossProduct3D(normal, tangent).GetNormalized();
	verts.push_back(Vertex_PCUTBN(topLeft, tint, tangent, biTangent, normal, Vec2(0.f, uvs.m_mins.y + uvStep.y)));

	//bottom cap
	Vec3 bottomPos = center + (Vec3::DOWN * radius);
	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		float latitude = yawStep * sliceNum;
		topRight = center + Vec3::MakeFromPolarDegrees(latitude + yawStep, 90.f - pitchStep, radius);

		AABB2 quadUvs = AABB2(Vec2(uvs.m_mins.x + (uvStep.x * sliceNum), uvs.m_mins.y),
			Vec2(uvs.m_mins.x + (uvStep.x * (sliceNum + 1.f)), uvs.m_mins.y + uvStep.y));

		indexes.push_back((unsigned int)verts.size());
		float uvXMiddle = quadUvs.m_mins.x + (uvStep.x * 0.5f);
		normal = Vec3::DOWN;
		biTangent = Vec3(Vec2::MakeFromPolarDegrees(latitude + yawStep), 0.f).GetNormalized();
		tangent = CrossProduct3D(biTangent, normal);
		verts.push_back(Vertex_PCUTBN(bottomPos, tint, tangent, biTangent, normal, Vec2(uvXMiddle, 0.f)));

		topRightIndex = (unsigned int)verts.size();
		bottomIndexes.push_back(topRightIndex);
		indexes.push_back(topRightIndex);
		normal = (topRight - center).GetNormalized();
		tangent = CrossProduct3D(Vec3::UP, normal).GetNormalized();
		biTangent = CrossProduct3D(normal, tangent).GetNormalized();
		verts.push_back(Vertex_PCUTBN(topRight, tint, tangent, biTangent, normal, Vec2(quadUvs.m_maxs.x, quadUvs.m_maxs.y)));

		indexes.push_back(topLeftIndex);
		topLeftIndex = topRightIndex;
		topLeft = topRight;
	}

	//Middle Stacks + Top Cap
	for (int stackNum = 1; stackNum < numStacks; ++stackNum)
	{
		float longitude = 90.f - (stackNum * pitchStep);

		topLeftIndex = (unsigned int)verts.size();
		bottomIndexes.push_back(topLeftIndex);
		topLeft = center + Vec3::MakeFromPolarDegrees(0.f, longitude - pitchStep, radius);
		normal = (topLeft - center).GetNormalized();
		tangent = CrossProduct3D(Vec3::UP, normal).GetNormalized();
		biTangent = CrossProduct3D(normal, tangent).GetNormalized();
		verts.push_back(Vertex_PCUTBN(topLeft, tint, tangent, biTangent, normal, Vec2(uvs.m_mins.x, uvs.m_mins.y + (uvStep.y * (stackNum + 1.f)))));
		

		for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
		{
			float latitude = yawStep * sliceNum;
			AABB2 quadUvs = AABB2(Vec2(uvs.m_mins.x + (uvStep.x * sliceNum), uvs.m_mins.y + (uvStep.y * stackNum)),
				Vec2(uvs.m_mins.x + (uvStep.x * (sliceNum + 1.f)), uvs.m_mins.y + (uvStep.y * (stackNum + 1.f))));

			topRight = center + Vec3::MakeFromPolarDegrees(latitude + yawStep, longitude - pitchStep, radius);
			
			bottomLeftIndex = bottomIndexes[((stackNum - 1) * (numSlices + 1)) + (sliceNum)];
			bottomRightIndex = bottomIndexes[((stackNum - 1) * (numSlices + 1)) + (sliceNum + 1)];

			//top cap
			if (stackNum == numStacks - 1)
			{
				float uvXMiddle = quadUvs.m_mins.x + (uvStep.x * 0.5f);
				indexes.push_back(bottomLeftIndex);
				indexes.push_back(bottomRightIndex);

				normal = Vec3::UP;
				biTangent = -Vec3(Vec2::MakeFromPolarDegrees(latitude + yawStep), 0.f).GetNormalized();
				tangent = CrossProduct3D(biTangent, normal).GetNormalized();
				topRightIndex = (unsigned int)verts.size();
				indexes.push_back(topRightIndex);
				bottomIndexes.push_back(topRightIndex);
				normal = (topRight - center).GetNormalized();
				verts.push_back(Vertex_PCUTBN(topRight, tint, tangent, biTangent, normal, Vec2(uvXMiddle, 1.f)));
			}

			else
			{
				indexes.push_back(bottomLeftIndex);
				indexes.push_back(bottomRightIndex);

				topRightIndex = (unsigned int)verts.size();
				indexes.push_back(topRightIndex);
				bottomIndexes.push_back(topRightIndex);
				normal = (topRight - center).GetNormalized();
				tangent = CrossProduct3D(Vec3::UP,normal).GetNormalized();
				biTangent = CrossProduct3D(normal,tangent).GetNormalized();
				verts.push_back(Vertex_PCUTBN(topRight, tint, tangent, biTangent, normal, quadUvs.m_maxs));

				indexes.push_back(bottomLeftIndex);
				indexes.push_back(topRightIndex);
				indexes.push_back(topLeftIndex);
			}

			topLeftIndex = topRightIndex;
			topLeft = topRight;
		}
	}
}


void AddVertsForCylinder3D(Verts& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& tint, AABB2 const& uvs, int numSlices)
{
	const int START_INDEX = (int)verts.size();
	const Vec3 DIRECTION = Vec3(end - start);
	float length = DIRECTION.GetLength();

	const float UV_LENGTH = radius + length;
	const float UV_Y_DIFF = uvs.m_maxs.y - uvs.m_mins.y;
	const float HALF_THICKNESS = radius * 0.5f;
	float bottomUVFraction = GetFractionWithinRange(HALF_THICKNESS, 0.f, UV_LENGTH);
	float shaftUVFraction = GetFractionWithinRange(length + HALF_THICKNESS, 0.f, UV_LENGTH);
	const FloatRange Y_UV_RANGE_SHAFT = FloatRange(uvs.m_mins.y + (bottomUVFraction * UV_Y_DIFF), uvs.m_mins.y + (shaftUVFraction * UV_Y_DIFF));
	const float UV_SLICE_STEP = (uvs.m_maxs.x - uvs.m_mins.x) / numSlices;

	const float SLICE_STEP_DEGREES = 360.f / numSlices;
	const Vec3 X_END = Vec3::FORWARD * (length);
	Vec3 leftOffset = Vec3(0.f, -radius, 0.f);

	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		float longitudeAfterStep = (sliceNum + 1) * SLICE_STEP_DEGREES;
		FloatRange sliceXuvs(uvs.m_mins.x + (UV_SLICE_STEP * sliceNum), uvs.m_mins.x + (UV_SLICE_STEP * (sliceNum + 1)));

		Vec3 rightOffset = Vec3::MakeFromPolarDegrees(-90.f, longitudeAfterStep, radius);
		Vec3 topLeft = leftOffset;
		Vec3 topRight = rightOffset;

		verts.push_back(Vertex_PCU(Vec3::ZERO, tint, Vec2(sliceXuvs.m_min + (0.5f * (sliceXuvs.m_max - sliceXuvs.m_min)), uvs.m_mins.y)));
		verts.push_back(Vertex_PCU(topRight, tint, Vec2(sliceXuvs.m_max, Y_UV_RANGE_SHAFT.m_min)));
		verts.push_back(Vertex_PCU(topLeft, tint, Vec2(sliceXuvs.m_min, Y_UV_RANGE_SHAFT.m_min)));

		Vec3 bottomLeft = topLeft;
		Vec3 bottomRight = topRight;
		topRight = X_END + rightOffset;
		topLeft = X_END + leftOffset;

		AABB2 quadUVS = AABB2(sliceXuvs.m_min, Y_UV_RANGE_SHAFT.m_min, sliceXuvs.m_max, Y_UV_RANGE_SHAFT.m_max);
		AddVertsForQuad3D(verts, bottomLeft, bottomRight, topRight, topLeft, tint, quadUVS);

		verts.push_back(Vertex_PCU(topLeft, tint, Vec2(sliceXuvs.m_min, Y_UV_RANGE_SHAFT.m_max)));
		verts.push_back(Vertex_PCU(topRight, tint, Vec2(sliceXuvs.m_max, Y_UV_RANGE_SHAFT.m_max)));
		verts.push_back(Vertex_PCU(X_END, tint, Vec2(sliceXuvs.m_min + (0.5f * (sliceXuvs.m_max - sliceXuvs.m_min)), uvs.m_maxs.y)));

		leftOffset = rightOffset;
	}

	Mat44 transform = GetLookAtTransform(start, end);
	
	//Transform all newly added verts to IJK space
	TransformVertexArray3D(verts, transform, IntRange(START_INDEX, (int)(verts.size() - 1)));
}

void AddVertsForZCylinder3D(Verts& verts, Vec3 const& bottom, float length, float radius, int numSlices, Rgba8 const& tint, AABB2 const& uvs)
{
	const float SLICE_STEP_DEGREES = 360.f / numSlices;
	const float UV_SLICE_STEP = (uvs.m_maxs.x - uvs.m_mins.x) / numSlices;
	float uvXRadius = 0.5f * (uvs.m_maxs.x - uvs.m_mins.x);
	float uvYRadius = 0.5f * (uvs.m_maxs.y - uvs.m_mins.y);
	Vec2 uvCenter = Vec2(uvs.m_mins.x + (0.5f * (uvs.m_maxs.x - uvs.m_mins.x)) , uvs.m_mins.y + (0.5f * (uvs.m_maxs.y - uvs.m_mins.y)));

	Vec3 top = bottom + (Vec3::UP * length);
	Vec3 leftOffset = Vec3(radius, 0.f, 0.f);
	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		float latitude = sliceNum * SLICE_STEP_DEGREES;
		float latitudeAfterStep = (sliceNum + 1) * SLICE_STEP_DEGREES;
		FloatRange sliceXuvs(uvs.m_mins.x + (UV_SLICE_STEP * sliceNum), uvs.m_mins.x + (UV_SLICE_STEP * (sliceNum + 1)));

		Vec3 rightOffset = Vec3::MakeFromPolarDegrees(latitudeAfterStep, 0.f, radius);
		Vec3 topLeft = bottom + leftOffset;
		Vec3 topRight = bottom + rightOffset;

		verts.push_back(Vertex_PCU(bottom, tint, uvCenter));
		verts.push_back(Vertex_PCU(topRight, tint, uvCenter + Vec2(uvXRadius * CosDegrees(latitudeAfterStep), -uvYRadius * SinDegrees(latitudeAfterStep))));
		verts.push_back(Vertex_PCU(topLeft, tint, uvCenter + Vec2(uvXRadius * CosDegrees(latitude), -uvYRadius * SinDegrees(latitude))));

		Vec3 bottomLeft = topLeft;
		Vec3 bottomRight = topRight;
		topRight = top + rightOffset;
		topLeft = top + leftOffset;

		AABB2 quadUVS = AABB2(sliceXuvs.m_min, uvs.m_mins.y, sliceXuvs.m_max, uvs.m_maxs.y);
		AddVertsForQuad3D(verts, bottomLeft, bottomRight, topRight, topLeft, tint, quadUVS);

		verts.push_back(Vertex_PCU(topLeft, tint, uvCenter + Vec2(uvXRadius * CosDegrees(latitude), uvYRadius * SinDegrees(latitude))));
		verts.push_back(Vertex_PCU(topRight, tint, uvCenter + Vec2(uvXRadius * CosDegrees(latitudeAfterStep), uvYRadius * SinDegrees(latitudeAfterStep))));
		verts.push_back(Vertex_PCU(top, tint, uvCenter));

		leftOffset = rightOffset;
	}
}

void AddVertsForIndexedZCylinder3D(VertTBNs& verts, std::vector<unsigned int>& indexes, Vec3 const& bottom, float length, float radius, Rgba8 const& tint, AABB2 const& uvs, int numSlices)
{
	const int START_INDEX = (int)verts.size();

	const float SLICE_STEP_DEGREES = 360.f / numSlices;
	const float UV_SLICE_STEP = (uvs.m_maxs.x - uvs.m_mins.x) / numSlices;
	float uvXRadius = 0.5f * (uvs.m_maxs.x - uvs.m_mins.x);
	float uvYRadius = 0.5f * (uvs.m_maxs.y - uvs.m_mins.y);
	Vec2 uvCenter = Vec2(uvs.m_mins.x + uvXRadius, uvs.m_mins.y + uvYRadius);

	Vec3 top = bottom + (Vec3::UP * length);
	Vec3 leftVertEdgeOffset = Vec3(radius, 0.f, 0.f);

	Vec3 tangent = Vec3::FORWARD;
	Vec3 biTangent = Vec3::RIGHT;
	Vec3 normal = Vec3::DOWN;

	//initialize bottom and top verts and one edge of a slice going up the side of the cylinder
	//----------------------------------------------------------------------------------------
	//Bottom vert
	verts.push_back(Vertex_PCUTBN(bottom, tint, tangent, biTangent, normal, uvCenter));

	//Top vert
	tangent = Vec3::FORWARD;
	biTangent = Vec3::LEFT;
	normal = Vec3::UP;
	verts.push_back(Vertex_PCUTBN(top, tint,tangent, biTangent, normal, uvCenter));

	unsigned int bottomIndex = START_INDEX;
	unsigned int topIndex = START_INDEX + 1;

	unsigned int bottomLeftIndexDown = START_INDEX;
	unsigned int bottomRightIndexDown = START_INDEX;
	unsigned int topLeftIndexUp = START_INDEX;
	unsigned int topRightIndexUp = START_INDEX;

	unsigned int bottomLeftIndexSide = START_INDEX;
	unsigned int bottomRightIndexSide = START_INDEX;
	unsigned int topLeftIndexSide = START_INDEX;
	unsigned int topRightIndexSide = START_INDEX;

	//Start Edge Verts
	tangent = Vec3::FORWARD;
	biTangent = Vec3::RIGHT;
	normal = Vec3::DOWN;
	Vec3 edgePos = bottom + leftVertEdgeOffset;
	bottomLeftIndexDown = (unsigned int)verts.size();
	verts.push_back(Vertex_PCUTBN(edgePos, tint, tangent, biTangent, normal, uvCenter + Vec2(uvXRadius, 0.f)));

	//normal = (edgePos - bottom).GetNormalized();
	normal = Vec3::FORWARD;
	tangent = Vec3::LEFT;
	biTangent = Vec3::UP;
	bottomLeftIndexSide = (unsigned int)verts.size();
	verts.push_back(Vertex_PCUTBN(edgePos, tint, tangent, biTangent, normal, uvs.m_mins));

	edgePos = top + leftVertEdgeOffset;
	normal = (edgePos - top).GetNormalized();
	topLeftIndexSide = (unsigned int)verts.size();
	verts.push_back(Vertex_PCUTBN(edgePos, tint, tangent, biTangent, normal, Vec2(uvs.m_mins.x, uvs.m_maxs.y)));

	tangent = Vec3::FORWARD;
	biTangent = Vec3::LEFT;
	normal = Vec3::UP;
	topLeftIndexUp = (unsigned int)verts.size();
	verts.push_back(Vertex_PCUTBN(edgePos, tint, tangent, biTangent, normal, uvCenter + Vec2(uvXRadius, 0.f)));

	//Fill in the rest of the slices
	//----------------------------------------------------------------------------------------
	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		float latitudeAfterStep = (sliceNum + 1) * SLICE_STEP_DEGREES;
		FloatRange sliceXuvs(uvs.m_mins.x + (UV_SLICE_STEP * sliceNum), uvs.m_mins.x + (UV_SLICE_STEP * (sliceNum + 1)));

		Vec3 rightVertEdgeOffset = Vec3::MakeFromPolarDegrees(latitudeAfterStep, 0.f, radius);
		Vec3 topLeft = bottom + leftVertEdgeOffset;
		Vec3 topRight = bottom + rightVertEdgeOffset;


		//Bottom Triangle
		tangent = Vec3::FORWARD;
		biTangent = Vec3::RIGHT;
		normal = Vec3::DOWN;

		indexes.push_back(bottomIndex);

		bottomRightIndexDown = (unsigned int)verts.size();
		verts.push_back(Vertex_PCUTBN(topRight, tint, tangent, biTangent, normal, uvCenter + Vec2(uvXRadius * CosDegrees(latitudeAfterStep), -uvYRadius * SinDegrees(latitudeAfterStep))));
		indexes.push_back(bottomRightIndexDown);

		indexes.push_back(bottomLeftIndexDown);

		Vec3 bottomLeft = topLeft;
		Vec3 bottomRight = topRight;
		topRight = top + rightVertEdgeOffset;
		topLeft = top + leftVertEdgeOffset;

		//Side quad
		tangent = (bottomRight - bottomLeft).GetNormalized();
		biTangent = Vec3::UP;
		normal = (bottomRight - bottom).GetNormalized();
		indexes.push_back(bottomLeftIndexSide);

		bottomRightIndexSide = (unsigned int)verts.size();
		verts.push_back(Vertex_PCUTBN(bottomRight, tint, tangent, biTangent, normal, Vec2(sliceXuvs.m_max, uvs.m_mins.y)));
		indexes.push_back(bottomRightIndexSide);

		topRightIndexSide = (unsigned int)verts.size();
		verts.push_back(Vertex_PCUTBN(topRight, tint, tangent, biTangent, normal, Vec2(sliceXuvs.m_max, uvs.m_maxs.y)));
		indexes.push_back(topRightIndexSide);

		indexes.push_back(bottomLeftIndexSide);
		indexes.push_back(topRightIndexSide);
		indexes.push_back(topLeftIndexSide);

		//Top Triangle
		tangent = Vec3::FORWARD;
		biTangent = Vec3::LEFT;
		normal = Vec3::UP;
		indexes.push_back(topLeftIndexUp);

		topRightIndexUp = (unsigned int)verts.size();
		verts.push_back(Vertex_PCUTBN(topRight, tint, tangent, biTangent, normal, uvCenter + Vec2(uvXRadius * CosDegrees(latitudeAfterStep), uvYRadius * SinDegrees(latitudeAfterStep))));
		indexes.push_back(topRightIndexUp);

		indexes.push_back(topIndex);

		//Save off edge of slice indexes for use in next slice
		leftVertEdgeOffset = rightVertEdgeOffset;
		bottomLeftIndexDown = bottomRightIndexDown;
		bottomLeftIndexSide = bottomRightIndexSide;
		topLeftIndexUp = topRightIndexUp;
		topLeftIndexSide = topRightIndexSide;
	}
	
}

void AddVertsForCone3D(Verts& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& tint, AABB2 const& uvs, int numSlices)
{
	const int START_INDEX = (int)verts.size();
	const Vec3 DIRECTION = Vec3(end - start);
	float length = DIRECTION.GetLength();

	const float UV_LENGTH = radius + length;
	const float UV_Y_DIFF = uvs.m_maxs.y - uvs.m_mins.y;
	float fraction = GetFractionWithinRange(radius, 0.f, UV_LENGTH);
	const float Y_UV_AT_CIRCLE_EDGE = uvs.m_mins.y + (fraction * UV_Y_DIFF);
	const float UV_SLICE_STEP = (uvs.m_maxs.x - uvs.m_mins.x) / numSlices;

	const float YAW_STEP = 360.f / numSlices;
	const Vec3 X_END = Vec3::FORWARD * (length);
	Vec3 leftOffset = Vec3(0.f, -radius, 0.f);

	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		float lonitudeAfterStep = (sliceNum + 1) * YAW_STEP;
		FloatRange sliceXuvs(uvs.m_mins.x + (UV_SLICE_STEP * sliceNum), uvs.m_mins.x + (UV_SLICE_STEP * (sliceNum + 1)));

		Vec3 rightOffset = Vec3::MakeFromPolarDegrees(-90.f,lonitudeAfterStep, radius);
		Vec3 topRight = rightOffset;
		Vec3 topLeft = leftOffset;

		verts.push_back(Vertex_PCU(Vec3::ZERO, tint, Vec2(sliceXuvs.m_min + (0.5f * (sliceXuvs.m_max - sliceXuvs.m_min)), uvs.m_mins.y)));
		verts.push_back(Vertex_PCU(topRight, tint, Vec2(sliceXuvs.m_max, Y_UV_AT_CIRCLE_EDGE)));
		verts.push_back(Vertex_PCU(topLeft, tint, Vec2(sliceXuvs.m_min, Y_UV_AT_CIRCLE_EDGE)));

		verts.push_back(Vertex_PCU(topLeft, tint, Vec2(sliceXuvs.m_min, Y_UV_AT_CIRCLE_EDGE)));
		verts.push_back(Vertex_PCU(topRight, tint, Vec2(sliceXuvs.m_max, Y_UV_AT_CIRCLE_EDGE)));
		verts.push_back(Vertex_PCU(X_END, tint, Vec2(sliceXuvs.m_min + (0.5f * (sliceXuvs.m_max - sliceXuvs.m_min)), uvs.m_maxs.y)));

		leftOffset = rightOffset;
	}

	Mat44 transform = GetLookAtTransform(start, end);

	//Transform all newly added verts to IJK space
	TransformVertexArray3D(verts, transform, IntRange(START_INDEX, (int)(verts.size() - 1)));
}

void AddVertsForIndexedCone3D(VertTBNs& verts, std::vector<unsigned int>& indexes, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& tint, AABB2 const& uvs, int numSlices)
{
	const int START_INDEX = (int)verts.size();
	const Vec3 DIRECTION = Vec3(end - start);
	float length = DIRECTION.GetLength();

	const float UV_SLICE_STEP = (uvs.m_maxs.x - uvs.m_mins.x) / numSlices;
	float uvXRadius = 0.5f * (uvs.m_maxs.x - uvs.m_mins.x);
	float uvYRadius = 0.5f * (uvs.m_maxs.y - uvs.m_mins.y);
	Vec2 uvCenter = Vec2(uvs.m_mins.x + uvXRadius, uvs.m_mins.y + uvYRadius);

	const float YAW_STEP = 360.f / numSlices;
	const Vec3 X_END = Vec3::FORWARD * (length);
	Vec3 leftOffset = Vec3(0.f, -radius, 0.f);

	Vec3 normal = -Vec3::FORWARD;
	Vec3 tangent = Vec3::ZERO;
	Vec3 biTangent = Vec3::ZERO;

	unsigned int bottomIndex = START_INDEX;
	unsigned int edgeLeftIndexDown = START_INDEX;
	unsigned int edgeRightIndexDown = START_INDEX;
	unsigned int edgeLeftIndexOut = START_INDEX;
	unsigned int edgeRightIndexOut = START_INDEX;

	tangent = Vec3::RIGHT;
	biTangent = Vec3::UP;
	normal = Vec3::BACKWARD;
	verts.push_back(Vertex_PCUTBN(Vec3::ZERO, tint, tangent, biTangent, normal, uvCenter));

	Vec3 edgePos = Vec3::ZERO + leftOffset;
	edgeLeftIndexDown = (unsigned int)verts.size();
	verts.push_back(Vertex_PCUTBN(edgePos, tint, tangent, biTangent, normal, uvCenter + Vec2(uvXRadius, 0.f)));

	tangent = Vec3::DOWN;
	biTangent = Vec3::FORWARD;
	normal = Vec3::RIGHT;
	Vec3 leftEdgeNormal = (edgePos - Vec3::ZERO).GetNormalized();
	edgeLeftIndexOut = (unsigned int)verts.size();
	verts.push_back(Vertex_PCUTBN(edgePos, tint, tangent, biTangent, leftEdgeNormal, uvs.m_mins));


	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		float longitudeAfterStep = (sliceNum + 1) * YAW_STEP;
		FloatRange sliceXuvs(uvs.m_mins.x + (UV_SLICE_STEP * sliceNum), uvs.m_mins.x + (UV_SLICE_STEP * (sliceNum + 1)));

		Vec3 rightOffset = Vec3::MakeFromPolarDegrees(-90.f, longitudeAfterStep, radius);
		Vec3 topRight = rightOffset;
		Vec3 topLeft = leftOffset;

		//bottom Face
		tangent = Vec3::RIGHT;
		biTangent = Vec3::UP;
		normal = Vec3::BACKWARD;
		indexes.push_back(bottomIndex);

		edgeRightIndexDown = (unsigned int)verts.size();
		verts.push_back(Vertex_PCUTBN(topRight, tint, tangent, biTangent, normal, uvCenter + Vec2(uvXRadius * CosDegrees(longitudeAfterStep), -uvYRadius * SinDegrees(longitudeAfterStep))));
		indexes.push_back(edgeRightIndexDown);

		indexes.push_back(edgeLeftIndexDown);


		//Side and Point
		indexes.push_back(edgeLeftIndexOut);

		tangent = (topRight - topLeft).GetNormalized();
		//#TODO: check biTangents on cone
		//biTangent = (X_END - topRight).GetNormalized();
		biTangent = Vec3::FORWARD;
		edgeRightIndexOut = (unsigned int)verts.size();
		Vec3 rightEdgeNormal = (topRight).GetNormalized();
		verts.push_back(Vertex_PCUTBN(topRight, tint, tangent, biTangent, rightEdgeNormal, Vec2(sliceXuvs.m_max, uvs.m_mins.y)));
		indexes.push_back(edgeRightIndexOut);

		//#TODO cone apex normal will not work with normal maps
		//normal = Lerp(leftEdgeNormal, rightEdgeNormal, 0.5f, true);
		normal = Vec3::ZERO;
		indexes.push_back((unsigned int)verts.size());
		verts.push_back(Vertex_PCUTBN(X_END, tint, tangent, biTangent, normal, Vec2(sliceXuvs.m_max, uvs.m_maxs.y)));
		

		leftOffset = rightOffset;
		edgeLeftIndexDown = edgeRightIndexDown;
		edgeLeftIndexOut = edgeRightIndexOut;
		leftEdgeNormal = rightEdgeNormal;
	}

	Mat44 transform = GetLookAtTransform(start, end);

	//Transform all newly added verts to IJK space
	TransformVertexArray3D(verts, transform, IntRange(START_INDEX, (int)(verts.size() - 1)));
}

void AddVertsForLineSegment3D(Verts& verts, Vec3 const& start, Vec3 const& end, float thickness, Rgba8 const& tint, AABB2 const& uvs, float xNudgeFrac)
{
	const int START_INDEX = (int)verts.size();
	const float HALF_THICKNESS = 0.5f * thickness;
	const float X_NUDGE = xNudgeFrac * thickness;
	float length = (end - start).GetLength();

	Vec3 bottomLeft = Vec3(-X_NUDGE, 0.f, -HALF_THICKNESS);
	Vec3 bottomRight = Vec3(length + X_NUDGE, 0.f, -HALF_THICKNESS);
	Vec3 topRight = Vec3(length + X_NUDGE, 0.f, HALF_THICKNESS);
	Vec3 topLeft = Vec3(-X_NUDGE, 0.f, HALF_THICKNESS);
	AddVertsForQuad3D(verts, bottomLeft, bottomRight, topRight, topLeft, tint, uvs);
	AddVertsForQuad3D(verts,bottomRight, bottomLeft, topLeft, topRight, tint, uvs);

	bottomLeft = Vec3(-X_NUDGE, -HALF_THICKNESS, 0.f);
	bottomRight = Vec3(length + X_NUDGE, -HALF_THICKNESS, 0.f);
	topRight = Vec3(length + X_NUDGE, HALF_THICKNESS, 0.f);
	topLeft = Vec3(-X_NUDGE, HALF_THICKNESS, 0.f);
	AddVertsForQuad3D(verts, bottomLeft, bottomRight, topRight, topLeft, tint, uvs);
	AddVertsForQuad3D(verts, bottomRight, bottomLeft, topLeft, topRight, tint, uvs);

	Mat44 transform = GetLookAtTransform(start, end);
	transform.AppendXRotation(45.f);
	TransformVertexArray3D(verts, transform, IntRange(START_INDEX, (int)(verts.size() - 1)));
}

void AddVertsFor3DAsterisk(Verts& verts, Vec3 const& center, float radius, float lineThickness, Rgba8 const& tint, AABB2 const& uvs)
{
	Vec3 start = center + Vec3(radius, 0.f, 0.f);
	Vec3 end = center - Vec3(radius, 0.f, 0.f);
	AddVertsForLineSegment3D(verts, start, end, lineThickness, tint, uvs);
	start = center + Vec3(0.f, radius, 0.f);
	end = center - Vec3(0.f, radius, 0.f);
	AddVertsForLineSegment3D(verts, start, end, lineThickness, tint, uvs);
	start = center + Vec3(0.f, 0.f, radius);
	end = center - Vec3(0.f, 0.f, radius);
	AddVertsForLineSegment3D(verts, start, end, lineThickness, tint, uvs);

	Vec3 offset = Vec3::MakeFromPolarDegrees(45.f, 45.f, radius);
	start = center + offset;
	end = center - offset;
	AddVertsForLineSegment3D(verts, start, end, lineThickness, tint, uvs);

	offset = Vec3::MakeFromPolarDegrees(45.f, 135.f, radius);
	start = center + offset;
	end = center - offset;
	AddVertsForLineSegment3D(verts, start, end, lineThickness, tint, uvs);

	offset = Vec3::MakeFromPolarDegrees(-45.f, 45.f, radius);
	start = center + offset;
	end = center - offset;
	AddVertsForLineSegment3D(verts, start, end, lineThickness, tint, uvs);

	offset = Vec3::MakeFromPolarDegrees(-45.f, 135.f, radius);
	start = center + offset;
	end = center - offset;
	AddVertsForLineSegment3D(verts, start, end, lineThickness, tint, uvs);

}

void AddVertsForWireFrameAABB3D(Verts& verts, AABB3 const& bounds, float lineThickness, Rgba8 const& color, AABB2 const& uvs)
{
	Vec3 backRightBottom = bounds.m_mins;
	Vec3 frontRightBottom = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 frontRightTop = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 backRightTop = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 backLeftBottom = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 frontLeftBottom = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 frontLeftTop = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);
	Vec3 backLeftTop = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);

	AddVertsForLineSegment3D(verts, backRightBottom, frontRightBottom, lineThickness, color, uvs);
	AddVertsForLineSegment3D(verts, frontRightBottom, frontRightTop, lineThickness, color, uvs);
	AddVertsForLineSegment3D(verts, frontRightTop, backRightTop, lineThickness, color, uvs);
	AddVertsForLineSegment3D(verts, backRightTop, backRightBottom, lineThickness, color, uvs);

	AddVertsForLineSegment3D(verts, backLeftBottom, frontLeftBottom, lineThickness, color, uvs);
	AddVertsForLineSegment3D(verts, frontLeftBottom, frontLeftTop, lineThickness, color, uvs);
	AddVertsForLineSegment3D(verts, frontLeftTop, backLeftTop, lineThickness, color, uvs);
	AddVertsForLineSegment3D(verts, backLeftTop, backLeftBottom, lineThickness, color, uvs);

	AddVertsForLineSegment3D(verts, backLeftBottom, backRightBottom, lineThickness, color, uvs);
	AddVertsForLineSegment3D(verts, backLeftTop, backRightTop, lineThickness, color, uvs);
	AddVertsForLineSegment3D(verts, frontLeftBottom, frontRightBottom, lineThickness, color, uvs);
	AddVertsForLineSegment3D(verts, frontLeftTop, frontRightTop, lineThickness, color, uvs);

}

void AddVertsForWireFrameOBB3D(Verts& verts, OBB3 const& orientedBox, float lineThickness, Rgba8 const& color, AABB2 const& uvs)
{
	const int START_INDEX = (int)verts.size();
	AABB3 bounds(-orientedBox.m_halfDimensionsIJK.x, -orientedBox.m_halfDimensionsIJK.y, -orientedBox.m_halfDimensionsIJK.z,
		orientedBox.m_halfDimensionsIJK.x, orientedBox.m_halfDimensionsIJK.y, orientedBox.m_halfDimensionsIJK.z);

	AddVertsForWireFrameAABB3D(verts, bounds, lineThickness, color, uvs);

	Mat44 transform(orientedBox.m_iBasis, orientedBox.m_jBasis, orientedBox.m_kBasis, orientedBox.m_center);
	TransformVertexArray3D(verts, transform, IntRange(START_INDEX, (int)verts.size() - 1));
}

void AddVertsForWireFrameZCylinder3D(Verts& verts, float lineThickness, Vec3 const& bottom, float length, float radius, int numSlices, Rgba8 const& tint, AABB2 const& uvs)
{
	const float SLICE_STEP_DEGREES = 360.f / numSlices;

	Vec3 top = bottom + (Vec3::UP * length);
	Vec3 leftOffset = Vec3(radius, 0.f, 0.f);
	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		float latitudeAfterStep = (sliceNum + 1) * SLICE_STEP_DEGREES;

		Vec3 rightOffset = Vec3::MakeFromPolarDegrees(latitudeAfterStep, 0.f, radius);

		Vec3 bottomLeft = bottom + leftOffset;
		Vec3 bottomRight = bottom + rightOffset;
		Vec3 topRight = top + rightOffset;
		Vec3 topLeft = top + leftOffset;

		AddVertsForLineSegment3D(verts, bottomLeft, topLeft, lineThickness, tint, uvs, 0.2f);
		AddVertsForLineSegment3D(verts, bottomLeft, bottomRight, lineThickness, tint, uvs, 0.2f);
		AddVertsForLineSegment3D(verts, topLeft, topRight, lineThickness, tint, uvs, 0.2f);

		leftOffset = rightOffset;
	}

}

void AddVertsForWireFrameZSphere3D(Verts& verts, float lineThickness, Vec3 const& center, float radius, int numSlices, int numStacks, Rgba8 const& tint, AABB2 const& uvs)
{
	float yawStep = 360.f / (float)numSlices;
	float pitchStep = 180.f / (float)numStacks;

	for (int stackNum = 0; stackNum < numStacks; ++stackNum)
	{
		float longitude = 90.f - (stackNum * pitchStep);
		for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
		{
			float latitude = yawStep * sliceNum;
			

			Vec3 bottomLeft = center + Vec3::MakeFromPolarDegrees(latitude, longitude, radius);
			Vec3 bottomRight = center + Vec3::MakeFromPolarDegrees(latitude + yawStep, longitude, radius);
			Vec3 topRight = center + Vec3::MakeFromPolarDegrees(latitude + yawStep, longitude - pitchStep, radius);
			Vec3 topLeft = center + Vec3::MakeFromPolarDegrees(latitude, longitude - pitchStep, radius);

			//bottom cap
			if (stackNum == 0)
			{
				AddVertsForLineSegment3D(verts, bottomLeft, topRight, lineThickness, tint, uvs, 0.1f);
				AddVertsForLineSegment3D(verts, topRight, topLeft, lineThickness, tint, uvs, 0.1f);
				AddVertsForLineSegment3D(verts, topLeft, bottomLeft, lineThickness, tint, uvs, 0.1f);
			}

			//top cap
			else if (stackNum == numStacks - 1)
			{
				AddVertsForLineSegment3D(verts, bottomLeft, topRight, lineThickness, tint, uvs, 0.1f);
				AddVertsForLineSegment3D(verts, topRight, topLeft, lineThickness, tint, uvs, 0.1f);
				AddVertsForLineSegment3D(verts, topLeft, bottomLeft, lineThickness, tint, uvs, 0.1f);
			}

			else
			{
				AddVertsForLineSegment3D(verts, bottomLeft, bottomRight, lineThickness, tint, uvs, 0.1f);
				AddVertsForLineSegment3D(verts, topRight, topLeft, lineThickness, tint, uvs, 0.1f);
				AddVertsForLineSegment3D(verts, topLeft, bottomLeft, lineThickness, tint, uvs, 0.1f);
				AddVertsForLineSegment3D(verts, topRight, bottomRight, lineThickness, tint, uvs, 0.1f);
			}

		}
	}
}

void AddVertsForWireFrameCone3D(Verts& verts, float lineThickness, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& tint, int numSlices)
{
	const int START_INDEX = (int)verts.size();
	const Vec3 DIRECTION = Vec3(end - start);
	float length = DIRECTION.GetLength();

	const float YAW_STEP = 360.f / numSlices;
	const Vec3 X_END = Vec3::FORWARD * (length);
	Vec3 leftOffset = Vec3(0.f, -radius, 0.f);

	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		float lonitudeAfterStep = (sliceNum + 1) * YAW_STEP;

		Vec3 rightOffset = Vec3::MakeFromPolarDegrees(-90.f, lonitudeAfterStep, radius);
		Vec3 topRight = rightOffset;
		Vec3 topLeft = leftOffset;

		AddVertsForLineSegment3D(verts, X_END, topRight, lineThickness, tint);
		AddVertsForLineSegment3D(verts, topRight, topLeft, lineThickness, tint);

		leftOffset = rightOffset;
	}

	Mat44 transform = GetLookAtTransform(start, end);
	TransformVertexArray3D(verts, transform, IntRange(START_INDEX, (int)(verts.size() - 1)));
}




