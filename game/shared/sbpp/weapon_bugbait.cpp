//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_hl2mp_player.h"
#else
#include "hl2mp_player.h"
#include "grenade_bugbait.h"
#include "antlion_maker.h"
#include "gamestats.h"
#endif

#include "weapon_hl2mpbasehlmpcombatweapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define CWeaponBugbait C_WeaponBugbait
#endif

//-----------------------------------------------------------------------------
// CWeaponBugbait
//-----------------------------------------------------------------------------

class CWeaponBugbait : public CBaseHL2MPCombatWeapon
{
	DECLARE_CLASS( CWeaponBugbait, CBaseHL2MPCombatWeapon );

public:
	CWeaponBugbait( void );

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	void Precache( void );
	void ItemPostFrame( void );
	void PrimaryAttack( void );
	void SecondaryAttack( void );

	bool Deploy( void );
	bool Holster( CBaseCombatWeapon *pSwitchingTo );

	void OnPickedUp( CBaseCombatCharacter *pNewOwner );
#ifndef CLIENT_DLL
	void Spawn( void );
	void FallInit( void );
	void Drop( const Vector &vecVelocity );
	void BugbaitStickyTouch( CBaseEntity *pOther );

	void ThrowGrenade( CBasePlayer *pPlayer );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
#endif

	virtual bool Reload( void );

	DECLARE_DATADESC();
	DECLARE_ACTTABLE();

protected:
	CNetworkVar( bool, m_bDrawBackFinished );
	CNetworkVar( bool, m_bRedraw );
	CNetworkVar( bool, m_bEmitSpores );
	EHANDLE m_hSporeTrail;

private:
	CWeaponBugbait( const CWeaponBugbait & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponBugbait, DT_WeaponBugbait )

BEGIN_NETWORK_TABLE( CWeaponBugbait, DT_WeaponBugbait )
#ifdef CLIENT_DLL
RecvPropTime( RECVINFO( m_bRedraw ) ), RecvPropFloat( RECVINFO( m_bEmitSpores ) ), RecvPropInt( RECVINFO( m_bDrawBackFinished ) ),
#else
SendPropTime( SENDINFO( m_bRedraw ) ), SendPropFloat( SENDINFO( m_bEmitSpores ) ), SendPropInt( SENDINFO( m_bDrawBackFinished ) ),
#endif
	END_NETWORK_TABLE()

		BEGIN_PREDICTION_DATA( CWeaponBugbait ) END_PREDICTION_DATA()

			BEGIN_DATADESC( CWeaponBugbait )

				DEFINE_FIELD( m_hSporeTrail, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bRedraw, FIELD_BOOLEAN ), DEFINE_FIELD( m_bEmitSpores, FIELD_BOOLEAN ), DEFINE_FIELD( m_bDrawBackFinished, FIELD_BOOLEAN ),

#ifndef CLIENT_DLL
	DEFINE_FUNCTION( BugbaitStickyTouch ),
#endif

END_DATADESC
()

	LINK_ENTITY_TO_CLASS( weapon_bugbait, CWeaponBugbait );
PRECACHE_WEAPON_REGISTER( weapon_bugbait );

acttable_t CWeaponBugbait::m_acttable[] = {
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, true },

	{ ACT_MP_STAND_IDLE, ACT_HL2MP_IDLE_GRENADE, false },
	{ ACT_MP_CROUCH_IDLE, ACT_HL2MP_IDLE_CROUCH_GRENADE, false },

	{ ACT_MP_RUN, ACT_HL2MP_RUN_GRENADE, false },
	{ ACT_MP_CROUCHWALK, ACT_HL2MP_WALK_CROUCH_GRENADE, false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE, ACT_HL2MP_GESTURE_RANGE_ATTACK_GRENADE, false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE, ACT_HL2MP_GESTURE_RANGE_ATTACK_GRENADE, false },

	{ ACT_MP_RELOAD_STAND, ACT_HL2MP_GESTURE_RELOAD_GRENADE, false },
	{ ACT_MP_RELOAD_CROUCH, ACT_HL2MP_GESTURE_RELOAD_GRENADE, false },

	{ ACT_MP_JUMP, ACT_HL2MP_JUMP_GRENADE, false },

	{ ACT_MP_SWIM_IDLE, ACT_HL2MP_SWIM_IDLE_GRENADE, false },
	{ ACT_MP_SWIM, ACT_HL2MP_SWIM_GRENADE, false },
};

IMPLEMENT_ACTTABLE( CWeaponBugbait );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponBugbait::CWeaponBugbait( void )
{
	m_fMinRange1 = 24;
	m_fMaxRange1 = 1500;
	m_fMinRange2 = 24;
	m_fMaxRange2 = 200;

	m_bDrawBackFinished = false;
	m_bRedraw = false;
	m_hSporeTrail = nullptr;

	m_bFiresUnderwater = true;
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponBugbait::Spawn( void )
{
	BaseClass::Spawn();

	// Increase the bugbait's pickup volume. It spawns inside the antlion guard's body,
	// and playtesters seem to be wary about moving into the body.
	SetSize( Vector( -4, -4, -4 ), Vector( 4, 4, 4 ) );
	CollisionProp()->UseTriggerBounds( true, 100 );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponBugbait::FallInit( void )
{
	// Bugbait shouldn't be physics, because it musn't roll/move away from it's spawnpoint.
	// The game will break if the player can't pick it up, so it must stay still.
	SetModel( GetWorldModel() );

	VPhysicsDestroyObject();
	SetMoveType( MOVETYPE_FLYGRAVITY );
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_TRIGGER );

	SetPickupTouch();

	SetThink( &CBaseCombatWeapon::FallThink );

	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponBugbait::Drop( const Vector &vecVelocity )
{
	BaseClass::Drop( vecVelocity );

	// On touch, stick & stop moving. Increase our thinktime a bit so we don't stomp the touch for a bit
	SetNextThink( gpGlobals->curtime + 3.0 );
	SetTouch( &CWeaponBugbait::BugbaitStickyTouch );

	m_hSporeTrail = SporeExplosion::CreateSporeExplosion();
	if ( m_hSporeTrail )
	{
		SporeExplosion *pSporeExplosion = (SporeExplosion *)m_hSporeTrail.Get();

		QAngle angles;
		VectorAngles( Vector( 0, 0, 1 ), angles );

		pSporeExplosion->SetAbsAngles( angles );
		pSporeExplosion->SetAbsOrigin( GetAbsOrigin() );
		pSporeExplosion->SetParent( this );

		pSporeExplosion->m_flSpawnRate = 16.0f;
		pSporeExplosion->m_flParticleLifetime = 0.5f;
		pSporeExplosion->SetRenderColor( 0.0f, 0.5f, 0.25f, 0.15f );

		pSporeExplosion->m_flStartSize = 32;
		pSporeExplosion->m_flEndSize = 48;
		pSporeExplosion->m_flSpawnRadius = 4;

		pSporeExplosion->SetLifetime( 9999 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Stick to the world when we touch it
//-----------------------------------------------------------------------------
void CWeaponBugbait::BugbaitStickyTouch( CBaseEntity *pOther )
{
	if ( !pOther->IsWorld() )
		return;

	// Stop moving, wait for pickup
	SetMoveType( MOVETYPE_NONE );
	SetThink( NULL );
	SetPickupTouch();
}
#endif

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponBugbait::Precache( void )
{
	BaseClass::Precache();

	//	UTIL_PrecacheOther( "npc_grenade_bugbait" );
	PrecacheScriptSound( "Weapon_Bugbait.Splat" );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *pPicker -
//-----------------------------------------------------------------------------
void CWeaponBugbait::OnPickedUp( CBaseCombatCharacter *pNewOwner )
{
	BaseClass::OnPickedUp( pNewOwner );

#ifndef CLIENT_DLL
	if ( m_hSporeTrail )
		UTIL_Remove( m_hSporeTrail );
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponBugbait::PrimaryAttack( void )
{
	if ( m_bRedraw )
		return;

	CBaseCombatCharacter *pOwner = GetOwner();

	if ( pOwner == NULL )
		return;

	CHL2MP_Player *pPlayer = ToHL2MPPlayer( GetOwner() );

	if ( pPlayer == NULL )
		return;

	SendWeaponAnim( ACT_VM_HAULBACK );

	m_flTimeWeaponIdle = FLT_MAX;
	m_flNextPrimaryAttack = FLT_MAX;

#ifndef CLIENT_DLL
	gamestats->Event_WeaponFired( pPlayer, true, GetClassname() );
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponBugbait::SecondaryAttack( void )
{
	// Squeeze!
	CPASAttenuationFilter filter( this );

	EmitSound( filter, entindex(), "Weapon_Bugbait.Splat" );

	// Moon: serverside only crap, haha
#ifndef CLIENT_DLL
	if ( CGrenadeBugBait::ActivateBugbaitTargets( GetOwner(), GetAbsOrigin(), true ) == false )
	{
		g_AntlionMakerManager.BroadcastFollowGoal( GetOwner() );
	}
#endif

	SendWeaponAnim( ACT_VM_SECONDARYATTACK );
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

#ifndef CLIENT_DLL
	CHL2MP_Player *pOwner = ToHL2MPPlayer( GetOwner() );
	if ( pOwner )
		gamestats->Event_WeaponFired( pOwner, false, GetClassname() );
#endif
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose:
// Input  : *pPlayer -
//-----------------------------------------------------------------------------
void CWeaponBugbait::ThrowGrenade( CBasePlayer *pPlayer )
{
	Vector vForward, vRight, vUp, vThrowPos, vThrowVel;

	pPlayer->EyeVectors( &vForward, &vRight, &vUp );

	vThrowPos = pPlayer->EyePosition();

	vThrowPos += vForward * 18.0f;
	vThrowPos += vRight * 12.0f;

	pPlayer->GetVelocity( &vThrowVel, NULL );
	vThrowVel += vForward * 1000;

	CGrenadeBugBait *pGrenade = BugBaitGrenade_Create( vThrowPos, vec3_angle, vThrowVel, QAngle( 600, random->RandomInt( -1200, 1200 ), 0 ), pPlayer );

	if ( pGrenade != NULL )
	{
		// If the shot is clear to the player, give the missile a grace period
		trace_t tr;
		UTIL_TraceLine( pPlayer->EyePosition(), pPlayer->EyePosition() + ( vForward * 128 ), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

		if ( tr.fraction == 1.0 )
		{
			pGrenade->SetGracePeriod( 0.1f );
		}
	}

	m_bRedraw = true;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *pEvent -
//			*pOperator -
//-----------------------------------------------------------------------------
void CWeaponBugbait::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	switch ( pEvent->event )
	{
	case EVENT_WEAPON_SEQUENCE_FINISHED:
		m_bDrawBackFinished = true;
		break;

	case EVENT_WEAPON_THROW:
		ThrowGrenade( pOwner );
		break;

	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Allows firing as fast as button is pressed
//-----------------------------------------------------------------------------
void CWeaponBugbait::ItemPostFrame( void )
{
	CHL2MP_Player *pOwner = ToHL2MPPlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	// See if we're cocked and ready to throw
	if ( m_bDrawBackFinished )
	{
		if ( ( pOwner->m_nButtons & IN_ATTACK ) == false )
		{
			SendWeaponAnim( ACT_VM_THROW );
			m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
			m_bDrawBackFinished = false;

			CHL2MP_Player *pPlayer = ToHL2MPPlayer( ToBasePlayer( pOwner ) );
			if ( pPlayer )
			{
				pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
			}
		}
	}
	else
	{
		//See if we're attacking
		if ( ( pOwner->m_nButtons & IN_ATTACK ) && ( m_flNextPrimaryAttack < gpGlobals->curtime ) )
		{
			PrimaryAttack();
		}
		else if ( ( pOwner->m_nButtons & IN_ATTACK2 ) && ( m_flNextSecondaryAttack < gpGlobals->curtime ) )
		{
			SecondaryAttack();
		}
	}

	if ( m_bRedraw )
	{
		if ( IsViewModelSequenceFinished() )
		{
			Reload();
		}
	}

	WeaponIdle();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CWeaponBugbait::Deploy( void )
{
	CHL2MP_Player *pOwner = ToHL2MPPlayer( GetOwner() );
	if ( pOwner == NULL )
		return false;

	m_bRedraw = false;
	m_bDrawBackFinished = false;

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CWeaponBugbait::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_bRedraw = false;
	m_bDrawBackFinished = false;

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose:
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponBugbait::Reload( void )
{
	if ( ( m_bRedraw ) && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) )
	{
		//Redraw the weapon
		SendWeaponAnim( ACT_VM_DRAW );

		//Update our times
		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();

		//Mark this as done
		m_bRedraw = false;
	}

	return true;
}