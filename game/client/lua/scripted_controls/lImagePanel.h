//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef LIMAGEPANEL_H
#define LIMAGEPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/ImagePanel.h>

namespace vgui
{

//-------------------------------------------------------------------------
// Purpose:
//-------------------------------------------------------------------------
class LImagePanel : public ImagePanel
{
	DECLARE_CLASS_SIMPLE( LImagePanel, ImagePanel );

public:
	LImagePanel(Panel *parent, const char *panelName, lua_State *L);
	virtual ~LImagePanel();

public:
#if defined( LUA_SDK )
	lua_State *m_lua_State;
	int m_nTableReference;
	int m_nRefCount;
#endif
};

} // namespace vgui

/* type for ImagePanel functions */
typedef vgui::ImagePanel lua_ImagePanel;



/*
** access functions (stack -> C)
*/

LUA_API lua_ImagePanel *(lua_toimagepanel) (lua_State *L, int idx);



/*
** push functions (C -> stack)
*/

LUA_API void (lua_pushimagepanel) (lua_State *L, lua_ImagePanel *pImagePanel);

LUALIB_API lua_ImagePanel *(luaL_checkimagepanel) (lua_State *L, int narg);

#endif // LIMAGEPANEL_H
