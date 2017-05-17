
#pragma once

#include "ItemHandler.h"
#include "../Entities/Player.h"
#include "../World.h"



class cItemEyeOfEnderHandler :
	public cItemHandler
{
	using super = cItemHandler;
public:
	cItemEyeOfEnderHandler() :
		super(E_ITEM_EYE_OF_ENDER)
	{
	}



	virtual bool OnItemUse(
		cWorld * a_World, cPlayer * a_Player, cBlockPluginInterface & a_PluginInterface, const cItem & a_Item,
		int a_BlockX, int a_BlockY, int a_BlockZ, eBlockFace a_BlockFace
	) override
	{

		if (a_BlockFace != BLOCK_FACE_NONE)
		{
			// Handle end portal use
			BLOCKTYPE ClickedBlock = 0;
			NIBBLETYPE ClickedBlockMeta = 0;
			VERIFY(a_World->GetBlockTypeMeta(a_BlockX, a_BlockY, a_BlockZ, ClickedBlock, ClickedBlockMeta));
			
			if ((ClickedBlock != E_BLOCK_END_PORTAL_FRAME) ||
			    ((ClickedBlockMeta & 0x04) == E_META_END_PORTAL_EYE)  // Eye in the frame already
			)
			{
				return true;
			}

			// Add eye to frame
			a_World->SetBlockMeta(a_BlockX, a_BlockY, a_BlockZ, ClickedBlockMeta | E_META_END_PORTAL_EYE);
			
			if (!a_Player->IsGameModeCreative())
			{
				a_Player->GetInventory().RemoveOneEquippedItem();
			}

			if(IsPortalComplete(a_World, a_BlockX, a_BlockY, a_BlockZ, ClickedBlockMeta))
			{
				a_Player->SendMessageWarning("End portals are not yet supported.");
			}
		}
		else
		{
			// TODO: Spawn eye of ender signal
		}
		return true;
	}

private:
	bool IsPortalComplete(cWorld * a_World, int a_BlockX, int a_BlockY, int a_BlockZ, NIBBLETYPE a_PortalDirection)
	{
		Vector3i Centre = FindPortalCentre(a_World, a_BlockX, a_BlockY, a_BlockZ, a_PortalDirection);
		return IsPortalComplete(a_World, Centre);
	}


	/** Find the centre block of the end portal.	
	The 3x3 central area cannot hold any end portal frames and either side of this must be an end portal frame. 
	This means the central block can be found by checking only 2 blocks.
	*/
	Vector3i FindPortalCentre(cWorld * a_World, int a_BlockX, int a_BlockY, int a_BlockZ, NIBBLETYPE a_PortalDirection)
	{
		Vector3i Centre(a_BlockX, a_BlockY, a_BlockZ);
		Vector3i PerpDir;  // Perpendicular to facing direction
		switch (a_PortalDirection)
		{
			case E_META_END_PORTAL_FRAME_ZP:
			{
				Centre.z += 2;
				PerpDir.x = 1;
				break;
			}
			case E_META_END_PORTAL_FRAME_ZM:
			{
				Centre.z -= 2;
				PerpDir.x = -1;
				break;
			}
			case E_META_END_PORTAL_FRAME_XP:
			{
				Centre.x += 2;
				PerpDir.z = -1;
				break;
			}
			case E_META_END_PORTAL_FRAME_XM:
			{
				Centre.x -= 2;
				PerpDir.z = 1;
				break;
			}
		}

		if (a_World->GetBlock(Centre + PerpDir) == E_BLOCK_END_PORTAL_FRAME)
		{
			Centre -= PerpDir;
		}
		else if (a_World->GetBlock(Centre - PerpDir) == E_BLOCK_END_PORTAL_FRAME)
		{
			Centre += PerpDir;
		}
		return Centre;
	}




	/** Returns true if the edges of the portal about Centre are eye filled portal frames and no portal frames are in the 3x3 area around the centre.*/
	bool IsPortalComplete(cWorld * a_World, const Vector3i& a_Centre)
	{
		// Check central blocks aren't portal frames
		for (int RelX = -1; RelX != 2; ++RelX)
		{
			for (int RelZ = -1; RelZ != 2; ++RelZ)
			{
				if (a_World->GetBlock(a_Centre.x + RelX, a_Centre.y, a_Centre.z + RelZ) == E_BLOCK_END_PORTAL_FRAME)
				{
					return false;
				}
			}
		}

		// Check edges are filled portal frames
		static const std::array<Vector3i, 12> PortalCheck
		{
			{
				{ -1,  0,  2 },
				{ 0,  0,  2 },
				{ 1,  0,  2 },
				{ -1,  0, -2 },
				{ 0,  0, -2 },
				{ 1,  0, -2 },
				{ 2,  0, -1 },
				{ 2,  0,  0 },
				{ 2,  0,  1 },
				{ -2,  0, -1 },
				{ -2,  0,  0 },
				{ -2,  0,  1 },
			}
		};

		for (auto Pos : PortalCheck)
		{
			BLOCKTYPE Block;
			NIBBLETYPE Meta;
			Pos += a_Centre;
			a_World->GetBlockTypeMeta(Pos.x, Pos.y, Pos.z, Block, Meta);
			if ((Block != E_BLOCK_END_PORTAL_FRAME) ||
				((Meta & 0x04) != E_META_END_PORTAL_EYE)
				)
			{
				return false;
			}
		}

		return true;
	}
};




