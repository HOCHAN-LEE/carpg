#include "Pch.h"
#include "Base.h"
#include "Quest_Main.h"
#include "Dialog.h"
#include "DialogDefine.h"
#include "Game.h"
#include "Journal.h"

//-----------------------------------------------------------------------------
DialogEntry dialog_main_event[] = {
	DO_ONCE,
	DO_QUEST("main"),
	RESTART,
	END_OF_DIALOG
};

//-----------------------------------------------------------------------------
DialogEntry dialog_main[] = {
	TALK2(1307),
	TALK(1308),
	SET_QUEST_PROGRESS(1),
	TALK2(1309),
	TALK(1310),	
	TALK(1311),
	END,
	END_OF_DIALOG
};

/*
DialogEntry dialog_main_new[] = {
	START_BLOCK,
		TALK2(1307),
		SET_QUEST_PROGRESS(1), // give letter
		TALK(1308),		
		TALK2(1309),
		SET_QUEST_PROGRESS(2), // give gold
		TALK(1310),
	END_BLOCK,
	TALK(1311),
	END
};
*/

//=================================================================================================
void Quest_Main::Start()
{
	start_loc = game->current_location;
	quest_id = Q_MAIN;
	type = Type::Unique;
	name = game->txQuest[269];
	timer = 0.f;
}

//=================================================================================================
DialogEntry* Quest_Main::GetDialog(int type2)
{
	return dialog_main;
}

//=================================================================================================
void Quest_Main::SetProgress(int prog2)
{
	prog = prog2;

	switch(prog)
	{
	case Progress::Started:
		{
			state = Quest::Started;

			GUI.SimpleDialog(game->txQuest[270], NULL);

			msgs.push_back(Format(game->txQuest[170], game->day + 1, game->month + 1, game->year));
			msgs.push_back(Format(game->txQuest[267], GetStartLocationName()));

			//game->RegisterDialogAction(dialog_main_event, start_loc, MAYOR);

			quest_index = game->quests.size();
			game->quests.push_back(this);
			RemoveElement<Quest*>(game->unaccepted_quests, this);

			if(game->IsOnline())
			{
				NetChange& c = Add1(game->net_changes);
				c.type = NetChange::ADD_QUEST_MAIN;
				c.id = refid;
			}
		}
		break;
	case Progress::TalkedWithMayor:
		{
			game->AddGold(75 + 25 * game->active_team.size(), NULL, true);
			const Item* letter = FindItem("q_main_letter");
			game->current_dialog->pc->unit->AddItem(letter, 1, true);

			close_loc = game->GetRandomCity(start_loc);
			Location& close = *game->locations[close_loc];
			target_loc = game->CreateLocation(L_ACADEMY, close.pos, 64.f, -1, SG_LOSOWO, false);
			Location& target = *game->locations[target_loc];
			target.state = LS_KNOWN;
			msgs.push_back(Format(game->txQuest[268], GetLocationDirName(close.pos, target.pos), close.name.c_str()));
			game->game_gui->journal->NeedUpdate(Journal::Quests, quest_index);
			game->AddGameMsg3(GMS_JOURNAL_UPDATED);

			if(game->IsOnline())
			{
				game->Net_UpdateQuest(refid);
				game->Net_ChangeLocationState(target_loc, false);
				if(!game->current_dialog->is_local)
				{
					game->Net_AddItem(game->current_dialog->pc, letter, true);
					game->Net_AddedItemMsg(game->current_dialog->pc);
				}
				else
					game->AddGameMsg3(GMS_ADDED_ITEM);
			}
			else
				game->AddGameMsg3(GMS_ADDED_ITEM);
		}
		break;
	}
}

//=================================================================================================
cstring Quest_Main::FormatString(const string& str)
{
	if(str == "player_name")
		return game->current_dialog->pc->name.c_str();
	else if(str == "target_dir")
		return GetLocationDirName(game->locations[close_loc]->pos, game->locations[target_loc]->pos);
	else if(str == "close_name")
		return game->locations[close_loc]->name.c_str();
	else
	{
		assert(0);
		return NULL;
	}
}

//=================================================================================================
bool Quest_Main::IfNeedTalk(cstring topic) const
{
	return strcmp(topic, "main") == 0;
}

//=================================================================================================
void Quest_Main::Save(HANDLE file)
{
	Quest::Save(file);

	FileWriter f(file);

	if(prog == Progress::TalkedWithMayor)
	{
		f << close_loc;
		f << target_loc;
	}
	else
		f << timer;
}

//=================================================================================================
void Quest_Main::Load(HANDLE file)
{
	Quest::Load(file);

	FileReader f(file);

	if(prog == Progress::TalkedWithMayor)
	{
		f >> close_loc;
		f >> target_loc;
	}
	else
		f >> timer;
}