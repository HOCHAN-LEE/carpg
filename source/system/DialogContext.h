#pragma once

//-----------------------------------------------------------------------------
#include "GameDialog.h"

//-----------------------------------------------------------------------------
struct DialogChoice
{
	enum Type
	{
		Normal,
		Perk,
		Hero
	};

	int pos, quest_dialog_index;
	cstring msg, talk_msg;
	string* pooled; // used for dialog choices when message is formatted
	Type type;

	DialogChoice(int pos, cstring msg, int quest_dialog_index, string* pooled = nullptr) : pos(pos), msg(msg),
		quest_dialog_index(quest_dialog_index), pooled(pooled), type(Normal), talk_msg(nullptr) {}
};

//-----------------------------------------------------------------------------
struct DialogContext
{
	struct Entry
	{
		GameDialog* dialog;
		Quest* quest;
		int pos;
	};

	bool dialog_mode; // czy jest tryb dialogowy
	bool show_choices; // czy wy�wietlono opcje dialogowe do wyboru
	vector<DialogChoice> choices; // opcje dialogowe do wyboru
	int dialog_pos; // pozycja w dialogu
	int choice_selected; // zaznaczona opcja dialogowa
	int dialog_esc; // opcja dialogowa wybierana po wci�ni�ciu ESC
	int dialog_skip; // pomijanie opcji dialogowych u�ywane przez DTF_RANDOM_TEXT
	cstring dialog_text; // tekst dialogu
	string dialog_s_text; // tekst dialogu zmiennego
	Quest* dialog_quest; // quest zwi�zany z dialogiem
	GameDialog* dialog; // aktualny dialog
	Unit* talker; // posta� z kt�r� si� rozmawia
	float dialog_wait; // czas wy�wietlania opcji dialogowej
	bool dialog_once; // wy�wietlanie opcji dialogowej tylko raz
	cstring last_rumor;
	bool is_local;
	PlayerController* pc;
	int skip_id; // u�ywane w mp do pomijania dialog�w
	bool update_news;
	int update_locations; // 1-update, 0-updated, -1-no locations
	vector<News*> active_news;
	vector<pair<int, bool>> active_locations;
	int team_share_id;
	const Item* team_share_item;
	bool can_skip, force_end;
	vector<Entry> prev;
	cstring talk_msg;
	vector<QuestDialog> quest_dialogs;
	int quest_dialog_index;

	static DialogContext* current;

	~DialogContext() { ClearChoices(); }
	void StartDialog(Unit* talker, GameDialog* dialog = nullptr, Quest* quest = nullptr);
	void StartNextDialog(GameDialog* dialog, Quest* quest = nullptr);
	void Update(float dt);
	void EndDialog();
	void ClearChoices();
	cstring GetText(int index);
	bool ExecuteSpecial(cstring msg);
	bool ExecuteSpecialIf(cstring msg);
	cstring FormatString(const string& str_part);
	void DialogTalk(cstring msg);
private:
	void UpdateLoop();
	bool DoIfOp(int value1, int value2, DialogOp op);
	bool LearnPerk(int perk);
	bool RecruitHero(Class* clas);
};
