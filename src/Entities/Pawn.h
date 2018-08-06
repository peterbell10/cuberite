
#pragma once

#include "Entity.h"
#include "EntityEffect.h"

// fwd cMonster
class cMonster;





// tolua_begin
class cPawn :
	public cEntity
{
	// tolua_end
	typedef cEntity super;

public:
	CLASS_PROTODEF(cPawn)

	cPawn(eEntityType a_EntityType, double a_Width, double a_Height);
	virtual ~cPawn() override;
	virtual void Destroyed() override;

	virtual void Tick(std::chrono::milliseconds a_Dt, cChunk & a_Chunk) override;
	virtual void KilledBy(TakeDamageInfo & a_TDI) override;

	virtual bool IsFireproof(void) const override;
	virtual bool IsInvisible() const override;
	virtual void HandleAir(void) override;
	virtual void HandleFalling(void);

	// tolua_begin

	/** Applies an entity effect.
	Checks with plugins if they allow the addition.
	a_EffectIntensity is the level of the effect (0 = Potion I, 1 = Potion II, etc).
	a_DistanceModifier is the scalar multiplied to the potion duration (only applies to splash potions).
	*/
	void AddEntityEffect(cEntityEffect::eType a_EffectType, int a_EffectDurationTicks, short a_EffectIntensity, double a_DistanceModifier = 1);

	/** Removes a currently applied entity effect. */
	void RemoveEntityEffect(cEntityEffect::eType a_EffectType);

	/** Returns true, if the entity effect is currently applied. */
	bool HasEntityEffect(cEntityEffect::eType a_EffectType) const;

	/** Removes all currently applied entity effects (used when drinking milk) */
	void ClearEntityEffects(void);

	// tolua_end

	/** Returns all entity effects */
	std::map<cEntityEffect::eType, cEntityEffect *> GetEntityEffects();

	/** Returns the entity effect, if it is currently applied or nullptr if not. */
	cEntityEffect * GetEntityEffect(cEntityEffect::eType a_EffectType);

protected:
	typedef std::map<cEntityEffect::eType, std::unique_ptr<cEntityEffect>> tEffectMap;
	tEffectMap m_EntityEffects;

	double m_LastGroundHeight;
	bool m_bTouchGround;

	virtual void ResetPosition(Vector3d a_NewPosition) override;

} ;  // tolua_export




