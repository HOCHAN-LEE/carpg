#pragma once

#include "Content.h"

//-----------------------------------------------------------------------------
// Building group
struct BuildingGroup
{
	string id;
	vector<Building*> buildings;
};

//-----------------------------------------------------------------------------
struct BuildingGroup;
struct UnitData;

//-----------------------------------------------------------------------------
// Old building enum - required for pre 0.5 compability
enum class OLD_BUILDING
{
	B_MERCHANT,
	B_BLACKSMITH,
	B_ALCHEMIST,
	B_TRAINING_GROUNDS,
	B_INN,
	B_CITY_HALL,
	B_VILLAGE_HALL,
	B_BARRACKS,
	B_HOUSE,
	B_HOUSE2,
	B_HOUSE3,
	B_ARENA,
	B_FOOD_SELLER,
	B_COTTAGE,
	B_COTTAGE2,
	B_COTTAGE3,
	B_VILLAGE_INN,
	B_NONE,
	B_VILLAGE_HALL_OLD,
	B_MAX
};

//-----------------------------------------------------------------------------
// Base building
struct Building : public TypeItem
{
	enum TileScheme
	{
		SCHEME_GRASS,
		SCHEME_BUILDING,
		SCHEME_SAND,
		SCHEME_PATH,
		SCHEME_UNIT,
		SCHEME_BUILDING_PART
	};

	enum Flags
	{
		FAVOR_CENTER = 1 << 0,
		FAVOR_ROAD = 1 << 1,
		HAVE_NAME = 1 << 2,
		LIST = 1 << 3
	};

	string id, mesh_id, inside_mesh_id, name;
	Int2 size, shift[4];
	vector<TileScheme> scheme;
	int flags;
	BuildingGroup* group;
	Animesh* mesh, *inside_mesh;
	UnitData* unit;

	Building() : size(0, 0), shift(), flags(0), mesh(nullptr), inside_mesh(nullptr), group(nullptr), unit(nullptr) {}
};

//-----------------------------------------------------------------------------
// Building used when constructing map
struct ToBuild
{
	Building* type;
	Int2 pt, unit_pt;
	int rot;
	bool required;

	ToBuild(Building* type, bool required = true) : type(type), required(required) {}
};
