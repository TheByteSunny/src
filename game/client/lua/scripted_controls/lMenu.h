//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef LMENU_H
#define LMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Menu.h>

namespace vgui
{

//-------------------------------------------------------------------------
// Purpose:
//-------------------------------------------------------------------------
class LMenu : public Menu
{
	DECLARE_CLASS_SIMPLE(LMenu, Menu);
public:
	LMenu(Panel* parent, const char* panelName, lua_State* L);
	virtual ~LMenu();
#if defined(LUA_SDK)
	lua_State* m_lua_State;
	int m_nTableReference;
	int m_nRefCount;
#endif
};

}

/* type for Menu functions */
typedef vgui::Menu lua_Menu;


/*
** access functions (stack -> C)
*/

LUA_API lua_Menu* lua_tomenu(lua_State* L, int idx);


/*
** push functions (C -> stack)
*/

LUA_API void lua_pushmenu(lua_State* L, lua_Menu* pMenu);



LUALIB_API lua_Menu* luaL_checkmenu(lua_State* L, int narg);

#endif // LMENU_H
