//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include <vgui_int.h>
#include <luamanager.h>
#include <vgui_controls/lPanel.h>
#include "lRadioButton.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

LRadioButton::LRadioButton(Panel *parent, const char *panelName, const char *text, lua_State* L) : RadioButton(parent, panelName, text)
{
#if defined(LUA_SDK)
	m_lua_State = L;
	m_nTableReference = LUA_NOREF;
	m_nRefCount = 0;
#endif
}

LRadioButton::~LRadioButton()
{
#if defined(LUA_SDK)
	lua_unref(m_lua_State, m_nTableReference);
#endif
}

/*
** access functions (stack -> C)
*/

LUA_API lua_RadioButton* lua_toradiobutton(lua_State* L, int idx)
{
	PHandle* phPanel = dynamic_cast<PHandle*>((PHandle*)lua_touserdata(L, idx));
	if (phPanel == NULL) return NULL;
	return dynamic_cast<lua_RadioButton*>(phPanel->Get());
}


/*
** push functions (C -> stack)
*/

LUA_API void lua_pushradiobutton(lua_State* L, RadioButton* pRadioButton)
{
	LRadioButton* plRadioButton = dynamic_cast<LRadioButton*>(pRadioButton);
	if (plRadioButton) ++plRadioButton->m_nRefCount;
	PHandle* phPanel = (PHandle*)lua_newuserdata(L, sizeof(PHandle));
	phPanel->Set(pRadioButton);
	luaL_getmetatable(L, "RadioButton");
	lua_setmetatable(L, -2);
}

LUALIB_API lua_RadioButton* luaL_checkradiobutton(lua_State* L, int narg)
{
	lua_RadioButton* d = lua_toradiobutton(L, narg);
	if (d == NULL) luaL_argerror(L, narg, "RadioButton expected, got INVALID_PANEL");
	return d;
}

static int RadioButton_SetSelected(lua_State* L)
{
	luaL_checkradiobutton(L, 1)->SetSelected(luaL_checkboolean(L, 2));
	return 0;
}

static int RadioButton_IsSelected(lua_State* L)
{
	lua_pushboolean(L, luaL_checkradiobutton(L, 1)->IsSelected());
	return 1;
}

static int RadioButton___index(lua_State* L)
{
	RadioButton* pRadioButton = lua_toradiobutton(L, 1);
	if (pRadioButton == NULL)
	{
		lua_Debug ar1; lua_getstack(L, 1, &ar1); lua_getinfo(L, "fl", &ar1);
		lua_Debug ar2; lua_getinfo(L, ">S", &ar2);
		lua_pushfstring(L, "%s:%d: attempt to index INVALID_PANEL", ar2.short_src, ar1.currentline);
		return lua_error(L);
	}
	LRadioButton* plRadioButton = dynamic_cast<LRadioButton*>(pRadioButton);
	if (plRadioButton && plRadioButton->m_nTableReference != LUA_NOREF)
	{
		lua_getref(L, plRadioButton->m_nTableReference);
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

static int RadioButton___newindex(lua_State* L)
{
	RadioButton* pRadioButton = lua_toradiobutton(L, 1);
	if (pRadioButton == NULL)
	{
		lua_Debug ar1; lua_getstack(L, 1, &ar1); lua_getinfo(L, "fl", &ar1);
		lua_Debug ar2; lua_getinfo(L, ">S", &ar2);
		lua_pushfstring(L, "%s:%d: attempt to index INVALID_PANEL", ar2.short_src, ar1.currentline);
		return lua_error(L);
	}
	LRadioButton* plRadioButton = dynamic_cast<LRadioButton*>(pRadioButton);
	if (plRadioButton)
	{
		if (plRadioButton->m_nTableReference == LUA_NOREF)
		{
			lua_newtable(L);
			plRadioButton->m_nTableReference = luaL_ref(L, LUA_REGISTRYINDEX);
		}
		lua_getref(L, plRadioButton->m_nTableReference);
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

static int RadioButton___gc(lua_State* L)
{
	LRadioButton* plRadioButton = dynamic_cast<LRadioButton*>(lua_toradiobutton(L, 1));
	if (plRadioButton)
	{
		--plRadioButton->m_nRefCount;
		if (plRadioButton->m_nRefCount <= 0) delete plRadioButton;
	}
	return 0;
}

static int RadioButton___eq(lua_State* L)
{
	lua_pushboolean(L, lua_toradiobutton(L, 1) == lua_toradiobutton(L, 2));
	return 1;
}

static int RadioButton___tostring(lua_State* L)
{
	RadioButton* pRadioButton = lua_toradiobutton(L, 1);
	if (pRadioButton == NULL) lua_pushstring(L, "INVALID_PANEL");
	else
	{
		const char* pName = pRadioButton->GetName();
		if (Q_strcmp(pName, "") == 0) pName = "(no name)";
		lua_pushfstring(L, "RadioButton: \"%s\"", pName);
	}
	return 1;
}

static const luaL_Reg RadioButtonMeta[] =
{
	{"SetSelected", RadioButton_SetSelected},
	{"IsSelected", RadioButton_IsSelected},
	{"__index", RadioButton___index},
	{"__newindex", RadioButton___newindex},
	{"__gc", RadioButton___gc},
	{"__eq", RadioButton___eq},
	{"__tostring", RadioButton___tostring},
	{NULL, NULL}
};

static int luasrc_RadioButton(lua_State* L)
{
	RadioButton* pRadioButton = new LRadioButton(luaL_optpanel(L, 1, VGui_GetClientLuaRootPanel()),
												 luaL_checkstring(L, 2),
												 luaL_checkstring(L, 3), L);
	lua_pushradiobutton(L, pRadioButton);
	return 1;
}

static const luaL_Reg RadioButton_funcs[] =
{
	{"RadioButton", luasrc_RadioButton},
	{NULL, NULL}
};

/* open RadioButton object */
LUALIB_API int luaopen_vgui_RadioButton(lua_State* L)
{
	luaL_newmetatable(L, "RadioButton");
	luaL_register(L, NULL, RadioButtonMeta);
	lua_pushstring(L, "panel");
	lua_setfield(L, -2, "__type");
	luaL_register(L, "vgui", RadioButton_funcs);
	lua_pop(L, 2);
	return 1;
}
