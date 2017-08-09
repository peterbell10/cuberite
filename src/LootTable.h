

#include "BasicVariant.h"


// fwd:
class cItem;
class cItems;




namespace LootTable
{
	template <class T>
	struct sRange
	{
		T Min, Max;

		template <class RNG>
		T GetValue(RNG & a_RNG) const
		{
			if (Min == Max)
			{
				return Min;
			}
			else
			{
				return Min + a_RNG() % (Max - Min + 1);
			}
		}
	};

	struct sLootGenInfo;

	class cCondition
	{
	public:
		enum class eEntity
		{
			This,
			Killer,
			KillerPlayer
		};

		struct sEntityProperties
		{
			eEntity Entity;
			bool OnFire;
		};

		struct sEntityScores
		{
			eEntity Entity;
			sRange<int> Score;
		};

		struct sKilledByPlayer
		{
			bool Inverse;
		};

		struct sRandomChance
		{
			float Chance;
		};

		struct sRandomChanceWithLooting
		{
			float Chance;
			float LootingMultiplier;
		};

		bool IsSatisfied(sLootGenInfo & a_Info) const;

	private:
		cBasicVariant<
			sEntityProperties,
			sEntityScores,
			sKilledByPlayer,
			sRandomChance,
			sRandomChanceWithLooting
		> m_ConditionData;
	};
	
	class cFunction
	{
	public:

		void operator() (cItem & a_Item, sLootGenInfo & a_Info) const;

	private:
		
		// Enchants the item with one randomly-selected enchantment.
		struct sEnchantRandomly
		{
			std::vector<cEnchantments::eEnchantment> Enchantments;
		};

		// Enchants the item, with the specified enchantment level as if in an enchanting table.
		struct sEnchantWithLevels
		{
			bool Treasure;
			sRange<int> Levels;
		};

		// Smelt the item as ig in a furnace
		struct sFurnaceSmelt
		{
		};

		// Adjusts the stack size based on the level of the Looting enchantment on the killer entity.
		struct sLootingEnchant
		{
			sRange<int> ItemsPerLevel;
		};

		// Add attribute modifiers to the item.
		struct sSetAttributes
		{
			// Cuberite doesn't support modifiers yet
		};

		// Sets the stack size.
		struct sSetCount
		{
			sRange<int> ItemCount;
		};

		// Sets the item's damage for tools as a fraction. 
		struct sSetDamage
		{
			sRange<float> DamageFraction;
		};

		// Sets the item data value.
		struct sSetData
		{
			sRange<int> Data;
		};

		// Set item NBT data
		struct sSetNBT
		{
			// Cuberite doesn't support items with NBT.
		};

		std::vector<cCondition> m_Conditions;
		cBasicVariant<
			sEnchantRandomly,
			sEnchantWithLevels,
			sFurnaceSmelt,
			sLootingEnchant,
			sSetAttributes,
			sSetCount,
			sSetDamage,
			sSetData,
			sSetNBT
		> m_FunctionData;
	};

	class cLootTable;

	class cEntry
	{
	public:
		bool IsSatisfied(sLootGenInfo & a_Info) const;

		void AddDrops(cItems & a_Items, sLootGenInfo & a_Info) const;

	private:

		struct sItemDrop
		{
			/** ID of item to drop. */
			ENUM_ITEM_ID ID;
			/** Functions to modify the item. */
			std::vector<cFunction> m_Functions;
		};
		
		std::vector<cCondition> m_Conditions;
		cBasicVariant<
			sItemDrop,
			cLootTable &
		> m_EntryData;
		int m_Weight;
		int m_Quality;
	};

	class cPool
	{
	public:
		bool IsSatisfied(sLootGenInfo & a_Info) const;

		void AddDrops(cItems & a_Items, sLootGenInfo & a_Info) const;

	private:

		std::vector<cCondition> m_Conditions;
		sRange<int> m_Rolls;
		sRange<float> m_BonusRolls;
		std::vector<cEntry> m_Entries;
		std::vector<int>    m_Weights;
		std::vector<int>    m_Qualities;
	};

	class cLootTable
	{
	public:
		void AddDrops(cItems & a_Items, sLootGenInfo & a_Info) const;

	private:
		std::vector<cPool> m_Pools;

	};
}

