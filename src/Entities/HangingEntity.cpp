
#include "Globals.h"  // NOTE: MSVC stupidness requires this to be the same across all modules

#include "HangingEntity.h"
#include "ClientHandle.h"
#include "Player.h"
#include "Chunk.h"





cHangingEntity::cHangingEntity(eEntityType a_EntityType, eBlockFace a_Facing, double a_X, double a_Y, double a_Z) :
	cEntity(a_EntityType, a_X, a_Y, a_Z, 0.8, 0.8),
	m_Facing(cHangingEntity::BlockFaceToProtocolFace(a_Facing))
{
	SetMaxHealth(1);
	SetHealth(1);
}





void cHangingEntity::SpawnOn(cClientHandle & a_ClientHandle)
{
	SetYaw(GetProtocolFacing() * 90);
}




void cHangingEntity::Tick(std::chrono::milliseconds a_Dt, cChunk & a_Chunk)
{
	UNUSED(a_Dt);
	if ((m_World->GetWorldAge() & 0x7) != 0)
	{
		return;
	}
	Vector3i BlockOnPos = POS_TOINT;
	if (IsPainting())
	{
		AddFaceDirection(BlockOnPos.x, BlockOnPos.y, BlockOnPos.z, ProtocolFaceToBlockFace(m_Facing), true);
	}
	Vector3i RelPos;
	cChunk * BlockChunk = &a_Chunk;
	if (!a_Chunk.GetChunkAndRelByAbsolute(BlockOnPos, &BlockChunk, RelPos))
	{
		// Chunk not valid, no way to check
		return;
	}
	if (BlockChunk->GetBlock(RelPos) == E_BLOCK_AIR)
	{
		Destroy();
	}
}




