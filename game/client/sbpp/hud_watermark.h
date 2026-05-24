//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef HUD_WATERMARK
#define HUD_WATERMARK
#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include "vgui_controls/Panel.h"

class CHudWatermark : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudWatermark, vgui::Panel );

public:
	CHudWatermark( const char *pElementName );

protected:
	virtual void Paint() OVERRIDE;

private:
	vgui::Label *m_pGameName;
	vgui::Label *m_pDiscord;
};

#endif