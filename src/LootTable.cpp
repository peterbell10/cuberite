

#include "Globals.h"

#include "LootTable.h"
#include "Item.h"
#include "FurnaceRecipe.h"
#include "Root.h"

#include <random>

namespace
{
	template <class ... Funcs>
	struct sOverload:
		public Funcs...
	{
		sOverload(Funcs ... a_Funcs):
			Funcs(a_Funcs)...
		{
		}
	};

	/** Utility to overload function objects. */
	template <class ... Funcs>
	auto MakeOverloaded(Funcs ... a_Funcs)
	{
		return sOverload<Funcs...>(a_Funcs...);
	}
}



namespace LootTable
{
	struct sLootGenInfo
	{
		std::mt19937 Rand;
		float Luck;
	};


	bool cCondition::IsSatisfied(sLootGenInfo & a_Info) const
	{
		bool res = false;

		auto Visitor = MakeOverloaded(
			[&](const sEntityProperties & a_Properties)
			{
				
			},
			[&](const sEntityScores & a_Scores)
			{
				
			},
			[&](const sKilledByPlayer & a_KilledByPlayer)
			{
				
			},
			[&](const sRandomChance & a_RandomChance)
			{
				auto CutOff = FloorC<std::mt19937::result_type>(a_RandomChance.Chance * a_Info.Rand.max());
				res = a_Info.Rand() < CutOff;
			},
			[&](const sRandomChanceWithLooting & a_RandomChanceWithLooting)
			{
				
			}
		);

		m_ConditionData.Visit(Visitor);

		return res;
	}





	void cFunction::operator() (cItem & a_Item, sLootGenInfo & a_Info) const
	{
		auto Visitor = MakeOverloaded(
			[&](const sEnchantRandomly & a_Enchant)
			{			
				//FIXME
				cWeightedEnchantments Enchants;
				if (a_Enchant.Enchantments.empty())
				{
					a_Item.m_Enchantments.AddItemEnchantmentWeights(Enchants, a_Item.m_ItemType, 30);
					for (auto & Enchant : Enchants)
					{
						Enchant.m_Weight = 1;
					}
				}
				else
				{
					for (auto & Enchant : a_Enchant.Enchantments)
					{
						Enchants.emplace_back(1, Enchant);
					}
				}
				a_Item.m_Enchantments.AddItemEnchantmentWeights
			},
			[&](const sEnchantWithLevels & a_EnchantLevels)
			{
				a_Item.EnchantByXPLevels(a_EnchantLevels.Levels.GetValue(a_Info.Rand));
			},
			[&](sFurnaceSmelt)
			{
				auto Recipe = cRoot::Get()->GetFurnaceRecipe()->GetRecipeFrom(a_Item);
				if (Recipe == nullptr)
				{
					return;
				}
				auto Products = Recipe->Out->m_ItemCount * (a_Item.m_ItemCount / Recipe->In->m_ItemCount);
				a_Item = *Recipe->Out;
				a_Item.m_ItemCount = Products;
			},
			[&](const sLootingEnchant & a_Looting)
			{
				a_Item.AddCount(a_Looting.ItemsPerLevel.GetValue(a_Info.Rand));
			},
			[&](const sSetCount & a_SetCount)
			{
				a_Item.m_ItemCount = a_SetCount.ItemCount.GetValue(a_Info.Rand);
			},
			[&](const sSetDamage & a_SetDamage)
			{
				auto MaxDamage = a_Item.GetMaxDamage();
				auto Fraction = a_SetDamage.DamageFraction.GetValue(a_Info.Rand);
				a_Item.m_ItemDamage = FloorC<short>(MaxDamage * Fraction);
			},
			[&](const sSetData & a_SetData)
			{
				a_Item.m_ItemDamage = a_SetData.Data.GetValue(a_Info.Rand);
			},
			// Not supported yet
			[&](sSetAttributes){},
			[&](sSetNBT){}
		);

		m_FunctionData.Visit(Visitor);
	}
	
	
	
	
	
	bool cEntry::IsSatisfied(sLootGenInfo & a_Info) const
	{
		return std::all_of(m_Conditions.begin(), m_Conditions.end(), [&](const cCondition & a_Cond)
			{
				return a_Cond.IsSatisfied(a_Info);
			}
		);
	}





	void cEntry::AddDrops(cItems & a_Items, sLootGenInfo & a_Info) const
	{
		auto Visitor = MakeOverloaded(
			[&](const sItemDrop & a_ItemData)
			{
				cItem Item(a_ItemData.ID);
				for (const auto & Func : a_ItemData.m_Functions)
				{
					Func(Item, a_Info);
				}
			},
			[&](const cLootTable & a_Table)
			{
				a_Table.AddDrops(a_Items, a_Info);
			}
		);

		m_EntryData.Visit(Visitor);
	}





	bool cPool::IsSatisfied(sLootGenInfo & a_Info) const
	{
		return std::all_of(m_Conditions.begin(), m_Conditions.end(), [&](const cCondition & a_Cond)
			{
				return a_Cond.IsSatisfied(a_Info);
			}
		);
	}





	void cPool::AddDrops(cItems & a_Items, sLootGenInfo & a_Info) const
	{
		std::vector<int> CumulativeWeights(m_Entries.size());
		float Luck = a_Info.Luck;
		int TotalWeight = 0;
		for (size_t i = 0; i != m_Entries.size(); ++i)
		{
			TotalWeight += FloorC(m_Weights[i] + m_Qualities[i] * Luck);
			CumulativeWeights[i] = TotalWeight;
		}

		int Rand = a_Info.Rand() % TotalWeight;
		auto itr = std::upper_bound(CumulativeWeights.begin(), CumulativeWeights.end(), Rand);
		auto EntryIdx = itr - CumulativeWeights.begin();

		m_Entries[static_cast<size_t>(EntryIdx)].AddDrops(a_Items, a_Info);
	}


}