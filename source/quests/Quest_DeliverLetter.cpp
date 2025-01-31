#include "Pch.h"
#include "GameCore.h"
#include "Quest_DeliverLetter.h"
#include "Game.h"
#include "Journal.h"
#include "LocationHelper.h"
#include "QuestManager.h"
#include "GameFile.h"
#include "World.h"
#include "Team.h"

//=================================================================================================
void Quest_DeliverLetter::Start()
{
	start_loc = world->GetCurrentLocationIndex();
	end_loc = world->GetRandomSettlementIndex(start_loc);
	type = Q_DELIVER_LETTER;
	category = QuestCategory::Mayor;
}

//=================================================================================================
GameDialog* Quest_DeliverLetter::GetDialog(int dialog_type)
{
	switch(dialog_type)
	{
	case QUEST_DIALOG_START:
		return GameDialog::TryGet("q_deliver_letter_start");
	case QUEST_DIALOG_FAIL:
		return GameDialog::TryGet("q_deliver_letter_timeout");
	case QUEST_DIALOG_NEXT:
		if(prog == Progress::Started)
			return GameDialog::TryGet("q_deliver_letter_give");
		else
			return GameDialog::TryGet("q_deliver_letter_end");
	default:
		assert(0);
		return nullptr;
	}
}

//=================================================================================================
void Quest_DeliverLetter::SetProgress(int prog2)
{
	prog = prog2;
	switch(prog)
	{
	case Progress::Started:
		// give letter to player
		{
			OnStart(game->txQuest[2]);
			quest_mgr->quests_timeout2.push_back(this);

			Location& loc = *world->GetLocation(end_loc);
			Item::Get("letter")->CreateCopy(letter);
			letter.id = "$letter";
			letter.name = Format(game->txQuest[0], LocationHelper::IsCity(loc) ? game->txForMayor : game->txForSoltys, loc.name.c_str());
			letter.quest_id = id;
			DialogContext::current->pc->unit->AddItem2(&letter, 1u, 1u);

			Location& loc2 = GetStartLocation();
			msgs.push_back(Format(game->txQuest[3], LocationHelper::IsCity(loc2) ? game->txForMayor : game->txForSoltys, loc2.name.c_str(), world->GetDate()));
			msgs.push_back(Format(game->txQuest[4], LocationHelper::IsCity(loc) ? game->txForMayor : game->txForSoltys, loc.name.c_str(),
				dir_name[GetLocationDir(loc2.pos, loc.pos)]));
		}
		break;
	case Progress::Timeout:
		// player failed to deliver letter in time
		{
			state = Quest::Failed;
			((City&)GetStartLocation()).quest_mayor = CityQuestState::Failed;
			if(world->GetCurrentLocationIndex() == end_loc)
				DialogContext::current->pc->unit->RemoveQuestItem(id);

			OnUpdate(game->txQuest[5]);
		}
		break;
	case Progress::GotResponse:
		// given letter, got response
		{
			Location& loc = *world->GetLocation(end_loc);
			letter.Rename(Format(game->txQuest[1], LocationHelper::IsCity(loc) ? game->txForMayor : game->txForSoltys, loc.name.c_str()));

			OnUpdate(game->txQuest[6]);
		}
		break;
	case Progress::Finished:
		// given response, end of quest
		{
			state = Quest::Completed;
			team->AddReward(250, 1000);

			((City&)GetStartLocation()).quest_mayor = CityQuestState::None;
			DialogContext::current->pc->unit->RemoveQuestItem(id);

			OnUpdate(game->txQuest[7]);
			RemoveElementTry(quest_mgr->quests_timeout2, static_cast<Quest*>(this));
		}
		break;
	}
}

//=================================================================================================
cstring Quest_DeliverLetter::FormatString(const string& str)
{
	Location& loc = *world->GetLocation(end_loc);
	if(str == "target_burmistrza")
		return (LocationHelper::IsCity(loc) ? game->txForMayor : game->txForSoltys);
	else if(str == "target_locname")
		return loc.name.c_str();
	else if(str == "target_dir")
		return dir_name[GetLocationDir(GetStartLocation().pos, loc.pos)];
	else
	{
		assert(0);
		return nullptr;
	}
}

//=================================================================================================
bool Quest_DeliverLetter::IsTimedout() const
{
	return world->GetWorldtime() - start_time > 30;
}

//=================================================================================================
bool Quest_DeliverLetter::OnTimeout(TimeoutType ttype)
{
	OnUpdate(game->txQuest[277]);
	return true;
}

//=================================================================================================
bool Quest_DeliverLetter::IfHaveQuestItem() const
{
	if(prog == Progress::Started)
		return world->GetCurrentLocationIndex() == end_loc;
	else
		return world->GetCurrentLocationIndex() == start_loc && prog == Progress::GotResponse;
}

//=================================================================================================
const Item* Quest_DeliverLetter::GetQuestItem()
{
	return &letter;
}

//=================================================================================================
void Quest_DeliverLetter::Save(GameWriter& f)
{
	Quest::Save(f);

	if(prog < Progress::Finished)
		f << end_loc;
}

//=================================================================================================
bool Quest_DeliverLetter::Load(GameReader& f)
{
	Quest::Load(f);

	if(prog < Progress::Finished)
	{
		f >> end_loc;
		if(prog >= Progress::Started)
		{
			Location& loc = *world->GetLocation(end_loc);
			Item::Get("letter")->CreateCopy(letter);
			letter.id = "$letter";
			letter.name = Format(game->txQuest[prog == Progress::GotResponse ? 1 : 0], LocationHelper::IsCity(loc) ? game->txForMayor : game->txForSoltys,
				loc.name.c_str());
			letter.quest_id = id;
		}
	}

	return true;
}
