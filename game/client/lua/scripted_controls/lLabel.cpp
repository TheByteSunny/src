//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include <vgui_int.h>
#include <luamanager.h>
#include <vgui_controls/lPanel.h>
#include "lLabel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

LLabel::LLabel(Panel *parent, const char *panelName, const char *text, lua_State *L) : Label(parent, panelName, text)
{
#if defined( LUA_SDK )
	m_lua_State = L;
	m_nTableReference = LUA_NOREF;
	m_nRefCount = 0;
#endif // LUA_SDK
}

LLabel::~LLabel()
{
#if defined( LUA_SDK )
	lua_unref( m_lua_State, m_nTableReference );
#endif // LUA_SDK
}


/*
** access functions (stack -> C)
*/

LUA_API lua_Label *lua_tolabel(lua_State *L, int idx)
{
	PHandle *phPanel = dynamic_cast<PHandle *>((PHandle *)lua_touserdata(L, idx));
	if (phPanel == NULL)
		return NULL;
	return dynamic_cast<lua_Label *>(phPanel->Get());
}


/*
** push functions (C -> stack)
*/

LUA_API void lua_pushlabel(lua_State *L, Label *pLabel)
{
	LLabel *plLabel = dynamic_cast<LLabel *>(pLabel);
	if (plLabel)
		++plLabel->m_nRefCount;
	PHandle *phPanel = (PHandle *)lua_newuserdata(L, sizeof(PHandle));
	phPanel->Set(pLabel);
	luaL_getmetatable(L, "Label");
	lua_setmetatable(L, -2);
}

LUALIB_API lua_Label *luaL_checklabel(lua_State *L, int narg)
{
	lua_Label *d = lua_tolabel(L, narg);
	if (d == NULL)
		luaL_argerror(L, narg, "Label expected, got INVALID_PANEL");
	return d;
}

static int Label_SetText(lua_State *L)
{
	luaL_checklabel(L, 1)->SetText(luaL_checkstring(L, 2));
	return 0;
}

static int Label_SetContentAlignment(lua_State *L)
{
	luaL_checklabel(L, 1)->SetContentAlignment((Label::Alignment)luaL_checkint(L, 2));
	return 0;
}

static int Label_SetEnabled(lua_State *L)
{
	luaL_checklabel(L, 1)->SetEnabled(luaL_checkboolean(L, 2));
	return 0;
}

static int Label_IsEnabled(lua_State *L)
{
	lua_pushboolean(L, luaL_checklabel(L, 1)->IsEnabled());
	return 1;
}

static int Label_SetVisible(lua_State *L)
{
	luaL_checklabel(L, 1)->SetVisible(luaL_checkboolean(L, 2));
	return 0;
}

static int Label_IsVisible(lua_State *L)
{
	lua_pushboolean(L, luaL_checklabel(L, 1)->IsVisible());
	return 1;
}

static int Label_SetPos(lua_State *L)
{
	luaL_checklabel(L, 1)->SetPos(luaL_checkint(L, 2), luaL_checkint(L, 3));
	return 0;
}

static int Label_GetPos(lua_State *L)
{
	int x, y;
	luaL_checklabel(L, 1)->GetPos(x, y);
	lua_pushinteger(L, x);
	lua_pushinteger(L, y);
	return 2;
}

static int Label_SetSize(lua_State *L)
{
	luaL_checklabel(L, 1)->SetSize(luaL_checkint(L, 2), luaL_checkint(L, 3));
	return 0;
}

static int Label_GetSize(lua_State *L)
{
	int wide, tall;
	luaL_checklabel(L, 1)->GetSize(wide, tall);
	lua_pushinteger(L, wide);
	lua_pushinteger(L, tall);
	return 2;
}

static int Label_SetFont(lua_State *L)
{
	luaL_checklabel(L, 1)->SetFont(scheme()->GetIScheme(scheme()->GetDefaultScheme())->GetFont(luaL_checkstring(L, 2), true));
	return 0;
}

static int Label_GetFont(lua_State *L)
{
	HFont font = luaL_checklabel(L, 1)->GetFont();
	lua_pushinteger(L, font);
	return 1;
}

static int Label_SizeToContents(lua_State *L)
{
	luaL_checklabel(L, 1)->SizeToContents();
	return 0;
}

static int Label_SetWrap(lua_State *L)
{
	luaL_checklabel(L, 1)->SetWrap(luaL_checkboolean(L, 2));
	return 0;
}

static int Label_ChainToAnimationMap(lua_State *L)
{
	luaL_checklabel(L, 1)->ChainToAnimationMap();
	return 0;
}

static int Label_ChainToMap(lua_State *L)
{
	luaL_checklabel(L, 1)->ChainToMap();
	return 0;
}

static int Label_GetPanelBaseClassName(lua_State *L)
{
	lua_pushstring(L, luaL_checklabel(L, 1)->GetPanelBaseClassName());
	return 1;
}

static int Label_GetPanelClassName(lua_State *L)
{
	lua_pushstring(L, luaL_checklabel(L, 1)->GetPanelClassName());
	return 1;
}

static int Label_GetRefTable(lua_State *L)
{
	LLabel *plLabel = dynamic_cast<LLabel *>(luaL_checklabel(L, 1));
	if (plLabel)
	{
		if (plLabel->m_nTableReference == LUA_NOREF)
		{
			lua_newtable(L);
			plLabel->m_nTableReference = luaL_ref(L, LUA_REGISTRYINDEX);
		}
		lua_getref(L, plLabel->m_nTableReference);
	}
	else
		lua_pushnil(L);
	return 1;
}

static int Label_KB_AddBoundKey(lua_State *L)
{
	luaL_checklabel(L, 1)->KB_AddBoundKey(luaL_checkstring(L, 2), luaL_checkint(L, 3), luaL_checkint(L, 4));
	return 0;
}

static int Label_KB_ChainToMap(lua_State *L)
{
	luaL_checklabel(L, 1)->KB_ChainToMap();
	return 0;
}

static int Label___index(lua_State *L)
{
	Label *pLabel = lua_tolabel(L, 1);
	if (pLabel == NULL)
	{
		lua_Debug ar1;
		lua_getstack(L, 1, &ar1);
		lua_getinfo(L, "fl", &ar1);
		lua_Debug ar2;
		lua_getinfo(L, ">S", &ar2);
		lua_pushfstring(L, "%s:%d: attempt to index an INVALID_PANEL", ar2.short_src, ar1.currentline);
		return lua_error(L);
	}
	LLabel *plLabel = dynamic_cast<LLabel *>(pLabel);
	if (plLabel && plLabel->m_nTableReference != LUA_NOREF)
	{
		lua_getref(L, plLabel->m_nTableReference);
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

static int Label___newindex(lua_State *L)
{
	Label *pLabel = lua_tolabel(L, 1);
	if (pLabel == NULL)
	{
		lua_Debug ar1;
		lua_getstack(L, 1, &ar1);
		lua_getinfo(L, "fl", &ar1);
		lua_Debug ar2;
		lua_getinfo(L, ">S", &ar2);
		lua_pushfstring(L, "%s:%d: attempt to index an INVALID_PANEL", ar2.short_src, ar1.currentline);
		return lua_error(L);
	}
	LLabel *plLabel = dynamic_cast<LLabel *>(pLabel);
	if (plLabel)
	{
		if (plLabel->m_nTableReference == LUA_NOREF)
		{
			lua_newtable(L);
			plLabel->m_nTableReference = luaL_ref(L, LUA_REGISTRYINDEX);
		}
		lua_getref(L, plLabel->m_nTableReference);
		lua_pushvalue(L, 3);
		lua_setfield(L, -2, luaL_checkstring(L, 2));
		lua_pop(L, 1);
		return 0;
	}
	else
	{
		lua_Debug ar1;
		lua_getstack(L, 1, &ar1);
		lua_getinfo(L, "fl", &ar1);
		lua_Debug ar2;
		lua_getinfo(L, ">S", &ar2);
		lua_pushfstring(L, "%s:%d: attempt to index a non-scripted panel", ar2.short_src, ar1.currentline);
		return lua_error(L);
	}
}

static int Label___gc(lua_State *L)
{
	LLabel *plLabel = dynamic_cast<LLabel *>(lua_tolabel(L, 1));
	if (plLabel)
	{
		--plLabel->m_nRefCount;
		if (plLabel->m_nRefCount <= 0)
		{
			delete plLabel;
		}
	}
	return 0;
}

static int Label___eq(lua_State *L)
{
	lua_pushboolean(L, lua_tolabel(L, 1) == lua_tolabel(L, 2));
	return 1;
}

static int Label___tostring(lua_State *L)
{
	Label *pLabel = lua_tolabel(L, 1);
	if (pLabel == NULL)
		lua_pushstring(L, "INVALID_PANEL");
	else
	{
		const char *pName = pLabel->GetName();
		if (Q_strcmp(pName, "") == 0)
			pName = "(no name)";
		lua_pushfstring(L, "Label: \"%s\"", pName);
	}
	return 1;
}

static const luaL_Reg Labelmeta[] =
{
	{"ChainToAnimationMap", Label_ChainToAnimationMap},
	{"ChainToMap", Label_ChainToMap},
	{"GetPanelBaseClassName", Label_GetPanelBaseClassName},
	{"GetPanelClassName", Label_GetPanelClassName},
	{"GetRefTable", Label_GetRefTable},
	{"KB_AddBoundKey", Label_KB_AddBoundKey},
	{"KB_ChainToMap", Label_KB_ChainToMap},
	{"SetText", Label_SetText},
	{"SetEnabled", Label_SetEnabled},
	{"IsEnabled", Label_IsEnabled},
	{"SetVisible", Label_SetVisible},
	{"IsVisible", Label_IsVisible},
	{"SetPos", Label_SetPos},
	{"GetPos", Label_GetPos},
	{"SetSize", Label_SetSize},
	{"GetSize", Label_GetSize},
	{"SetFont", Label_SetFont},
	{"GetFont", Label_GetFont},
	{"SizeToContents", Label_SizeToContents},
	{"SetWrap", Label_SetWrap},
	{"__index", Label___index},
	{"__newindex", Label___newindex},
	{"__gc", Label___gc},
	{"__eq", Label___eq},
	{"__tostring", Label___tostring},
	{NULL, NULL}
};

static int luasrc_Label(lua_State *L)
{
	Label *pLabel = new LLabel(luaL_optpanel(L, 1, VGui_GetClientLuaRootPanel()),
							   luaL_optstring(L, 2, NULL),
							   luaL_optstring(L, 3, ""),
							   L);
	lua_pushlabel(L, pLabel);
	return 1;
}

static const luaL_Reg Label_funcs[] =
{
	{"Label", luasrc_Label},
	{NULL, NULL}
};

/* open Label object */
LUALIB_API int luaopen_vgui_Label(lua_State *L)
{
	luaL_newmetatable(L, "Label");
	luaL_register(L, NULL, Labelmeta);
	lua_pushstring(L, "panel");
	lua_setfield(L, -2, "__type");
	luaL_register(L, "vgui", Label_funcs);
	lua_pop(L, 2);
	return 1;
}
