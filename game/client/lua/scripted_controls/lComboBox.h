//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef LCOMBOBOX_H
#define LCOMBOBOX_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/ComboBox.h>

namespace vgui
{

//-------------------------------------------------------------------------
// Purpose:
//-------------------------------------------------------------------------
class LComboBox : public ComboBox
{
	DECLARE_CLASS_SIMPLE(LComboBox, ComboBox);
public:
	LComboBox(Panel* parent, const char* panelName, int numLines, bool allowEdit, lua_State* L);
	virtual ~LComboBox();
#if defined(LUA_SDK)
	lua_State* m_lua_State;
	int m_nTableReference;
	int m_nRefCount;
#endif
};

}

/* type for ComboBox functions */
typedef vgui::ComboBox lua_ComboBox;



/*
** access functions (stack -> C)
*/

LUA_API lua_ComboBox* lua_tocombobox(lua_State* L, int idx);



/*
** push functions (C -> stack)
*/

LUA_API void lua_pushcombobox(lua_State* L, lua_ComboBox* pComboBox);



LUALIB_API lua_ComboBox* luaL_checkcombobox(lua_State* L, int narg);

#endif // LCOMBOBOX_H
