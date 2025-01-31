#include "Pch.h"
#include "GameCore.h"
#include "InsideBuilding.h"
#include "SaveState.h"
#include "Content.h"
#include "Door.h"
#include "BuildingGroup.h"
#include "GameFile.h"
#include "Level.h"
#include "Object.h"
#include "GroundItem.h"
#include "ParticleSystem.h"
#include "BitStreamFunc.h"

//=================================================================================================
void InsideBuilding::Save(GameWriter& f, bool local)
{
	f << offset;
	f << inside_spawn;
	f << outside_spawn;
	f << xsphere_pos;
	f << enter_region;
	f << exit_region;
	f << outside_rot;
	f << top;
	f << xsphere_radius;
	f << building->id;
	f << level_shift;
	f << region1;
	f << region2;
	f << enter_y;

	LevelArea::Save(f);
}

//=================================================================================================
void InsideBuilding::Load(GameReader& f, bool local)
{
	f >> offset;
	f >> inside_spawn;
	f >> outside_spawn;
	f >> xsphere_pos;
	f >> enter_region;
	f >> exit_region;
	f >> outside_rot;
	f >> top;
	f >> xsphere_radius;
	building = Building::Get(f.ReadString1());
	f >> level_shift;
	f >> region1;
	f >> region2;
	f >> enter_y;

	if(LOAD_VERSION >= V_0_11)
		LevelArea::Load(f, local);
	else
		LevelArea::Load(f, local, old::LoadCompatibility::InsideBuilding);
}

//=================================================================================================
void InsideBuilding::Write(BitStreamWriter& f)
{
	LevelArea::Write(f);

	f << level_shift;
	f << building->id;
	f << xsphere_pos;
	f << enter_region;
	f << exit_region;
	f << top;
	f << xsphere_radius;
	f << enter_y;
}

//=================================================================================================
bool InsideBuilding::Read(BitStreamReader& f)
{
	if(!LevelArea::Read(f))
		return false;

	f >> level_shift;
	const string& building_id = f.ReadString1();
	f >> xsphere_pos;
	f >> enter_region;
	f >> exit_region;
	f >> top;
	f >> xsphere_radius;
	f >> enter_y;
	if(!f)
	{
		Error("Broken packet for inside building.");
		return false;
	}
	building = Building::Get(building_id);
	if(!building || !building->inside_mesh)
	{
		Error("Invalid building id '%s'.", building_id.c_str());
		return false;
	}
	offset = Vec2(512.f*level_shift.x + 256.f, 512.f*level_shift.y + 256.f);
	game_level->ProcessBuildingObjects(*this, game_level->city_ctx, this, building->inside_mesh, nullptr, 0, 0, Vec3(offset.x, 0, offset.y), building, nullptr, true);

	return true;
}
