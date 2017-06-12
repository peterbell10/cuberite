
// LuaWindow.h

// Declares the cLuaWindow class representing a virtual window that plugins may create and open for the player





#pragma once

#include <atomic>
#include "LuaState.h"
#include "../UI/Window.h"
#include "../ItemGrid.h"


class cPlayer;
typedef cItemCallback<cPlayer> cPlayerListCallback;


/** A window that has been created by a Lua plugin and is handled entirely by that plugin
This object needs extra care with its lifetime management:
- It is created by Lua, so Lua expects to garbage-collect it later
- Normal cWindow objects are deleted when the last player closes them
	To overcome this, this object maintains a shared_ptr to itself which 
	prevents the ref count from falling to 0 while lua is still using it.
- Lua could GC the window while a player is still using it
	The `.collector' lua function is specified in MaualBindings.cpp to
	simply clear the self reference so players may still keep it alive.
*/
// tolua_begin
class cLuaWindow :
	public cWindow
	// tolua_end
	, public cItemGrid::cListener,
	public std::enable_shared_from_this<cLuaWindow>
{  // tolua_export
	typedef cWindow Super;

	/** Private tag type allows public constructors which can only be called with private access.
	This allows make_shared to work within Create */
	struct make_shared_tag {};

public:
	/** Create a window of the specified type, with a slot grid of a_SlotsX * a_SlotsY size.
	Exported in ManualBindings.cpp */
	cLuaWindow(make_shared_tag, cLuaState & a_LuaState, cWindow::WindowType a_WindowType, int a_SlotsX, int a_SlotsY, const AString & a_Title);

	virtual ~cLuaWindow() override;

	/** Returns the internal representation of the contents that are manipulated by Lua */
	cItemGrid & GetContents(void) { return m_Contents; }  // tolua_export

	/** Sets the Lua callback function to call when the window is about to close */
	void SetOnClosing(cLuaState::cCallbackPtr && a_OnClosing);

	/** Sets the Lua callback function to call when a slot is changed */
	void SetOnSlotChanged(cLuaState::cCallbackPtr && a_OnSlotChanged);

	/** Only this function may be used to create a cLuaWindow as there is no way to initialize m_Self in a normal constructor. */
	template <class... Args>
	static std::shared_ptr<cLuaWindow> Create(Args &&... a_Args)
	{
		auto Window = std::make_shared<cLuaWindow>(make_shared_tag{}, std::forward<Args>(a_Args)...);
		Window->m_Self = Window;
		return Window;
	}

	/** Clear the shared_ptr to self, to be called only by the lua GC */
	void ClearRef();

protected:

	/** Contents of the non-inventory part */
	cItemGrid m_Contents;

	/** The canon Lua state that has opened the window and owns the m_LuaRef */
	cLuaState * m_LuaState;

	/** The Lua callback to call when the window is closing for any player */
	cLuaState::cCallbackPtr m_OnClosing;

	/** The Lua callback to call when a slot has changed */
	cLuaState::cCallbackPtr m_OnSlotChanged;

	/** Reference to self, to keep shared_ptr use count from falling to 0 while lua is still using it.
	Must be set to this after construction and is cleared by the lua GC from ManualBindings.cpp */
	std::shared_ptr<cLuaWindow> m_Self;

	// cWindow overrides:
	virtual bool ClosedByPlayer(cPlayer & a_Player, bool a_CanRefuse) override;
	virtual void DistributeStack(cItem & a_ItemStack, int a_Slot, cPlayer & a_Player, cSlotArea * a_ClickedArea, bool a_ShouldApply) override;

	// cItemGrid::cListener overrides:
	virtual void OnSlotChanged(cItemGrid * a_ItemGrid, int a_SlotNum) override;
} ;  // tolua_export





