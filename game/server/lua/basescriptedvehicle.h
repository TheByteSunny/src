//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef BASESCRIPTEDVEHICLE_H
#define BASESCRIPTEDVEHICLE_H
#ifdef _WIN32
#pragma once
#endif

#include "iservervehicle.h"
#include "luamanager.h"
#include "lbaseentity_shared.h"
#include "vehicle_jeep.h"

class CScriptedServerVehicle : public CFourWheelServerVehicle
{
	DECLARE_CLASS( CFourWheelServerVehicle, CFourWheelServerVehicle );
};

class CBaseScriptedVehicle : public CPropJeep
{
	DECLARE_CLASS( CBaseScriptedVehicle, CPropJeep );
	DECLARE_DATADESC();

public:
	CBaseScriptedVehicle();
	~CBaseScriptedVehicle();

	virtual void Spawn();
	virtual void Think();

	virtual void	CreateServerVehicle( void );

	virtual void ExitVehicle( int nRole );
	virtual void SetupMove( CBasePlayer *pPlayer, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move );
	virtual void ProcessMovement( CBasePlayer *pPlayer, CMoveData *move );
	virtual void FinishMove( CBasePlayer *player, CUserCmd *ucmd, CMoveData *move );

	void InitScriptedVehicle();
	void LoadScriptedVehicle();

#ifdef LUA_SDK
	int m_nTableReference;
#endif
};

void RegisterScriptedVehicle( const char *className );
void ResetVehicleFactoryDatabase( void );

#endif