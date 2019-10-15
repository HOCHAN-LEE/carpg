#include "Pch.h"
#include "GameCore.h"
#include "Bullet.h"
#include "SaveState.h"
#include "ResourceManager.h"
#include "Mesh.h"
#include "Unit.h"
#include "Spell.h"
#include "ParticleSystem.h"

//=================================================================================================
void Bullet::Save(FileWriter& f)
{
	f << pos;
	f << rot;
	if(mesh)
		f << mesh->filename;
	else
		f.Write0();
	f << speed;
	f << timer;
	f << attack;
	f << tex_size;
	f << yspeed;
	f << poison_attack;
	f << (owner ? owner->id : -1);
	if(spell)
		f << spell->id;
	else
		f.Write0();
	if(tex)
		f << tex->filename;
	else
		f.Write0();
	f << (trail ? trail->id : -1);
	f << (pe ? pe->id : -1);
	f << remove;
	f << backstab;
	f << level;
	f << start_pos;
}

//=================================================================================================
void Bullet::Load(FileReader& f)
{
	f >> pos;
	f >> rot;
	const string& mesh_id = f.ReadString1();
	if(!mesh_id.empty())
		mesh = res_mgr->Load<Mesh>(mesh_id);
	else
		mesh = nullptr;
	f >> speed;
	f >> timer;
	f >> attack;
	f >> tex_size;
	f >> yspeed;
	f >> poison_attack;
	owner = Unit::GetById(f.Read<int>());
	const string& spell_id = f.ReadString1();
	if(!spell_id.empty())
	{
		spell = Spell::TryGet(spell_id);
		if(!spell)
			throw Format("Missing spell '%s' for bullet.", spell_id.c_str());
	}
	else
		spell = nullptr;
	const string& tex_name = f.ReadString1();
	if(!tex_name.empty())
		tex = res_mgr->Load<Texture>(tex_name);
	else
		tex = nullptr;
	trail = TrailParticleEmitter::GetById(f.Read<int>());
	if(LOAD_VERSION < V_DEV)
	{
		TrailParticleEmitter* trail2 = TrailParticleEmitter::GetById(f.Read<int>());
		trail2->destroy = true;
		trail2->alive = 0;
	}
	pe = ParticleEmitter::GetById(f.Read<int>());
	f >> remove;
	f >> level;
	if(LOAD_VERSION >= V_0_10)
		f >> backstab;
	else
	{
		int backstab_value;
		f >> backstab_value;
		backstab = 0.25f * (backstab_value + 1);
	}
	f >> start_pos;
}
