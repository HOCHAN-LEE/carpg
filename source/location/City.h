#pragma once

//-----------------------------------------------------------------------------
#include "OutsideLocation.h"
#include "InsideBuilding.h"
#include "EntryPoint.h"
#include "Content.h"
#include "BuildingGroup.h"

//-----------------------------------------------------------------------------
enum CityTarget
{
	VILLAGE,
	CITY,
	CAPITAL
};

//-----------------------------------------------------------------------------
// Budynek w mie�cie
struct CityBuilding
{
	Building* building;
	Int2 pt, unit_pt;
	GameDirection rot;
	Vec3 walk_pt;

	CityBuilding() {}
	explicit CityBuilding(Building* building) : building(building) {}
};

//-----------------------------------------------------------------------------
enum class CityQuestState
{
	None,
	InProgress,
	Failed
};

//-----------------------------------------------------------------------------
struct City : public OutsideLocation
{
	enum Flags
	{
		HaveExit = 1 << 0,
		HaveTrainingGrounds = 1 << 1,
		HaveBlacksmith = 1 << 2,
		HaveMerchant = 1 << 3,
		HaveAlchemist = 1 << 4,
		HaveFoodSeller = 1 << 5,
		HaveInn = 1 << 6,
		HaveArena = 1 << 7
		// saved as byte in PreparedWorldData
	};

	int citizens, citizens_world, quest_mayor_time, quest_captain_time,
		arena_time, // last arena combat worldtime or -1
		gates, flags, variant;
	CityQuestState quest_mayor, quest_captain;
	vector<CityBuilding> buildings; // when visited this contain buildings to spawn (only type), after entering it is fully filled
	vector<InsideBuilding*> inside_buildings;
	Int2 inside_offset;
	Vec3 arena_pos;
	vector<EntryPoint> entry_points;

	City() : quest_mayor(CityQuestState::None), quest_captain(CityQuestState::None), quest_mayor_time(-1), quest_captain_time(-1),
		inside_offset(1, 0), arena_time(-1), flags(HaveExit), variant(-1)
	{
	}
	~City();

	// from Location
	void Apply(vector<std::reference_wrapper<LevelArea>>& areas) override;
	void Save(GameWriter& f, bool local) override;
	void Load(GameReader& f, bool local) override;
	void Write(BitStreamWriter& f) override;
	bool Read(BitStreamReader& f) override;
	bool FindUnit(Unit* unit, int* level) override;
	Unit* FindUnit(UnitData* data, int& at_level) override;

	void GenerateCityBuildings(vector<Building*>& buildings, bool required);
	void PrepareCityBuildings(vector<ToBuild>& tobuild);
	bool IsInsideCity(const Vec3& _pos);
	InsideBuilding* FindInsideBuilding(Building* building);
	InsideBuilding* FindInsideBuilding(BuildingGroup* group);
	InsideBuilding* FindInsideBuilding(BuildingGroup* group, int& index);
	InsideBuilding* FindInn() { return FindInsideBuilding(BuildingGroup::BG_INN); }
	InsideBuilding* FindInn(int& id) { return FindInsideBuilding(BuildingGroup::BG_INN, id); }
	CityBuilding* FindBuilding(BuildingGroup* group);
	CityBuilding* FindBuilding(Building* building);
	bool IsVillage() const { return target == VILLAGE; }
	void GetEntry(Vec3& pos, float& rot);
};
