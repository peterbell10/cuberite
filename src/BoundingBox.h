
// BoundingBox.h

// Declares the cBoundingBox class representing an axis-aligned bounding box with floatingpoint coords




#pragma once

#include "Cuboid.h"





// tolua_begin

/** Represents two sets of coords, minimum and maximum for each direction.
All the coords within those limits (inclusive the edges) are considered "inside" the box.
For intersection purposes, though, if the intersection is "sharp" in any coord (i. e. zero volume),
the boxes are considered non-intersecting. */
class cBoundingBox:
	public cCuboid<double>
{
public:
	/** Returns true if this bounding box is intersected by the line specified by its two points
	Also calculates the distance along the line in which the intersection occurs, and the face hit (BLOCK_FACE_ constants)
	Only forward collisions (a_LineCoeff >= 0) are returned.
	Exported to Lua manually, because ToLua++ would generate needless input params (a_LineCoeff, a_Face). */
	bool CalcLineIntersection(Vector3d a_LinePoint1, Vector3d a_LinePoint2, double & a_LineCoeff, eBlockFace & a_Face) const;

	/** Returns true if the specified bounding box is intersected by the line specified by its two points
	Also calculates the distance along the line in which the intersection occurs, and the face hit (BLOCK_FACE_ constants)
	Only forward collisions (a_LineCoeff >= 0) are returned.
	Exported to Lua manually, because ToLua++ would generate needless input params (a_LineCoeff, a_Face). */
	static bool CalcLineIntersection(Vector3d a_Min, Vector3d a_Max, Vector3d a_LinePoint1, Vector3d a_LinePoint2, double & a_LineCoeff, eBlockFace & a_Face);
};




