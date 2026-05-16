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

#ifndef SBPP
class C_HL2MP_Player;
#endif
#include "c_basehlplayer.h"
#include "hl2mp_player_shared.h"
#include "beamdraw.h"
#ifdef SBPP
#include "hl2mp_playeranimstate.h"
#endif

//=============================================================================
// >> HL2MP_Player
//=============================================================================
class C_HL2MP_Player : public C_BaseHLPlayer
{
public:
	DECLARE_CLASS( C_HL2MP_Player, C_BaseHLPlayer );

	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();


	C_HL2MP_Player();
	~C_HL2MP_Player( void );

	void ClientThink( void );

	static C_HL2MP_Player* GetLocalHL2MPPlayer();
	
	virtual int DrawModel( int flags );
	virtual void AddEntity( void );

	QAngle GetAnimEyeAngles( void ) { return m_angEyeAngles; }
	Vector GetAttackSpread( CBaseCombatWeapon *pWeapon, CBaseEntity *pTarget = NULL );


	// Should this object cast shadows?
	virtual ShadowType_t		ShadowCastType( void );
	virtual C_BaseAnimating *BecomeRagdollOnClient();
	virtual const QAngle& GetRenderAngles();
	virtual bool ShouldDraw( void );
	virtual void OnDataChanged( DataUpdateType_t type );
	virtual float GetFOV( void );
	virtual CStudioHdr *OnNewModel( void );
	virtual void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );
	virtual void ItemPreFrame( void );
	virtual void ItemPostFrame( void );
	virtual float GetMinFOV()	const { return 5.0f; }
	virtual Vector GetAutoaimVector( float flDelta );
	virtual void NotifyShouldTransmit( ShouldTransmitState_t state );
	virtual void CreateLightEffects( void ) {}
	virtual bool ShouldReceiveProjectedTextures( int flags );
	virtual void PostDataUpdate( DataUpdateType_t updateType );
#ifndef SBPP
	virtual void PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force );
#endif
	virtual void PreThink( void );
	virtual void DoImpactEffect( trace_t &tr, int nDamageType );
	IRagdoll* GetRepresentativeRagdoll() const;
	virtual void CalcView( Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov );
	virtual const QAngle& EyeAngles( void );

	
	bool	CanSprint( void );
	void	StartSprinting( void );
	void	StopSprinting( void );
	void	HandleSpeedChanges( void );
	void	UpdateLookAt( void );
	void	Initialize( void );
	int		GetIDTarget() const;
	void	UpdateIDTarget( void );
#ifndef SBPP
	void	PrecacheFootStepSounds( void );
#endif
	const char	*GetPlayerModelSoundPrefix( void );

	HL2MPPlayerState State_Get() const;

	// Walking
	void StartWalking( void );
	void StopWalking( void );
	bool IsWalking( void ) { return m_fIsWalking; }

#ifndef SBPP
	virtual void PostThink( void );
#else
	virtual void					UpdateClientSideAnimation();
	void DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );
	virtual void CalculateIKLocks( float currentTime );

	CNetworkVar( float, m_flStartCharge );
	CNetworkVar( float, m_flAmmoStartCharge );
	CNetworkVar( float, m_flPlayAftershock );
	CNetworkVar( float, m_flNextAmmoBurn );

	CHL2MPPlayerAnimState *GetAnimState() const { return m_PlayerAnimState; }
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
#endif

private:
	
	C_HL2MP_Player( const C_HL2MP_Player & );

#ifdef SBPP
	CHL2MPPlayerAnimState *m_PlayerAnimState;
#else
	CPlayerAnimState m_PlayerAnimState;
#endif

	QAngle	m_angEyeAngles;

	CInterpolatedVar< QAngle >	m_iv_angEyeAngles;

	EHANDLE	m_hRagdoll;

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
#endif
	int	m_headYawPoseParam;
	int	m_headPitchPoseParam;
	float m_headYawMin;
	float m_headYawMax;
	float m_headPitchMin;
	float m_headPitchMax;

	bool m_isInit;
	Vector m_vLookAtTarget;

	float m_flLastBodyYaw;
	float m_flCurrentHeadYaw;
	float m_flCurrentHeadPitch;

	int	  m_iIDEntIndex;

	CountdownTimer m_blinkTimer;

	int	  m_iSpawnInterpCounter;
	int	  m_iSpawnInterpCounterCache;

#ifdef SBPP // workaround for c_hands
public:
#endif
	int	  m_iPlayerSoundType;
#ifdef SBPP
private:
#endif

	void ReleaseFlashlight( void );
	Beam_t	*m_pFlashlightBeam;

	CNetworkVar( HL2MPPlayerState, m_iPlayerState );	

	bool m_fIsWalking;
};

inline C_HL2MP_Player *ToHL2MPPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

	return dynamic_cast<C_HL2MP_Player*>( pEntity );
}


class C_HL2MPRagdoll : public C_BaseAnimatingOverlay
{
public:
	DECLARE_CLASS( C_HL2MPRagdoll, C_BaseAnimatingOverlay );
	DECLARE_CLIENTCLASS();
	
	C_HL2MPRagdoll();
	~C_HL2MPRagdoll();

	virtual void OnDataChanged( DataUpdateType_t type );

	int GetPlayerEntIndex() const;
	IRagdoll* GetIRagdoll() const;

	void ImpactTrace( trace_t *pTrace, int iDamageType, const char *pCustomImpactName );
	void UpdateOnRemove( void );
	virtual void SetupWeights( const matrix3x4_t *pBoneToWorld, int nFlexWeightCount, float *pFlexWeights, float *pFlexDelayedWeights );
	
private:
	
	C_HL2MPRagdoll( const C_HL2MPRagdoll & ) {}

	void Interp_Copy( C_BaseAnimatingOverlay *pDestinationEntity );
	void CreateHL2MPRagdoll( void );

#ifndef SBPP
private:
#else
public:
#endif

	EHANDLE	m_hPlayer;

#ifdef SBPP
private:
#endif
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
};

#endif //HL2MP_PLAYER_H
