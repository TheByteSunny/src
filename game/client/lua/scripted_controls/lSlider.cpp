//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include <vgui_int.h>
#include <luamanager.h>
#include <vgui_controls/lPanel.h>
#include "lSlider.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

LSlider::LSlider(Panel *parent, const char *panelName, lua_State *L) : Slider(parent, panelName)
{
#if defined( LUA_SDK )
	m_lua_State = L;
	m_nTableReference = LUA_NOREF;
	m_nRefCount = 0;
#endif
}

LSlider::~LSlider()
{
#if defined( LUA_SDK )
	lua_unref( m_lua_State, m_nTableReference );
#endif
}


/*
** access functions (stack -> C)
*/

LUA_API lua_Slider *lua_toslider(lua_State *L, int idx)
{
	PHandle *phPanel = dynamic_cast<PHandle *>((PHandle *)lua_touserdata(L, idx));
	if (phPanel == NULL) return NULL;
	return dynamic_cast<lua_Slider *>(phPanel->Get());
}


/*
** push functions (C -> stack)
*/

LUA_API void lua_pushslider(lua_State *L, Slider *pSlider)
{
	LSlider *plSlider = dynamic_cast<LSlider *>(pSlider);
	if (plSlider) ++plSlider->m_nRefCount;
	PHandle *phPanel = (PHandle *)lua_newuserdata(L, sizeof(PHandle));
	phPanel->Set(pSlider);
	luaL_getmetatable(L, "Slider");
	lua_setmetatable(L, -2);
}

LUALIB_API lua_Slider *luaL_checkslider(lua_State *L, int narg)
{
	lua_Slider *d = lua_toslider(L, narg);
	if (d == NULL) luaL_argerror(L, narg, "Slider expected, got INVALID_PANEL");
	return d;
}

static int Slider_SetRange(lua_State *L)
{
	luaL_checkslider(L, 1)->SetRange(luaL_checkint(L, 2), luaL_checkint(L, 3));
	return 0;
}

static int Slider_GetRange(lua_State *L)
{
	int min, max;
	luaL_checkslider(L, 1)->GetRange(min, max);
	lua_pushinteger(L, min);
	lua_pushinteger(L, max);
	return 2;
}

static int Slider_SetValue(lua_State *L)
{
	luaL_checkslider(L, 1)->SetValue(luaL_checkint(L, 2));
	return 0;
}

static int Slider_GetValue(lua_State *L)
{
	lua_pushinteger(L, luaL_checkslider(L, 1)->GetValue());
	return 1;
}

static int Slider_SetPaintBackgroundEnabled(lua_State *L)
{
	luaL_checkslider(L, 1)->SetPaintBackgroundEnabled(luaL_checkboolean(L, 2));
	return 0;
}

static int Slider___index(lua_State *L)
{
	Slider *pSlider = lua_toslider(L, 1);
	if (pSlider == NULL)
	{
		lua_Debug ar1; lua_getstack(L, 1, &ar1); lua_getinfo(L, "fl", &ar1);
		lua_Debug ar2; lua_getinfo(L, ">S", &ar2);
		lua_pushfstring(L, "%s:%d: attempt to index INVALID_PANEL", ar2.short_src, ar1.currentline);
		return lua_error(L);
	}
	LSlider *plSlider = dynamic_cast<LSlider *>(pSlider);
	if (plSlider && plSlider->m_nTableReference != LUA_NOREF)
	{
		lua_getref(L, plSlider->m_nTableReference);
		lua_pushvalue(L, 2);
		lua_gettable(L, -2);
		if (lua_isnil(L, -1))
		{
			lua_pop(L, 2);
			lua_getmetatable(L, 1);
			lua_pushvalue(L, 2);
			lua_gettable(L, -2);
			if (lua_isnil(L, -1))
			{
				lua_pop(L, 2);
				luaL_getmetatable(L, "Panel");
				lua_pushvalue(L, 2);
				lua_gettable(L, -2);
			}
		}
	}
	else
	{
		lua_getmetatable(L, 1);
		lua_pushvalue(L, 2);
		lua_gettable(L, -2);
		if (lua_isnil(L, -1))
		{
			lua_pop(L, 2);
			luaL_getmetatable(L, "Panel");
			lua_pushvalue(L, 2);
			lua_gettable(L, -2);
		}
	}
	return 1;
}

static int Slider___newindex(lua_State *L)
{
	Slider *pSlider = lua_toslider(L, 1);
	if (pSlider == NULL)
	{
		lua_Debug ar1; lua_getstack(L, 1, &ar1); lua_getinfo(L, "fl", &ar1);
		lua_Debug ar2; lua_getinfo(L, ">S", &ar2);
		lua_pushfstring(L, "%s:%d: attempt to index INVALID_PANEL", ar2.short_src, ar1.currentline);
		return lua_error(L);
	}
	LSlider *plSlider = dynamic_cast<LSlider *>(pSlider);
	if (plSlider)
	{
		if (plSlider->m_nTableReference == LUA_NOREF)
		{
			lua_newtable(L);
			plSlider->m_nTableReference = luaL_ref(L, LUA_REGISTRYINDEX);
		}
		lua_getref(L, plSlider->m_nTableReference);
		lua_pushvalue(L, 3);
		lua_setfield(L, -2, luaL_checkstring(L, 2));
		lua_pop(L, 1);
		return 0;
	}
	else
	{
		lua_Debug ar1; lua_getstack(L, 1, &ar1); lua_getinfo(L, "fl", &ar1);
		lua_Debug ar2; lua_getinfo(L, ">S", &ar2);
		lua_pushfstring(L, "%s:%d: attempt to index non-scripted panel", ar2.short_src, ar1.currentline);
		return lua_error(L);
	}
}

static int Slider___gc(lua_State *L)
{
	LSlider *plSlider = dynamic_cast<LSlider *>(lua_toslider(L, 1));
	if (plSlider)
	{
		--plSlider->m_nRefCount;
		if (plSlider->m_nRefCount <= 0) delete plSlider;
	}
	return 0;
}

static int Slider___eq(lua_State *L)
{
	lua_pushboolean(L, lua_toslider(L, 1) == lua_toslider(L, 2));
	return 1;
}

static int Slider___tostring(lua_State *L)
{
	Slider *pSlider = lua_toslider(L, 1);
	if (pSlider == NULL) lua_pushstring(L, "INVALID_PANEL");
	else
	{
		const char *pName = pSlider->GetName();
		if (Q_strcmp(pName, "") == 0) pName = "(no name)";
		lua_pushfstring(L, "Slider: \"%s\"", pName);
	}
	return 1;
}

static const luaL_Reg SliderMeta[] =
{
	{"SetRange", Slider_SetRange},
	{"GetRange", Slider_GetRange},
	{"SetValue", Slider_SetValue},
	{"GetValue", Slider_GetValue},
	{"SetPaintBackgroundEnabled", Slider_SetPaintBackgroundEnabled},
	{"__index", Slider___index},
	{"__newindex", Slider___newindex},
	{"__gc", Slider___gc},
	{"__eq", Slider___eq},
	{"__tostring", Slider___tostring},
	{NULL, NULL}
};

static int luasrc_Slider(lua_State *L)
{
	Slider *pSlider = new LSlider(luaL_optpanel(L, 1, VGui_GetClientLuaRootPanel()),
								   luaL_optstring(L, 2, NULL), L);
	lua_pushslider(L, pSlider);
	return 1;
}

static const luaL_Reg Slider_funcs[] =
{
	{"Slider", luasrc_Slider},
	{NULL, NULL}
};

/* open Slider object */
LUALIB_API int luaopen_vgui_Slider(lua_State *L)
{
	luaL_newmetatable(L, "Slider");
	luaL_register(L, NULL, SliderMeta);
	lua_pushstring(L, "panel");
	lua_setfield(L, -2, "__type");
	luaL_register(L, "vgui", Slider_funcs);
	lua_pop(L, 2);
	return 1;
}
