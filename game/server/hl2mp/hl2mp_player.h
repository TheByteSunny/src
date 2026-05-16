//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef HL2MP_PLAYER_H
#define HL2MP_PLAYER_H
#pragma once

class CHL2MP_Player;

#include "basemultiplayerplayer.h"
#include "hl2_playerlocaldata.h"
#include "hl2_player.h"
#include "simtimer.h"
#include "soundenvelope.h"
#include "hl2mp_player_shared.h"
#include "hl2mp_gamerules.h"
#include "utldict.h"
#ifdef SBPP
#include "hl2mp_playeranimstate.h"
#endif

//=============================================================================
// >> HL2MP_Player
//=============================================================================
class CHL2MPPlayerStateInfo
{
public:
	HL2MPPlayerState m_iPlayerState;
	const char *m_pStateName;

	void (CHL2MP_Player::*pfnEnterState)();	// Init and deinit the state.
	void (CHL2MP_Player::*pfnLeaveState)();

	void (CHL2MP_Player::*pfnPreThink)();	// Do a PreThink() in this state.
};

class CHL2MP_Player : public CHL2_Player
{
public:
	DECLARE_CLASS( CHL2MP_Player, CHL2_Player );

	CHL2MP_Player();
	~CHL2MP_Player( void );
	
	static CHL2MP_Player *CreatePlayer( const char *className, edict_t *ed )
	{
		CHL2MP_Player::s_PlayerEdict = ed;
		return (CHL2MP_Player*)CreateEntityByName( className );
	}

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
#ifdef SBPP
	DECLARE_PREDICTABLE();

	// This passes the event to the client's and server's CHL2MPPlayerAnimState.
	void			DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );
	void			SetupBones( matrix3x4_t *pBoneToWorld, int boneMask );
#endif

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual void PostThink( void );
	virtual void PreThink( void );
	virtual void PlayerDeathThink( void );
#ifndef SBPP
	virtual void SetAnimation( PLAYER_ANIM playerAnim );
#endif
	virtual bool HandleCommand_JoinTeam( int team );
	virtual bool ClientCommand( const CCommand &args );
	virtual void CreateViewModel( int viewmodelindex = 0 );
	virtual bool BecomeRagdollOnClient( const Vector &force );
	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual int OnTakeDamage( const CTakeDamageInfo &inputInfo );
	virtual bool WantsLagCompensationOnEntity( const CBasePlayer *pPlayer, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const;
	virtual void FireBullets ( const FireBulletsInfo_t &info );
	virtual bool Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex = 0);
	virtual bool BumpWeapon( CBaseCombatWeapon *pWeapon );
	virtual void ChangeTeam( int iTeam );
	virtual void PickupObject ( CBaseEntity *pObject, bool bLimitMassAndSize );
#ifndef SBPP
	virtual void PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force );
#endif
	virtual void Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget = NULL, const Vector *pVelocity = NULL );
	virtual void UpdateOnRemove( void );
	virtual void DeathSound( const CTakeDamageInfo &info );
	virtual CBaseEntity* EntSelectSpawnPoint( void );
		
	int FlashlightIsOn( void );
	void FlashlightTurnOn( void );
	void FlashlightTurnOff( void );
#ifndef SBPP
	void	PrecacheFootStepSounds( void );
#endif
	bool	ValidatePlayerModel( const char *pModel );

#ifndef SBPP
	QAngle GetAnimEyeAngles( void ) { return m_angEyeAngles.Get(); }
#endif

	Vector GetAttackSpread( CBaseCombatWeapon *pWeapon, CBaseEntity *pTarget = NULL );
	virtual Vector GetAutoaimVector( float flDelta );

	void CheatImpulseCommands( int iImpulse );
	void CreateRagdollEntity( void );
	void GiveAllItems( void );
	void GiveDefaultItems( void );

	void NoteWeaponFired( void );

#ifdef SBPP
	void SetAnimation( PLAYER_ANIM playerAnim );
#else
	void ResetAnimation( void );
#endif
	void SetPlayerModel( void );
	void SetPlayerTeamModel( void );
	Activity TranslateTeamActivity( Activity ActToTranslate );
	
	float GetNextModelChangeTime( void ) { return m_flNextModelChangeTime; }
	float GetNextTeamChangeTime( void ) { return m_flNextTeamChangeTime; }
	void  PickDefaultSpawnTeam( void );
	void  SetupPlayerSoundsByModel( const char *pModelName );
	const char *GetPlayerModelSoundPrefix( void );
	int	  GetPlayerModelType( void ) { return m_iPlayerSoundType;	}
	
	void  DetonateTripmines( void );

	void Reset();

	bool IsReady();
	void SetReady( bool bReady );

	void CheckChatText( char *p, int bufsize );

	void State_Transition( HL2MPPlayerState newState );
	void State_Enter( HL2MPPlayerState newState );
	void State_Leave();
	void State_PreThink();
	CHL2MPPlayerStateInfo *State_LookupInfo( HL2MPPlayerState state );

	void State_Enter_ACTIVE();
	void State_PreThink_ACTIVE();
	void State_Enter_OBSERVER_MODE();
	void State_PreThink_OBSERVER_MODE();


	virtual bool StartObserverMode( int mode );
	virtual void StopObserverMode( void );


	Vector m_vecTotalBulletForce;	//Accumulator for bullet force in a single frame

	// Tracks our ragdoll entity.
	CNetworkHandle( CBaseEntity, m_hRagdoll );	// networked entity handle 
#ifdef SBPP
	CNetworkVar( float, m_flStartCharge );
	CNetworkVar( float, m_flAmmoStartCharge );
	CNetworkVar( float, m_flPlayAftershock );
	CNetworkVar( float, m_flNextAmmoBurn );
#endif

	virtual bool	CanHearAndReadChatFrom( CBasePlayer *pPlayer );

		
#ifdef SBPP
	CHL2MPPlayerAnimState *GetAnimState() const { return m_PlayerAnimState; }
	virtual void StartTaunt(Activity aDance);
	virtual void EndTaunt();
	virtual void TauntThink();

	virtual bool IsTaunting() const { return m_bTaunting; }

	virtual Activity GetDanceAct() const { return m_aCurrentTaunt; }

	virtual int GetPlayerColorR() const { return m_iPlayerColorR; }
	virtual int GetPlayerColorG() const { return m_iPlayerColorG; }
	virtual int GetPlayerColorB() const { return m_iPlayerColorB; }

	virtual bool IsChatting() const { return m_bIsChatting; }
	virtual bool IsNoclipping() const { return m_bIsNoclipping; }

	/* And whatever this monstrosity is */
	virtual void SetChatting(bool bValue) { m_bIsChatting = bValue; }
	virtual void SetNoclipping(bool bValue) { m_bIsNoclipping = bValue; }

	const char *GetSpecialID() const { return m_szUserID; }
	void SetSpecialID( const char *id ) { Q_strncpy( m_szUserID.GetForModify(), id, sizeof( m_szUserID ) ); }

	virtual void UpdatePlayerColors();

public:
    CUtlString m_CurrentHandModel;
#endif
private:

	CNetworkQAngle( m_angEyeAngles );
#ifdef SBPP
	CHL2MPPlayerAnimState *m_PlayerAnimState;
#else
	CPlayerAnimState   m_PlayerAnimState;
#endif

	int m_iLastWeaponFireUsercmd;
	int m_iModelType;
	CNetworkVar( int, m_iSpawnInterpCounter );
	CNetworkVar( int, m_iPlayerSoundType );

	float m_flNextModelChangeTime;
	float m_flNextTeamChangeTime;
#ifdef SBPP
	CNetworkVar( bool, m_bTaunting );
	CNetworkVar( Activity, m_aCurrentTaunt );

	// player color
	CNetworkVar( int, m_iPlayerColorR );
	CNetworkVar( int, m_iPlayerColorG );
	CNetworkVar( int, m_iPlayerColorB );

	CNetworkVar( bool, m_bIsChatting );
	CNetworkVar( bool, m_bIsNoclipping );

	CNetworkString( m_szUserID, 33 );

	float   m_flTauntEndTime;
	int     m_iTauntSeq;
#endif

	float m_flSlamProtectTime;	

	HL2MPPlayerState m_iPlayerState;
	CHL2MPPlayerStateInfo *m_pCurStateInfo;

	bool ShouldRunRateLimitedCommand( const CCommand &args );

	// This lets us rate limit the commands the players can execute so they don't overflow things like reliable buffers.
	CUtlDict<float,int>	m_RateLimitLastCommandTimes;

    bool m_bEnterObserver;
	bool m_bReady;
};

inline CHL2MP_Player *ToHL2MPPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

	return dynamic_cast<CHL2MP_Player*>( pEntity );
}

#endif //HL2MP_PLAYER_H
