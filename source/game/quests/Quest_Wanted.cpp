#include "Pch.h"
#include "Base.h"
#include "Quest_Wanted.h"
#include "Dialog.h"
#include "DialogDefine.h"
#include "Game.h"
#include "Journal.h"
#include "SaveState.h"
#include "GameFile.h"

//-----------------------------------------------------------------------------
DialogEntry wanted_start[] = {
	TALK(689),
	TALK2(690),
	TALK(691),
	CHOICE(692),
		SET_QUEST_PROGRESS(Quest_Wanted::Progress::Started),
		TALK2(693),
		TALK(694),
		TALK(695),
		END,
	END_CHOICE,
	CHOICE(696),
		END,
	END_CHOICE,
	ESCAPE_CHOICE,
	SHOW_CHOICES,
	END_OF_DIALOG
};

//-----------------------------------------------------------------------------
DialogEntry wanted_timeout[] = {
	SET_QUEST_PROGRESS(Quest_Wanted::Progress::Timeout),
	TALK2(697),
	TALK(698),
	END,
	END_OF_DIALOG
};

//-----------------------------------------------------------------------------
DialogEntry wanted_end[] = {
	TALK2(699),
	SET_QUEST_PROGRESS(Quest_Wanted::Progress::Finished),
	TALK2(700),
	END,
	END_OF_DIALOG
};

//=================================================================================================
void Quest_Wanted::Start()
{
	start_loc = game->current_location;
	quest_id = Q_WANTED;
	type = Type::Captain;
	level = random(5, 15);
	crazy = (rand2()%5 == 0);
	clas = ClassInfo::GetRandomEvil();
	target_unit = NULL;
	in_location = -1;
}

//=================================================================================================
DialogEntry* Quest_Wanted::GetDialog(int type2)
{
	switch(type2)
	{
	case QUEST_DIALOG_START:
		return wanted_start;
	case QUEST_DIALOG_FAIL:
		return wanted_timeout;
	case QUEST_DIALOG_NEXT:
		return wanted_end;
	default:
		assert(0);
		return NULL;
	}
}

//=================================================================================================
void Quest_Wanted::SetProgress(int prog2)
{
	prog = prog2;
	switch(prog2)
	{
	case Progress::Started: // zaakceptowano
		{
			game->GenerateHeroName(clas, crazy, unit_name);
			target_loc = game->GetFreeRandomCityLocation(start_loc);
			// je�li nie ma wolnego miasta to powie jakie� ale go tam nie b�dzie...
			if(target_loc == -1)
				target_loc = game->GetRandomCityLocation(start_loc);
			Location& target = GetTargetLocation();
			if(!target.active_quest)
			{
				target.active_quest = this;
				unit_to_spawn = game->GetUnitDataFromClass(clas, crazy);
				unit_dont_attack = true;
				unit_event_handler = this;
				send_spawn_event = true;
				unit_spawn_level = level;
			}

			// dane questa
			start_time = game->worldtime;
			state = Quest::Started;
			name = game->txQuest[257];

			// dodaj list
			letter.ani = NULL;
			letter.flags = ITEM_QUEST|ITEM_IMPORTANT|ITEM_TEX_ONLY;
			letter.id = "$wanted_letter";
			letter.mesh.clear();
			letter.name = game->txQuest[258];
			letter.refid = refid;
			letter.tex = game->tListGonczy;
			letter.type = IT_OTHER;
			letter.value = 0;
			letter.weight = 1;
			letter.desc = Format(game->txQuest[259], level*100, unit_name.c_str());
			letter.other_type = OtherItems;
			game->current_dialog->pc->unit->AddItem(&letter, 1, true);

			quest_index = game->quests.size();
			game->quests.push_back(this);
			game->quests_timeout.push_back(this);
			RemoveElement<Quest*>(game->unaccepted_quests, this);

			// wpis do dziennika
			msgs.push_back(Format(game->txQuest[29], GetStartLocationName(), game->day+1, game->month+1, game->year));
			msgs.push_back(Format(game->txQuest[260], level*100, unit_name.c_str(), GetTargetLocationName(), GetTargetLocationDir()));
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);

			if(game->IsOnline())
			{
				game->Net_AddQuest(refid);
				game->Net_RegisterItem(&letter);
				if(!game->current_dialog->is_local)
				{
					game->Net_AddItem(game->current_dialog->pc, &letter, true);
					game->Net_AddedItemMsg(game->current_dialog->pc);
				}
				else
					game->AddGameMsg3(GMS_ADDED_ITEM);
			}
			else
				game->AddGameMsg3(GMS_ADDED_ITEM);
		}
		break;
	case Progress::Timeout: // czas min��
		{
			state = Quest::Failed;
			((City*)game->locations[start_loc])->quest_captain = CityQuestState::Failed;

			Location& target = GetTargetLocation();
			if(target.active_quest == this)
				target.active_quest = NULL;

			msgs.push_back(Format(game->txQuest[261], unit_name.c_str()));
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);

			if(game->IsOnline())
				game->Net_UpdateQuest(refid);

			done = false;
		}
		break;
	case Progress::Killed: // zabito
		{
			state = Quest::Started; // if recruited that will change it to in progress
			msgs.push_back(Format(game->txQuest[262], unit_name.c_str()));
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);

			RemoveElementTry<Quest_Dungeon*>(game->quests_timeout, this);

			if(game->IsOnline())
				game->Net_UpdateQuest(refid);
		}
		break;
	case Progress::Finished: // wykonano
		{
			state = Quest::Completed;
			((City*)game->locations[start_loc])->quest_captain = CityQuestState::None;

			game->AddReward(level*100);

			msgs.push_back(Format(game->txQuest[263], unit_name.c_str()));
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);

			if(game->IsOnline())
				game->Net_UpdateQuest(refid);
		}
		break;
	case Progress::Recruited:
		{
			state = Quest::Failed;
			msgs.push_back(Format(game->txQuest[276], target_unit->GetName()));
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);

			if(game->IsOnline())
				game->Net_UpdateQuest(refid);
		}
		break;
	}
}

//=================================================================================================
cstring Quest_Wanted::FormatString(const string& str)
{
	if(str == "reward")
		return Format("%d", level*100);
	else if(str == "name")
		return unit_name.c_str();
	else if(str == "target_loc")
		return GetTargetLocationName();
	else if(str == "target_dir")
		return GetTargetLocationDir();
	else if(str == "player")
		return game->current_dialog->pc->name.c_str();
	else
	{
		assert(0);
		return NULL;
	}
}

//=================================================================================================
bool Quest_Wanted::IsTimedout() const
{
	return game->worldtime - start_time > 30;
}

//=================================================================================================
bool Quest_Wanted::OnTimeout(TimeoutType ttype)
{
	if(target_unit)
	{
		if(state == Quest::Failed)
			((City*)game->locations[start_loc])->quest_captain = CityQuestState::Failed;
		if(!target_unit->hero->team_member)
		{
			// not a team member, remove
			game->RemoveUnit(game->ForLevel(in_location), target_unit);
		}
		else
			target_unit->event_handler = NULL;
		target_unit = NULL;
	}

	msgs.push_back(game->txQuest[277]);
	game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
	game->AddGameMsg3(GMS_JOURNAL_UPDATED);

	return true;
}

//=================================================================================================
bool Quest_Wanted::IfHaveQuestItem() const
{
	return game->current_dialog->talker == target_unit;
}

//=================================================================================================
bool Quest_Wanted::IfNeedTalk(cstring topic) const
{
	return prog == Progress::Killed && strcmp(topic, "wanted") == 0;
}

//=================================================================================================
void Quest_Wanted::HandleUnitEvent(UnitEventHandler::TYPE type, Unit* unit)
{
	switch(type)
	{
	case UnitEventHandler::SPAWN:
		unit->hero->name = unit_name;
		GetTargetLocation().active_quest = NULL;
		target_unit = unit;
		in_location = game->current_location;
		break;
	case UnitEventHandler::DIE:
		if(!unit->hero->team_member)
		{
			SetProgress(Progress::Killed);
			target_unit = NULL;
		}
		break;
	case UnitEventHandler::RECRUIT:
		// target recruited, add note to journal
		if(prog != Progress::Recruited)
			SetProgress(Progress::Recruited);
		break;
	case UnitEventHandler::KICK:
		// kicked from team, can be killed now, don't dissapear
		unit->temporary = false;
		in_location = game->current_location;
		break;
	case UnitEventHandler::LEAVE:
		if(state == Quest::Failed)
			((City*)game->locations[start_loc])->quest_captain = CityQuestState::Failed;
		target_unit = NULL;
		break;
	}
}

//=================================================================================================
void Quest_Wanted::Save(HANDLE file)
{
	Quest_Dungeon::Save(file);

	GameWriter f(file);
	f << level;
	f << crazy;
	f << clas;
	f << unit_name;
	f << target_unit;
	f << in_location;
}

//=================================================================================================
void Quest_Wanted::Load(HANDLE file)
{
	Quest_Dungeon::Load(file);

	GameReader f(file);
	f >> level;
	f >> crazy;
	f >> clas;
	if(LOAD_VERSION < V_DEVEL)
		clas = ClassInfo::OldToNew(clas);
	f >> unit_name;
	f >> target_unit;
	if(LOAD_VERSION >= V_DEVEL)
		f >> in_location;
	else if(!target_unit || target_unit->hero->team_member)
		in_location = -1;
	else
		in_location = game->FindWorldUnit(target_unit, target_loc, game->current_location);

	if(!done)
	{
		unit_to_spawn = game->GetUnitDataFromClass(clas, crazy);
		unit_dont_attack = true;
		unit_event_handler = this;
		send_spawn_event = true;
		unit_spawn_level = level;
	}

	// list
	letter.ani = NULL;
	letter.flags = ITEM_QUEST|ITEM_IMPORTANT|ITEM_TEX_ONLY;
	letter.id = "$wanted_letter";
	letter.mesh.clear();
	letter.name = game->txQuest[258];
	letter.refid = refid;
	letter.tex = game->tListGonczy;
	letter.type = IT_OTHER;
	letter.value = 0;
	letter.weight = 1;
	letter.desc = Format(game->txQuest[259], level*100, unit_name.c_str());
	letter.other_type = OtherItems;

	if(game->mp_load)
		game->Net_RegisterItem(&letter);
}
