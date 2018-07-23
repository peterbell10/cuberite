#include "Globals.h"  // NOTE: MSVC stupidness requires this to be the same across all modules

#include "Chicken.h"
#include "../World.h"



cChicken::cChicken(void) :
	super(mtChicken, "entity.chicken.hurt", "entity.chicken.death", 0.3, 0.4)
{
	SetGravity(-2.0f);
	SetAirDrag(0.0f);
	m_EMPersonality = PASSIVE;
	m_BehaviorBreeder.AttachToMonster(*this);
	m_BehaviorCoward.AttachToMonster(*this);
	m_BehaviorItemFollower.AttachToMonster(*this);
	m_BehaviorWanderer.AttachToMonster(*this);
	m_BehaviorItemDropper.AttachToMonster(*this);
	GetMonsterConfig("Chicken");
}





void cChicken::GetDrops(cItems & a_Drops, cEntity * a_Killer)
{
	if (IsBaby())
	{
		return;  // Babies don't drop items
	}

	unsigned int LootingLevel = 0;
	if (a_Killer != nullptr)
	{
		LootingLevel = a_Killer->GetEquippedWeapon().m_Enchantments.GetLevel(cEnchantments::enchLooting);
	}
	AddRandomDropItem(a_Drops, 0, 2 + LootingLevel, E_ITEM_FEATHER);
	AddRandomDropItem(a_Drops, 1, 1, IsOnFire() ? E_ITEM_COOKED_CHICKEN : E_ITEM_RAW_CHICKEN);
}





bool cChicken::DoTakeDamage(TakeDamageInfo & a_TDI)
{
	if (a_TDI.DamageType == dtFalling)
	{
		return false;
	}

	return super::DoTakeDamage(a_TDI);
}
