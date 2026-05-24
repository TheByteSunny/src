//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef LTEXTENTRY_H
#define LTEXTENTRY_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/TextEntry.h>

namespace vgui
{

//-------------------------------------------------------------------------
// Purpose:
//-------------------------------------------------------------------------
class LTextEntry : public TextEntry
{
	DECLARE_CLASS_SIMPLE( LTextEntry, TextEntry );

public:
	LTextEntry(Panel *parent, const char *panelName, const char *text, lua_State *L);
	virtual ~LTextEntry();

#if defined( LUA_SDK )
	lua_State *m_lua_State;
	int m_nTableReference;
	int m_nRefCount;
#endif
};

} // namespace vgui

/* type for TextEntry functions */
typedef vgui::TextEntry lua_TextEntry;


/*
** access functions (stack -> C)
*/

LUA_API lua_TextEntry *(lua_totextentry) (lua_State *L, int idx);


/*
** push functions (C -> stack)
*/

LUA_API void (lua_pushtextentry) (lua_State *L, lua_TextEntry *pTextEntry);


LUALIB_API lua_TextEntry *(luaL_checktextentry) (lua_State *L, int narg);

#endif // LTEXTENTRY_H
