//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef LMENUBUTTON_H
#define LMENUBUTTON_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/MenuButton.h>

namespace vgui
{

//-------------------------------------------------------------------------
// Purpose:
//-------------------------------------------------------------------------
class LMenuButton : public MenuButton
{
	DECLARE_CLASS_SIMPLE(LMenuButton, MenuButton);
public:
	LMenuButton(Panel* parent, const char* panelName, const char* text, lua_State* L);
	virtual ~LMenuButton();
#if defined(LUA_SDK)
	lua_State* m_lua_State;
	int m_nTableReference;
	int m_nRefCount;
#endif
};

}

/* type for MenuButton functions */
typedef vgui::MenuButton lua_MenuButton;


/*
** access functions (stack -> C)
*/

LUA_API lua_MenuButton* lua_tomenubutton(lua_State* L, int idx);



/*
** push functions (C -> stack)
*/

LUA_API void lua_pushmenubutton(lua_State* L, lua_MenuButton* pMenuButton);


LUALIB_API lua_MenuButton* luaL_checkmenubutton(lua_State* L, int narg);

#endif // LMENUBUTTON_H
