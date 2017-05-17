#pragma once

#include "BlockHandler.h"
#include "MetaRotator.h"

#include "../BlockID.h"

class cBlockEndPortalFrameHandler :
	public cMetaRotator<cBlockHandler, 0x03,
	E_META_END_PORTAL_FRAME_ZM,
	E_META_END_PORTAL_FRAME_XP,
	E_META_END_PORTAL_FRAME_ZP,
	E_META_END_PORTAL_FRAME_XM
	>
{
	using super = cMetaRotator<cBlockHandler, 0x03,
		E_META_END_PORTAL_FRAME_ZM,
		E_META_END_PORTAL_FRAME_XP,
		E_META_END_PORTAL_FRAME_ZP,
		E_META_END_PORTAL_FRAME_XM
	>;
public:
	cBlockEndPortalFrameHandler()
		: super(E_BLOCK_END_PORTAL_FRAME)
	{
	}

	virtual bool GetPlacementBlockTypeMeta(
		cChunkInterface & a_ChunkInterface, cPlayer * a_Player,
		int a_BlockX, int a_BlockY, int a_BlockZ, eBlockFace a_BlockFace,
		int a_CursorX, int a_CursorY, int a_CursorZ,
		BLOCKTYPE & a_BlockType, NIBBLETYPE & a_BlockMeta
	) override
	{
		a_BlockType = m_BlockType;
		a_BlockMeta = YawToMetaData(a_Player->GetYaw());
		return true;
	}

	static NIBBLETYPE YawToMetaData(double a_Rotation)
	{
		a_Rotation += 90 + 45;  // So its not aligned with axis

		if (a_Rotation > 360.f)
		{
			a_Rotation -= 360.f;
		}
		if ((a_Rotation >= 0.f) && (a_Rotation < 90.f))
		{
			return E_META_END_PORTAL_FRAME_XM;
		}
		else if ((a_Rotation >= 180) && (a_Rotation < 270))
		{
			return E_META_END_PORTAL_FRAME_XP;
		}
		else if ((a_Rotation >= 90) && (a_Rotation < 180))
		{
			return E_META_END_PORTAL_FRAME_ZM;
		}
		else
		{
			return E_META_END_PORTAL_FRAME_ZP;
		}
	}
};