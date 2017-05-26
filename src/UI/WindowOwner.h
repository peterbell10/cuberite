#pragma once

#include "../BlockEntities/BlockEntity.h"
#include "../Entities/Entity.h"
#include "../Entities/Player.h"
#include "Window.h"


/* Being a descendant of cWindowOwner means that the class can own one window. That window can be
queried, opened by other players, closed by players and finally destroyed.
Also, a cWindowOwner can be queried for the block coords where the window is displayed. That will be used
for entities / players in motion to close their windows when they get too far away from the window "source". */





/** Base class for the window owning */
class cWindowOwner
{
public:
	cWindowOwner()
	{
	}

	virtual ~cWindowOwner()
	{
		DestroyWindow();
	}


	/** Forcefully close the window for all players. */
	void DestroyWindow()
	{
		auto Window = m_Window.lock();
		if (Window != nullptr)
		{
			Window->OwnerDestroyed();
		}
		m_Window.reset();
	}

	/** Called by window destructor to ensure no unnecessary weak refs are held. (Would prevents some deallocations) */
	void CloseWindow(void)
	{
		m_Window.reset();
	}

	bool OpenWindow(cPlayer * a_Player, bool a_Force = false)
	{
		// If the window is not created, open it anew:
		auto Window = GetWindow();
		if (Window == nullptr)
		{
			Window = NewWindow();
			if (Window == nullptr)
			{
				return false;
			}

			m_Window = Window;
			Window->SetOwner(this);
		}

		// Open the window for the player
		if (a_Force || (a_Player->GetWindow() != Window.get()))
		{
			a_Player->OpenWindow(Window);
		}
		return true;
	}

	std::shared_ptr<cWindow> GetWindow(void)
	{
		return m_Window.lock();
	}

	/** Returns the block position at which the element owning the window is */
	virtual Vector3i GetBlockPos(void) = 0;

private:

	/** Called by OpenWindow whenever m_Window is expired and a new window is needed. */
	virtual std::shared_ptr<cWindow> NewWindow() = 0;

	std::weak_ptr<cWindow> m_Window;
};





/** Window owner that is associated with a block entity (chest, furnace, ...) */
class cBlockEntityWindowOwner :
	public cWindowOwner
{
public:
	cBlockEntityWindowOwner(cBlockEntity * a_BlockEntity) :
		m_BlockEntity(a_BlockEntity)
	{
	}

	virtual Vector3i GetBlockPos(void) override
	{
		return Vector3i(m_BlockEntity->GetPosX(), m_BlockEntity->GetPosY(), m_BlockEntity->GetPosZ());
	}

private:
	cBlockEntity * m_BlockEntity;
};





/** Window owner that is associated with an entity (chest minecart etc.) */
class cEntityWindowOwner :
	public cWindowOwner
{
public:
	cEntityWindowOwner(cEntity * a_Entity) :
		m_Entity(a_Entity)
	{
	}

	virtual Vector3i GetBlockPos(void) override
	{
		return m_Entity->GetPosition().Floor();
	}

private:
	cEntity * m_Entity;
};




