
#pragma once





/** Represents two sets of coords, minimum and maximum for each direction.
All the coords within those limits (inclusive the edges) are considered "inside" the box. */
template <typename T>
class cCuboid
{
public:

	cCuboid() = default;
	cCuboid(Vector3<T> a_Min, Vector3<T> a_Max):
		m_Min(a_Min),
		m_Max(a_Max)
	{
		Sort();
	}

	Vector3<T> Diff() const { return p2 - p1; }

	/** Returns the volume of the cuboid, in blocks.
	Note that the volume considers both coords inclusive.
	Works on unsorted cuboids, too. */
	T Volume(void) const
	{
		static_assert(std::is_signed<T>::value, "cCuboid<T> assumes T is signed");
		auto Diff = Dif();
		Diff.Abs();
		Diff += Vector3<T>{ 1, 1, 1 };
		return Diff.x * Diff.y * Diff.z;
	}

	void Move(Vector3<T> a_Offset)
	{
		m_Min += a_Offset;
		m_Max += a_Offset;
	}

	void Expand(Vector3<T> a_ExpandSize)
	{
		m_Min -= a_ExpandSize;
		m_Max += a_ExpandSize;
	}

	/** Returns true if the cuboids have at least one voxel in common. Both coords are considered inclusive.
	Assumes both cuboids are sorted. */
	bool DoesIntersect(const cCuboid & a_Other) const
	{
		// In order for cuboids to intersect, each of their coord intervals need to intersect
		return (
			DoIntervalsIntersect(p1.x, p2.x, a_Other.p1.x, a_Other.p2.x) &&
			DoIntervalsIntersect(p1.y, p2.y, a_Other.p1.y, a_Other.p2.y) &&
			DoIntervalsIntersect(p1.z, p2.z, a_Other.p1.z, a_Other.p2.z)
		);
	}

	/** Returns the union of the two cuboids. */
	cCuboid Union(const cCuboid & a_Other) const
	{
		return {
			VectorMin(m_Min, a_Other.m_Min),
			VectorMax(m_Max, a_Other.m_Max)
		};
	}

	/** Returns the intersection of the two cuboids. */
	cCuboid Intersection(const cCuboid & a_Other) const
	{
		return {
			VectorMax(m_Min, a_Other.m_Min),
			VectorMin(m_Max, a_Other.m_Max)
		};
	}

	bool IsEmpty() const
	{
		return (
		    (m_Min.x == m_Max.x) ||
		    (m_Min.y == m_Max.y) ||
		    (m_Min.z == m_Max.z)
		);
	}

	template <typename U>
	bool IsInside(Vector3<U> v) const
	{
		return (
			(v.x >= m_Min.x) && (v.x <= m_Max.x) &&
			(v.y >= m_Min.y) && (v.y <= m_Max.y) &&
			(v.z >= m_Min.z) && (v.z <= m_Max.z)
		);
	}

	bool IsInside(const cCuboid & a_Other) const
	{
		return (IsInside(a_Other.m_Min) && IsInside(a_Other.m_Max));
	}

	/** Returns true if this cuboid is completely inside the specifie cuboid (in all 6 coords).
	Assumes both cuboids are sorted. */
	bool IsCompletelyInside(const cCuboid & a_Outer) const
	{
		return (
		        a_Outer.IsInside(m_Min) &&
		        a_Outer.IsInside(m_Max)
	   );
	}

	/** Moves the cuboid by the specified offsets in each direction */
	void Move(Vector3<T> a_Offset)
	{
		p1 += a_Offset;
		p2 += a_Offset;
	}

	/** Expands the cuboid by the specified amount in each direction.
	Works on unsorted cuboids as well.
	Note that this function doesn't check for underflows when using negative amounts. */
	void Expand(Vector3<T> a_SubMin, Vector3<T> a_AddMax)
	{
		p1 -= a_SubMin;
		p2 += a_AddMax;
	}

	/** Clamps both X coords to the specified range. Works on unsorted cuboids, too. */
	void ClampX(T a_MinX, T a_MaxX)
	{
		m_Min.x = Clamp(m_Min.x, a_MinX, a_MaxX);
		m_Max.x = Clamp(m_Max.x, a_MinX, a_MaxX);
	}

	/** Clamps both Y coords to the specified range. Works on unsorted cuboids, too. */
	void ClampY(T a_MinY, T a_MaxY)
	{
		m_Min.y = Clamp(m_Min.y, a_MinY, a_MaxY);
		m_Max.y = Clamp(m_Max.y, a_MinY, a_MaxY);
	}

	/** Clamps both Z coords to the specified range. Works on unsorted cuboids, too. */
	void ClampZ(T a_MinZ, T a_MaxZ)
	{
		m_Min.z = Clamp(m_Min.z, a_MinZ, a_MaxZ);
		m_Max.z = Clamp(m_Max.z, a_MinZ, a_MaxZ);
	}

	/** If needed, expands the cuboid so that it contains the specified point. Assumes sorted. Doesn't contract. */
	void Engulf(Vector3<T> a_Point)
	{
		m_Min = VectorMin(m_Min, a_Point);
		m_Max = VectorMax(m_Max, a_Point);
	}

	Vector3<T> GetMin() const { return m_Min; }
	Vector3<T> GetMax() const { return m_Max; }

	void SetMin(Vector3<T> a_NewMin)
	{
		m_Min = a_NewMin;
		m_Max = VectorMax(a_NewMin, m_Max);
	}
	void SetMax(Vector3<T> a_NewMax)
	{
		m_Max = a_NewMax;
		m_Min = VectorMin(a_NewMax, m_Min);
	}

private:

	Vector3<T> m_Min, m_Max;  // Maintained as m_Min.[xyz] <= m_Max.[xyz] 

	/** Returns true if the two specified intervals have a non-empty union */
	static bool DoIntervalsIntersect(T a_Min1, T a_Max1, T a_Min2, T a_Max2)
	{
		ASSERT(a_Min1 <= a_Max1);
		ASSERT(a_Min2 <= a_Max2);
		return ((a_Min1 <= a_Max2) && (a_Max1 >= a_Min2));
	}

	void Sort()
	{
		m_Min = VectorMin(p1, p2);
		m_Max = VectorMax(p1, p2);
	}

	template <typename U>
	static Vector3<U> VectorMin(Vector3<U> a_Vec1, Vector3<U> a_Vec2)
	{
		return {
			std::min(a_Vec1.x, a_Vec2.x),
			std::min(a_Vec1.y, a_Vec2.y),
			std::min(a_Vec1.z, a_Vec2.z)
		};
	}

	template <typename U>
	static Vector3<U> VectorMax(Vector3<U> a_Vec1, Vector3<U> a_Vec2)
	{
		return {
			std::max(a_Vec1.x, a_Vec2.x),
			std::max(a_Vec1.y, a_Vec2.y),
			std::max(a_Vec1.z, a_Vec2.z)
		};
	}

};



