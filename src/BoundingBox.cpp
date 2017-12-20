// BoundingBox.cpp

// Implements the cBoundingBox class representing an axis-aligned bounding box with floatingpoint coords

#include "Globals.h"
#include "BoundingBox.h"





bool cBoundingBox::CalcLineIntersection(Vector3d a_Line1, Vector3d a_Line2, double & a_LineCoeff, eBlockFace & a_Face) const
{
	return CalcLineIntersection(m_Min, m_Max, a_Line1, a_Line2, a_LineCoeff, a_Face);
}





bool cBoundingBox::CalcLineIntersection(Vector3d a_Min, Vector3d a_Max, Vector3d a_Line1, Vector3d a_Line2, double & a_LineCoeff, eBlockFace & a_Face)
{
	if (IsInside(a_Min, a_Max, a_Line1))
	{
		// The starting point is inside the bounding box.
		a_LineCoeff = 0;
		a_Face = BLOCK_FACE_NONE;  // No faces hit
		return true;
	}

	eBlockFace Face = BLOCK_FACE_NONE;
	double Coeff = Vector3d::NO_INTERSECTION;

	// Check each individual bbox face for intersection with the line, remember the one with the lowest coeff
	double c = a_Line1.LineCoeffToXYPlane(a_Line2, a_Min.z);
	if ((c >= 0) && (c < Coeff) && IsInside(a_Min, a_Max, a_Line1 + (a_Line2 - a_Line1) * c))
	{
		Face = (a_Line1.z > a_Line2.z) ? BLOCK_FACE_ZP : BLOCK_FACE_ZM;
		Coeff = c;
	}
	c = a_Line1.LineCoeffToXYPlane(a_Line2, a_Max.z);
	if ((c >= 0) && (c < Coeff) && IsInside(a_Min, a_Max, a_Line1 + (a_Line2 - a_Line1) * c))
	{
		Face = (a_Line1.z > a_Line2.z) ? BLOCK_FACE_ZP : BLOCK_FACE_ZM;
		Coeff = c;
	}
	c = a_Line1.LineCoeffToXZPlane(a_Line2, a_Min.y);
	if ((c >= 0) && (c < Coeff) && IsInside(a_Min, a_Max, a_Line1 + (a_Line2 - a_Line1) * c))
	{
		Face = (a_Line1.y > a_Line2.y) ? BLOCK_FACE_YP : BLOCK_FACE_YM;
		Coeff = c;
	}
	c = a_Line1.LineCoeffToXZPlane(a_Line2, a_Max.y);
	if ((c >= 0) && (c < Coeff) && IsInside(a_Min, a_Max, a_Line1 + (a_Line2 - a_Line1) * c))
	{
		Face = (a_Line1.y > a_Line2.y) ? BLOCK_FACE_YP : BLOCK_FACE_YM;
		Coeff = c;
	}
	c = a_Line1.LineCoeffToYZPlane(a_Line2, a_Min.x);
	if ((c >= 0) && (c < Coeff) && IsInside(a_Min, a_Max, a_Line1 + (a_Line2 - a_Line1) * c))
	{
		Face = (a_Line1.x > a_Line2.x) ? BLOCK_FACE_XP : BLOCK_FACE_XM;
		Coeff = c;
	}
	c = a_Line1.LineCoeffToYZPlane(a_Line2, a_Max.x);
	if ((c >= 0) && (c < Coeff) && IsInside(a_Min, a_Max, a_Line1 + (a_Line2 - a_Line1) * c))
	{
		Face = (a_Line1.x > a_Line2.x) ? BLOCK_FACE_XP : BLOCK_FACE_XM;
		Coeff = c;
	}

	if (Coeff >= Vector3d::NO_INTERSECTION)
	{
		// There has been no intersection
		return false;
	}

	a_LineCoeff = Coeff;
	a_Face = Face;
	return true;
}




