//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef SPAWNMENU_NEW
#define SPAWNMENU_NEW
#ifdef _WIN32
#pragma once
#endif

class SpawnMenu
{
public:
	virtual void Create( vgui::VPANEL parent ) = 0;
	virtual void Destroy( void ) = 0;
	virtual void Activate( void ) = 0;
};

extern SpawnMenu *spawnmenu;

#endif