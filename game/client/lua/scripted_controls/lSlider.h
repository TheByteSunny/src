//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef LSLIDER_H
#define LSLIDER_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Slider.h>

namespace vgui
{

//-------------------------------------------------------------------------
// Purpose:
//-------------------------------------------------------------------------
class LSlider : public Slider
{
	DECLARE_CLASS_SIMPLE( LSlider, Slider );

public:
	LSlider(Panel *parent, const char *panelName, lua_State *L);
	virtual ~LSlider();

public:
#if defined( LUA_SDK )
	lua_State *m_lua_State;
	int m_nTableReference;
	int m_nRefCount;
#endif
};

} // namespace vgui

/* type for Slider functions */
typedef vgui::Slider lua_Slider;


/*
** access functions (stack -> C)
*/

LUA_API lua_Slider *(lua_toslider) (lua_State *L, int idx);


/*
** push functions (C -> stack)
*/

LUA_API void (lua_pushslider) (lua_State *L, lua_Slider *pSlider);



LUALIB_API lua_Slider *(luaL_checkslider) (lua_State *L, int narg);

#endif // LSLIDER_H
