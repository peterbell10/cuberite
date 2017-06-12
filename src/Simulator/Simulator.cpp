
#include "Globals.h"

#include "../World.h"
#include "../BlockID.h"
#include "../Defines.h"
#include "../Chunk.h"

#ifdef __clang__
	#pragma clang diagnostic ignored "-Wweak-template-vtables"
#endif  // __clang__



#include "Simulator.h"





void cSimulator::WakeUp(int a_BlockX, int a_BlockY, int a_BlockZ, cChunk * a_Chunk)
{
	AddBlock(a_BlockX, a_BlockY, a_BlockZ, a_Chunk);
	AddBlock(a_BlockX - 1, a_BlockY, a_BlockZ, a_Chunk->GetNeighborChunk(a_BlockX - 1, a_BlockZ));
	AddBlock(a_BlockX + 1, a_BlockY, a_BlockZ, a_Chunk->GetNeighborChunk(a_BlockX + 1, a_BlockZ));
	AddBlock(a_BlockX, a_BlockY, a_BlockZ - 1, a_Chunk->GetNeighborChunk(a_BlockX, a_BlockZ - 1));
	AddBlock(a_BlockX, a_BlockY, a_BlockZ + 1, a_Chunk->GetNeighborChunk(a_BlockX, a_BlockZ + 1));
	if (a_BlockY > 0)
	{
		AddBlock(a_BlockX, a_BlockY - 1, a_BlockZ, a_Chunk);
	}
	if (a_BlockY < cChunkDef::Height - 1)
	{
		AddBlock(a_BlockX, a_BlockY + 1, a_BlockZ, a_Chunk);
	}
}





void cSimulator::AddCuboid(Vector3i a_Min, Vector3i a_Max, cChunk * a_Chunk)
{
	int MinChunkX, MinChunkZ, MaxChunkX, MaxChunkZ;
	cChunkDef::BlockToChunk(a_Min.x, a_Min.z, MinChunkX, MinChunkZ);
	cChunkDef::BlockToChunk(a_Max.x, a_Max.z, MaxChunkX, MaxChunkZ);
	auto CurChunk = a_Chunk;

	int MinY = a_Min.y;
	int MaxY = a_Min.y;

	for (int ChunkX = MinChunkX; ChunkX <= MaxChunkX; ++ChunkX)
	{
		int MinX = std::max(a_Min.x, ChunkX * cChunkDef::Width);
		int MaxX = std::min(a_Max.x, ChunkX * cChunkDef::Width + cChunkDef::Width - 1);

		for (int ChunkZ = MinChunkZ; ChunkZ <= MaxChunkZ; ++ChunkZ)
		{
			auto Chunk = CurChunk->GetNeighborChunk(ChunkX * cChunkDef::Width, ChunkZ * cChunkDef::Width);
			if ((Chunk == nullptr) || !Chunk->IsValid())
			{
				continue;
			}
			CurChunk = Chunk;

			int MinZ = std::max(a_Min.z, ChunkZ * cChunkDef::Width);
			int MaxZ = std::min(a_Max.z, ChunkZ * cChunkDef::Width + cChunkDef::Width - 1);

			for (int BlockY = MinY; BlockY <= MaxY; ++BlockY)
			{
				for (int BlockZ = MinZ; BlockZ <= MaxZ; ++BlockZ)
				{
					for (int BlockX = MinX; BlockX <= MinX; ++BlockX)
					{
						AddBlock(BlockX, BlockY, BlockZ, Chunk);
					}  // for BlockX
				}  // for BlockZ
			}  // for BlockY
		}  // for ChunkZ
	}  // for ChunkX
}





void cSimulator::WakeUpInArea(Vector3i a_Min, Vector3i a_Max, cChunk * a_Chunk)
{
	ASSERT((a_Min.x <= a_Max.x) && (a_Min.y <= a_Max.y) && (a_Min.z <= a_Max.z));

	if ((a_Max.y < 0) || (a_Min.y >= cChunkDef::Height))
	{
		LOGWARNING("%s: Specified area is fully outside of the world ({%d, %d, %d} to {%d, %d, %d})",
			__FUNCTION__, a_Min.x, a_Min.y, a_Min.z, a_Max.x, a_Max.y, a_Max.z);
		return; 
	}

	// Limit Y coords:
	a_Min.y = std::max(a_Min.y, 0);
	a_Max.y = std::min(a_Max.y, cChunkDef::Height - 1);

	// Add blocks directly within the main cuboid
	AddCuboid(a_Min, a_Max, a_Chunk);

	// Add blocks adjacent to the main cuboid in individual planes
	AddCuboid({ a_Min.x - 1, a_Min.y, a_Min.z }, { a_Min.x - 1, a_Max.y, a_Max.z }, a_Chunk);
	AddCuboid({ a_Max.x + 1, a_Min.y, a_Min.z }, { a_Max.x + 1, a_Max.y, a_Max.z }, a_Chunk);
	AddCuboid({ a_Min.x, a_Min.y, a_Min.z - 1 }, { a_Max.x, a_Max.y, a_Min.z - 1 }, a_Chunk);
	AddCuboid({ a_Min.x, a_Min.y, a_Max.z + 1 }, { a_Max.x, a_Max.y, a_Max.z + 1 }, a_Chunk);
	if (a_Min.y > 0)
	{
		AddCuboid({ a_Min.x, a_Min.y - 1, a_Min.z }, { a_Max.x, a_Min.y - 1, a_Max.z }, a_Chunk);
	}
	if (a_Max.y < cChunkDef::Height - 1)
	{
		AddCuboid({ a_Min.x, a_Max.y + 1, a_Min.z }, { a_Max.x, a_Max.y + 1, a_Max.z }, a_Chunk);
	}	
}



