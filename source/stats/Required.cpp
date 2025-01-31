#include "Pch.h"
#include "GameCore.h"
#include "Game.h"
#include "Spell.h"
#include "BuildingGroup.h"
#include "Building.h"
#include "BuildingScript.h"
#include "BaseUsable.h"
#include "Stock.h"
#include "UnitGroup.h"
#include "Item.h"
#include "UnitData.h"
#include "QuestList.h"

extern string g_system_dir;

enum RequiredType
{
	R_ITEM,
	R_LIST,
	R_STOCK,
	R_UNIT,
	R_GROUP,
	R_SPELL,
	R_DIALOG,
	R_BUILDING_GROUP,
	R_BUILDING,
	R_BUILDING_SCRIPT,
	R_OBJECT,
	R_USABLE,
	R_QUEST_LIST
};

//=================================================================================================
void CheckStartItems(SkillId skill, bool required, uint& errors)
{
	bool have_0 = !required, have_heirloom = false;

	for(StartItem& si : StartItem::start_items)
	{
		if(si.skill == skill)
		{
			if(si.value == 0)
				have_0 = true;
			else if(si.value == HEIRLOOM)
				have_heirloom = true;
			if(have_0 && have_heirloom)
				return;
		}
	}

	if(!have_0)
	{
		Error("Missing starting item for skill %s.", Skill::skills[(int)skill].id);
		++errors;
	}
	if(!have_heirloom)
	{
		Error("Missing heirloom item for skill %s.", Skill::skills[(int)skill].id);
		++errors;
	}
}

//=================================================================================================
void CheckBaseItem(cstring name, int num, uint& errors)
{
	if(num == 0)
	{
		Error("Missing base %s.", name);
		++errors;
	}
	else if(num > 1)
	{
		Error("Multiple base %ss (%d).", name, num);
		++errors;
	}
}

//=================================================================================================
void CheckBaseItems(uint& errors)
{
	int have_short_blade = 0,
		have_long_blade = 0,
		have_axe = 0,
		have_blunt = 0,
		have_wand = 0,
		have_bow = 0,
		have_shield = 0,
		have_light_armor = 0,
		have_medium_armor = 0,
		have_heavy_armor = 0,
		have_mage_armor = 0;
	const ItemList* lis = ItemList::Get("base_items").lis;

	for(const Item* item : lis->items)
	{
		if(item->type == IT_WEAPON)
		{
			if(IsSet(item->flags, ITEM_MAGE))
				++have_wand;
			else
			{
				switch(item->ToWeapon().weapon_type)
				{
				case WT_SHORT_BLADE:
					++have_short_blade;
					break;
				case WT_LONG_BLADE:
					++have_long_blade;
					break;
				case WT_AXE:
					++have_axe;
					break;
				case WT_BLUNT:
					++have_blunt;
					break;
				}
			}
		}
		else if(item->type == IT_BOW)
			++have_bow;
		else if(item->type == IT_SHIELD)
			++have_shield;
		else if(item->type == IT_ARMOR)
		{
			if(IsSet(item->flags, ITEM_MAGE))
				++have_mage_armor;
			else
			{
				switch(item->ToArmor().armor_type)
				{
				case AT_LIGHT:
					++have_light_armor;
					break;
				case AT_MEDIUM:
					++have_medium_armor;
					break;
				case AT_HEAVY:
					++have_heavy_armor;
					break;
				}
			}
		}
	}

	CheckBaseItem("short blade weapon", have_short_blade, errors);
	CheckBaseItem("long blade weapon", have_long_blade, errors);
	CheckBaseItem("axe weapon", have_axe, errors);
	CheckBaseItem("blunt weapon", have_blunt, errors);
	CheckBaseItem("mage weapon", have_wand, errors);
	CheckBaseItem("bow", have_bow, errors);
	CheckBaseItem("shield", have_shield, errors);
	CheckBaseItem("light armor", have_light_armor, errors);
	CheckBaseItem("medium armor", have_medium_armor, errors);
	CheckBaseItem("heavy armor", have_heavy_armor, errors);
	CheckBaseItem("mage armor", have_mage_armor, errors);
}

//=================================================================================================
bool Game::LoadRequiredStats(uint& errors)
{
	Tokenizer t;
	if(!t.FromFile(Format("%s/required.txt", g_system_dir.c_str())))
	{
		Error("Failed to open required.txt.");
		++errors;
		return false;
	}

	t.AddKeywords(0, {
		{ "item", R_ITEM },
		{ "list", R_LIST },
		{ "stock", R_STOCK },
		{ "unit", R_UNIT },
		{ "group", R_GROUP },
		{ "spell", R_SPELL },
		{ "dialog", R_DIALOG },
		{ "building_group", R_BUILDING_GROUP },
		{ "building", R_BUILDING },
		{ "building_script", R_BUILDING_SCRIPT },
		{ "object", R_OBJECT },
		{ "usable", R_USABLE },
		{ "quest_list", R_QUEST_LIST }
		});

	try
	{
		t.Next();

		while(!t.IsEof())
		{
			try
			{
				RequiredType type = (RequiredType)t.MustGetKeywordId(0);
				t.Next();

				const string& str = t.MustGetItemKeyword();

				switch(type)
				{
				case R_ITEM:
					{
						ItemListResult result;
						const Item* item = Item::TryGet(str);
						if(!item)
						{
							Error("Missing required item '%s'.", str.c_str());
							++errors;
						}
					}
					break;
				case R_LIST:
					{
						bool leveled = false;
						if(t.IsItem("leveled"))
						{
							leveled = true;
							t.Next();
						}
						ItemListResult result = ItemList::TryGet(str.c_str());
						if(!result.lis)
						{
							Error("Missing required item list '%s'.", str.c_str());
							++errors;
						}
						else if(result.is_leveled != leveled)
						{
							if(leveled)
								Error("Required list '%s' must be leveled.", str.c_str());
							else
								Error("Required list '%s' is leveled.", str.c_str());
							++errors;
						}
						else if(result.lis->items.empty())
						{
							Error("Required list '%s' is empty.", str.c_str());
							++errors;
						}
					}
					break;
				case R_STOCK:
					{
						Stock* stock = Stock::TryGet(str);
						if(!stock)
						{
							Error("Missing required item stock '%s'.", str.c_str());
							++errors;
						}
					}
					break;
				case R_UNIT:
					{
						UnitData* ud = UnitData::TryGet(str.c_str());
						if(!ud)
						{
							Error("Missing required unit '%s'.", str.c_str());
							++errors;
						}
					}
					break;
				case R_GROUP:
					if(str == "list")
					{
						t.Next();
						const string& group_id = t.MustGetItemKeyword();
						UnitGroup* group = UnitGroup::TryGet(group_id);
						if(!group)
						{
							Error("Missing required unit group list '%s'.", group_id.c_str());
							++errors;
						}
						else if(!group->is_list)
						{
							Error("Required unit group '%s' is not list.", group_id.c_str());
							++errors;
						}
					}
					else
					{
						bool need_leader = false;
						if(str == "with_leader")
						{
							need_leader = true;
							t.Next();
						}
						const string& group_id = t.MustGetItemKeyword();
						UnitGroup* group = UnitGroup::TryGet(group_id);
						if(!group)
						{
							Error("Missing required unit group '%s'.", group_id.c_str());
							++errors;
						}
						else if(group->is_list)
						{
							Error("Required unit group '%s' is list.", group_id.c_str());
							++errors;
						}
						else if(need_leader && !group->HaveLeader())
						{
							Error("Required unit group '%s' is missing leader.", group_id.c_str());
							++errors;
						}
					}
					break;
				case R_SPELL:
					{
						Spell* spell = Spell::TryGet(str);
						if(!spell)
						{
							Error("Missing required spell '%s'.", str.c_str());
							++errors;
						}
					}
					break;
				case R_DIALOG:
					{
						GameDialog* dialog = GameDialog::TryGet(str.c_str());
						if(!dialog)
						{
							Error("Missing required dialog '%s'.", str.c_str());
							++errors;
						}
					}
					break;
				case R_BUILDING_GROUP:
					{
						BuildingGroup* group = BuildingGroup::TryGet(str);
						if(!group)
						{
							Error("Missing required building group '%s'.", str.c_str());
							++errors;
						}
					}
					break;
				case R_BUILDING:
					{
						Building* building = Building::TryGet(str);
						if(!building)
						{
							Error("Missing required building '%s'.", str.c_str());
							++errors;
						}
					}
					break;
				case R_BUILDING_SCRIPT:
					{
						BuildingScript* script = BuildingScript::TryGet(str);
						if(!script)
						{
							Error("Missing required building script '%s'.", str.c_str());
							++errors;
							break;
						}

						t.Next();
						t.AssertSymbol('{');
						t.Next();

						while(!t.IsSymbol('}'))
						{
							const string& id = t.MustGetItem();
							if(!script->HaveBuilding(id))
							{
								Error("Missing required building '%s' for building script '%s'.", script->id.c_str(), id.c_str());
								++errors;
								break;
							}
							t.Next();
						}
					}
					break;
				case R_OBJECT:
					{
						auto obj = BaseObject::TryGet(str);
						if(!obj)
						{
							Error("Missing required object '%s'.", str.c_str());
							++errors;
						}
					}
					break;
				case R_USABLE:
					{
						auto use = BaseUsable::TryGet(str);
						if(!use)
						{
							Error("Missing required usable object '%s'.", str.c_str());
							++errors;
						}
					}
					break;
				case R_QUEST_LIST:
					{
						bool not_none = false;
						if(t.IsItem("not_none"))
						{
							not_none = true;
							t.Next();
						}

						QuestList* list = QuestList::TryGet(str);
						if(!list)
						{
							Error("Missing required quest list '%s'.", str.c_str());
							++errors;
						}
						else if(not_none)
						{
							for(QuestList::Entry& e : list->entries)
							{
								if(!e.info)
								{
									Error("Required quest list '%s' can't contain 'none'.", list->id.c_str());
									++errors;
									break;
								}
							}
						}
					}
					break;
				}

				t.Next();
			}
			catch(const Tokenizer::Exception& e)
			{
				Error("Parse error: %s", e.ToString());
				++errors;
				t.SkipToKeywordGroup(0);
			}
		}
	}
	catch(const Tokenizer::Exception& e)
	{
		Error("Failed to load required entities: %s", e.ToString());
		++errors;
	}

	CheckStartItems(SkillId::SHORT_BLADE, true, errors);
	CheckStartItems(SkillId::LONG_BLADE, true, errors);
	CheckStartItems(SkillId::AXE, true, errors);
	CheckStartItems(SkillId::BLUNT, true, errors);
	CheckStartItems(SkillId::BOW, false, errors);
	CheckStartItems(SkillId::SHIELD, false, errors);
	CheckStartItems(SkillId::LIGHT_ARMOR, true, errors);
	CheckStartItems(SkillId::MEDIUM_ARMOR, true, errors);
	CheckStartItems(SkillId::HEAVY_ARMOR, true, errors);

	CheckBaseItems(errors);

	return true;
}
