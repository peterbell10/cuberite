
// DropSpenser.h

// Declares the cDropSpenser class representing a common ancestor to the cDispenserEntity and cDropperEntity
// The dropper and dispenser only needs to override the DropSpenseFromSlot() function to provide the specific item behavior





#pragma once

#include "BlockEntityWithItems.h"





class cClientHandle;





// tolua_begin
class cDropSpenserEntity :
	public cBlockEntityWithItems
{
	typedef cBlockEntityWithItems super;

public:
	enum
	{
		ContentsHeight = 3,
		ContentsWidth  = 3,
	} ;

	// tolua_end

	BLOCKENTITY_PROTODEF(cDropSpenserEntity)

	cDropSpenserEntity(BLOCKTYPE a_BlockType, int a_BlockX, int a_BlockY, int a_BlockZ, cWorld * a_World);

	// cBlockEntity overrides:
	virtual bool Tick(std::chrono::milliseconds a_Dt, cChunk & a_Chunk) override;
	virtual void SendTo(cClientHandle & a_Client) override;
	virtual bool UsedBy(cPlayer * a_Player) override;

	// tolua_begin

	/** Modifies the block coords to match the dropspenser direction given (where the dropspensed pickups should materialize) */
	void AddDropSpenserDir(int & a_BlockX, int & a_BlockY, int & a_BlockZ, NIBBLETYPE a_Direction);

	/** Sets the dropspenser to dropspense an item in the next tick */
	void Activate(void);

	// tolua_end

protected:
	bool m_ShouldDropSpense;  ///< If true, the dropspenser will dropspense an item in the next tick

	/** Does the actual work on dropspensing an item. Chooses the slot, calls DropSpenseFromSlot() and handles smoke / sound effects */
	void DropSpense(cChunk & a_Chunk);

	/** Override this function to provide the specific behavior for item dropspensing (drop / shoot / pour / ...) */
	virtual void DropSpenseFromSlot(cChunk & a_Chunk, int a_SlotNum) = 0;

	/** Helper function, drops one item from the specified slot (like a dropper) */
	void DropFromSlot(cChunk & a_Chunk, int a_SlotNum);

	/** Creates a new window for this dropspenser. */
	virtual std::shared_ptr<cWindow> NewWindow() override;
} ;  // tolua_export




