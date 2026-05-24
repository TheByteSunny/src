//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "basescriptedvehicle.h"
#include "luamanager.h"
#include "iservervehicle.h"
#include "tier0/memdbgon.h"
#include "lbaseplayer_shared.h"
#include "lbaseanimating.h"
#include "limovehelper.h"
#include "lmovedata.h"
#include "mathlib/lvector.h"

BEGIN_DATADESC( CBaseScriptedVehicle )
END_DATADESC()

static CUtlDict< CEntityFactory<CBaseScriptedVehicle>*, unsigned short > m_TriggerFactoryDatabase;

#if defined( LUA_SDK )
static bool PushTableFromRef(lua_State *L, int ref)
{
	if (ref == LUA_NOREF || !L)
		return false;

	lua_getref(L, ref);
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return false;
	}

	return true;
}

static bool GetFieldRemoveTable(lua_State *L, const char *key)
{
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return false;
	}

	lua_getfield(L, -1, key);

	if (lua_isnil(L, -1))
	{
		lua_pop(L, 2);
		return false;
	}

	lua_remove(L, -2);
	return true;
}
#endif

void RegisterScriptedVehicle( const char *className )
{
	if ( EntityFactoryDictionary()->FindFactory( className ) )
		return;

	unsigned short lookup = m_TriggerFactoryDatabase.Find( className );
	if ( lookup != m_TriggerFactoryDatabase.InvalidIndex() )
		return;

	CEntityFactory<CBaseScriptedVehicle> *pFactory = new CEntityFactory<CBaseScriptedVehicle>( className );
	lookup = m_TriggerFactoryDatabase.Insert( className, pFactory );
	Assert( lookup != m_TriggerFactoryDatabase.InvalidIndex() );
}

void ResetVehicleFactoryDatabase( void )
{
	for ( int i = m_TriggerFactoryDatabase.First(); i != m_TriggerFactoryDatabase.InvalidIndex(); i = m_TriggerFactoryDatabase.Next( i ) )
	{
		delete m_TriggerFactoryDatabase[ i ];
	}
	m_TriggerFactoryDatabase.RemoveAll();
}

CBaseScriptedVehicle::CBaseScriptedVehicle()
{
#ifdef LUA_SDK
	m_nTableReference = LUA_NOREF;
#endif
}

CBaseScriptedVehicle::~CBaseScriptedVehicle()
{
#ifdef LUA_SDK
	lua_unref( L, m_nTableReference );
	m_nTableReference = LUA_NOREF;
#endif
}

void CBaseScriptedVehicle::Spawn()
{
	InitScriptedVehicle();
	BaseClass::Spawn();
}

void CBaseScriptedVehicle::InitScriptedVehicle()
{
#if defined( LUA_SDK )
	if ( m_nTableReference != LUA_NOREF )
		return;

	SetThink( &CBaseScriptedVehicle::Think );
	SetNextThink( gpGlobals->curtime );

	char className[255];
	Q_strncpy( className, GetClassname(), sizeof(className) );
	Q_strlower( className );
	SetClassname( className );

	lua_getglobal( L, "entity" );
	if ( lua_istable( L, -1 ) )
	{
		lua_getfield( L, -1, "get" );
		if ( lua_isfunction( L, -1 ) )
		{
			lua_remove( L, -2 );
			lua_pushstring( L, className );
			luasrc_pcall( L, 1, 1, 0 );
		}
		else
		{
			lua_pop( L, 2 );
		}
	}
	else
	{
		lua_pop( L, 1 );
	}

	if ( m_nTableReference == LUA_NOREF )
	{
		LoadScriptedVehicle();
		m_nTableReference = luaL_ref( L, LUA_REGISTRYINDEX );
	}

	// VehicleScript
	if ( PushTableFromRef( L, m_nTableReference ) )
	{
		GetFieldRemoveTable( L, "VehicleScript" );
		if ( lua_isstring( L, -1 ) )
			m_vehicleScript = AllocPooledString( lua_tostring( L, -1 ) );
		else
			m_vehicleScript = AllocPooledString( "scripts/vehicles/jeep_test.txt" );
		lua_pop( L, 1 );
	}
	else
	{
		m_vehicleScript = AllocPooledString( "scripts/vehicles/jeep_test.txt" );
	}

	// ModelName
	if ( PushTableFromRef( L, m_nTableReference ) )
	{
		GetFieldRemoveTable( L, "ModelName" );
		if ( lua_isstring( L, -1 ) )
		{
			PrecacheModel( lua_tostring( L, -1 ) );
			SetModel( lua_tostring( L, -1 ) );
		}
		else
		{
			PrecacheModel( "models/buggy.mdl" );
			SetModel( "models/buggy.mdl" );
		}
		lua_pop( L, 1 );
	}
	else
	{
		PrecacheModel( "models/buggy.mdl" );
		SetModel( "models/buggy.mdl" );
	}

	m_bHasGun = false;
	m_bLocked = false;

	BEGIN_LUA_CALL_ENTITY_METHOD( "Initialize" );
	END_LUA_CALL_ENTITY_METHOD( 0, 0 );
#endif
}

void CBaseScriptedVehicle::ExitVehicle( int nRole )
{
	BaseClass::ExitVehicle( nRole );

#ifdef LUA_SDK
	BEGIN_LUA_CALL_ENTITY_METHOD( "ExitVehicle" );
		lua_pushinteger( L, nRole );
	END_LUA_CALL_ENTITY_METHOD( 1, 0 );
#endif
}

void CBaseScriptedVehicle::SetupMove( CBasePlayer *pPlayer, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move )
{
	BaseClass::SetupMove( pPlayer, ucmd, pHelper, move );

#ifdef LUA_SDK
	BEGIN_LUA_CALL_ENTITY_METHOD( "SetupMove" );
		lua_pushplayer( L, pPlayer );
		lua_pushmovehelper( L, pHelper );
		lua_pushmovedata( L, move );
	END_LUA_CALL_ENTITY_METHOD( 3, 0 );
#endif
}

void CBaseScriptedVehicle::ProcessMovement( CBasePlayer *pPlayer, CMoveData *move )
{
	BaseClass::ProcessMovement( pPlayer, move );

#ifdef LUA_SDK
	BEGIN_LUA_CALL_ENTITY_METHOD( "ProcessMovement" );
		lua_pushplayer( L, pPlayer );
		lua_pushmovedata( L, move );
	END_LUA_CALL_ENTITY_METHOD( 2, 0 );
#endif
}

void CBaseScriptedVehicle::FinishMove( CBasePlayer *pPlayer, CUserCmd *ucmd, CMoveData *move )
{
	BaseClass::FinishMove( pPlayer, ucmd, move );

#ifdef LUA_SDK
	BEGIN_LUA_CALL_ENTITY_METHOD( "FinishMove" );
		lua_pushplayer( L, pPlayer );
		lua_pushmovedata( L, move );
	END_LUA_CALL_ENTITY_METHOD( 2, 0 );
#endif
}

void CBaseScriptedVehicle::CreateServerVehicle( void )
{
	m_pServerVehicle = new CScriptedServerVehicle();
	m_pServerVehicle->SetVehicle( this );
}

void CBaseScriptedVehicle::LoadScriptedVehicle()
{
	lua_getglobal( L, "entity" );
	if ( lua_istable( L, -1 ) )
	{
		lua_getfield( L, -1, "get" );
		if ( lua_isfunction( L, -1 ) )
		{
			lua_remove( L, -2 );
			lua_pushstring( L, GetClassname() );
			luasrc_pcall( L, 1, 1, 0 );
		}
		else
		{
			lua_pop( L, 2 );
		}
	}
	else
	{
		lua_pop( L, 1 );
	}
}

void CBaseScriptedVehicle::Think()
{
#ifdef LUA_SDK
	BEGIN_LUA_CALL_ENTITY_METHOD( "Think" );
	END_LUA_CALL_ENTITY_METHOD( 0, 0 );
#endif

	BaseClass::Think();
}
