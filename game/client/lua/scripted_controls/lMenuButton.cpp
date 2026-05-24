//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include <vgui_int.h>
#include <luamanager.h>
#include <vgui_controls/lPanel.h>
#include "lMenuButton.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

LMenuButton::LMenuButton(Panel* parent, const char* panelName, const char* text, lua_State* L) : MenuButton(parent, panelName, text)
{
#if defined(LUA_SDK)
	m_lua_State = L;
	m_nTableReference = LUA_NOREF;
	m_nRefCount = 0;
#endif
}

LMenuButton::~LMenuButton()
{
#if defined(LUA_SDK)
	lua_unref(m_lua_State, m_nTableReference);
#endif
}


/*
** access functions (stack -> C)
*/

LUA_API lua_MenuButton* lua_tomenubutton(lua_State* L, int idx)
{
	PHandle* phPanel = dynamic_cast<PHandle*>((PHandle*)lua_touserdata(L, idx));
	if (phPanel == NULL) return NULL;
	return dynamic_cast<lua_MenuButton*>(phPanel->Get());
}


/*
** push functions (C -> stack)
*/

LUA_API void lua_pushmenubutton(lua_State* L, MenuButton* pMenuButton)
{
	LMenuButton* plMenuButton = dynamic_cast<LMenuButton*>(pMenuButton);
	if (plMenuButton) ++plMenuButton->m_nRefCount;
	PHandle* phPanel = (PHandle*)lua_newuserdata(L, sizeof(PHandle));
	phPanel->Set(pMenuButton);
	luaL_getmetatable(L, "MenuButton");
	lua_setmetatable(L, -2);
}

LUALIB_API lua_MenuButton* luaL_checkmenubutton(lua_State* L, int narg)
{
	lua_MenuButton* d = lua_tomenubutton(L, narg);
	if (d == NULL) luaL_argerror(L, narg, "MenuButton expected, got INVALID_PANEL");
	return d;
}

static int MenuButton_SetEnabled(lua_State* L)
{
	luaL_checkmenubutton(L, 1)->SetEnabled(luaL_checkboolean(L, 2));
	return 0;
}

static int MenuButton_IsEnabled(lua_State* L)
{
	lua_pushboolean(L, luaL_checkmenubutton(L, 1)->IsEnabled());
	return 1;
}

static int MenuButton___index(lua_State* L)
{
	MenuButton* pMenuButton = lua_tomenubutton(L, 1);
	if (pMenuButton == NULL)
	{
		lua_Debug ar1; lua_getstack(L, 1, &ar1); lua_getinfo(L, "fl", &ar1);
		lua_Debug ar2; lua_getinfo(L, ">S", &ar2);
		lua_pushfstring(L, "%s:%d: attempt to index INVALID_PANEL", ar2.short_src, ar1.currentline);
		return lua_error(L);
	}
	LMenuButton* plMenuButton = dynamic_cast<LMenuButton*>(pMenuButton);
	if (plMenuButton && plMenuButton->m_nTableReference != LUA_NOREF)
	{
		lua_getref(L, plMenuButton->m_nTableReference);
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
				luaL_getmetatable(L, "Button");
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
	}
	else
	{
		lua_getmetatable(L, 1);
		lua_pushvalue(L, 2);
		lua_gettable(L, -2);
		if (lua_isnil(L, -1))
		{
			lua_pop(L, 2);
			luaL_getmetatable(L, "Button");
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
	return 1;
}

static int MenuButton___newindex(lua_State* L)
{
	MenuButton* pMenuButton = lua_tomenubutton(L, 1);
	if (pMenuButton == NULL)
	{
		lua_Debug ar1; lua_getstack(L, 1, &ar1); lua_getinfo(L, "fl", &ar1);
		lua_Debug ar2; lua_getinfo(L, ">S", &ar2);
		lua_pushfstring(L, "%s:%d: attempt to index INVALID_PANEL", ar2.short_src, ar1.currentline);
		return lua_error(L);
	}
	LMenuButton* plMenuButton = dynamic_cast<LMenuButton*>(pMenuButton);
	if (plMenuButton)
	{
		if (plMenuButton->m_nTableReference == LUA_NOREF)
		{
			lua_newtable(L);
			plMenuButton->m_nTableReference = luaL_ref(L, LUA_REGISTRYINDEX);
		}
		lua_getref(L, plMenuButton->m_nTableReference);
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

static int MenuButton___gc(lua_State* L)
{
	LMenuButton* plMenuButton = dynamic_cast<LMenuButton*>(lua_tomenubutton(L, 1));
	if (plMenuButton)
	{
		--plMenuButton->m_nRefCount;
		if (plMenuButton->m_nRefCount <= 0) delete plMenuButton;
	}
	return 0;
}

static int MenuButton___eq(lua_State* L)
{
	lua_pushboolean(L, lua_tomenubutton(L, 1) == lua_tomenubutton(L, 2));
	return 1;
}

static int MenuButton___tostring(lua_State* L)
{
	MenuButton* pMenuButton = lua_tomenubutton(L, 1);
	if (pMenuButton == NULL) lua_pushstring(L, "INVALID_PANEL");
	else
	{
		const char* pName = pMenuButton->GetName();
		if (Q_strcmp(pName, "") == 0) pName = "(no name)";
		lua_pushfstring(L, "MenuButton: \"%s\"", pName);
	}
	return 1;
}

static const luaL_Reg MenuButtonMeta[] =
{
	{"SetEnabled", MenuButton_SetEnabled},
	{"IsEnabled", MenuButton_IsEnabled},
	{"__index", MenuButton___index},
	{"__newindex", MenuButton___newindex},
	{"__gc", MenuButton___gc},
	{"__eq", MenuButton___eq},
	{"__tostring", MenuButton___tostring},
	{NULL, NULL}
};

static int luasrc_MenuButton(lua_State* L)
{
	MenuButton* pMenuButton = new LMenuButton(luaL_optpanel(L, 1, VGui_GetClientLuaRootPanel()),
											   luaL_checkstring(L, 2),
											   luaL_checkstring(L, 3), L);
	lua_pushmenubutton(L, pMenuButton);
	return 1;
}

static const luaL_Reg MenuButton_funcs[] =
{
	{"MenuButton", luasrc_MenuButton},
	{NULL, NULL}
};

/* open MenuButton object */
LUALIB_API int luaopen_vgui_MenuButton(lua_State* L)
{
	luaL_newmetatable(L, "MenuButton");
	luaL_register(L, NULL, MenuButtonMeta);
	lua_pushstring(L, "panel");
	lua_setfield(L, -2, "__type");
	luaL_register(L, "vgui", MenuButton_funcs);
	lua_pop(L, 2);
	return 1;
}
