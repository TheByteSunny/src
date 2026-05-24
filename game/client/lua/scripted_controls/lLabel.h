//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef LLABEL_H
#define LLABEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Label.h>

namespace vgui
{

//-------------------------------------------------------------------------
// Purpose:
//-------------------------------------------------------------------------
class LLabel : public Label
{
	DECLARE_CLASS_SIMPLE( LLabel, Label );

public:
	LLabel(Panel *parent, const char *panelName, const char *text, lua_State *L);
	virtual ~LLabel();

public:
#if defined( LUA_SDK )
	lua_State *m_lua_State;
	int m_nTableReference;
	int m_nRefCount;
#endif
};

} // namespace vgui

/* type for Label functions */
typedef vgui::Label lua_Label;



/*
** access functions (stack -> C)
*/

LUA_API lua_Label *(lua_tolabel) (lua_State *L, int idx);



/*
** push functions (C -> stack)
*/

LUA_API void (lua_pushlabel) (lua_State *L, lua_Label *pLabel);

LUALIB_API lua_Label *(luaL_checklabel) (lua_State *L, int narg);

#endif // LLABEL_H
