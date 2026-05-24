//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef LRADIOBUTTON_H
#define LRADIOBUTTON_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/RadioButton.h>

namespace vgui
{

//-------------------------------------------------------------------------
// Purpose:
//-------------------------------------------------------------------------
class LRadioButton : public RadioButton
{
	DECLARE_CLASS_SIMPLE(LRadioButton, RadioButton);

public:
	LRadioButton(Panel *parent, const char *panelName, const char *text, lua_State* L);
	virtual ~LRadioButton();

#if defined(LUA_SDK)
	lua_State* m_lua_State;
	int m_nTableReference;
	int m_nRefCount;
#endif
};

}

/* type for RadioButton functions */
typedef vgui::RadioButton lua_RadioButton;


/*
** access functions (stack -> C)
*/

LUA_API lua_RadioButton* lua_toradiobutton(lua_State* L, int idx);


/*
** push functions (C -> stack)
*/

LUA_API void lua_pushradiobutton(lua_State* L, lua_RadioButton* pRadioButton);



LUALIB_API lua_RadioButton* luaL_checkradiobutton(lua_State* L, int narg);

#endif // LRADIOBUTTON_H
