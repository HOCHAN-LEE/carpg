#include "Pch.h"
#include "GameCore.h"
#include "Journal.h"
#include "Input.h"
#include "Game.h"
#include "GetTextDialog.h"
#include "Language.h"
#include "GameKeys.h"
#include "QuestManager.h"
#include "Quest.h"
#include "ResourceManager.h"
#include "World.h"
#include "GameGui.h"
#include "GameMessages.h"
#include "SoundManager.h"

//=================================================================================================
Journal::Journal() : mode(Quests)
{
	visible = false;
	Reset();

	font_height = GameGui::font->height;
}

//=================================================================================================
void Journal::LoadLanguage()
{
	Language::Section s = Language::GetSection("Journal");
	txAdd = s.Get("add");
	txNoteText = s.Get("noteText");
	txNoQuests = s.Get("noQuests");
	txNoRumors = s.Get("noRumors");
	txNoNotes = s.Get("noNotes");
	txAddNote = s.Get("addNote");
	txAddTime = s.Get("addTime");
}

//=================================================================================================
void Journal::Draw(ControlDrawData*)
{
	Rect r = { global_pos.x, global_pos.y, global_pos.x + size.x, global_pos.y + size.y };
	gui->DrawSpriteRect(tBook, r);
	gui->DrawSprite(tPage[mode], global_pos - Int2(64, 0));

	int x1 = page * 2, x2 = page * 2 + 1;

	// wypisz teksty na odpowiednich stronach
	// mo�na by to zoptymalizowa� �e najpierw szuka do, p�nij a� do ko�ca
	for(vector<Text>::iterator it = texts.begin(), end = texts.end(); it != end; ++it)
	{
		if(it->x == x1 || it->x == x2)
		{
			Rect r;
			if(it->x == x1)
				r = rect;
			else
				r = rect2;
			r.Top() += it->y * font_height;

			const Color color[3] = { Color::Black, Color::Red, Color::Green };

			gui->DrawText(GameGui::font, it->text, 0, color[it->color], r);
		}
	}

	// numery stron
	int pages = texts.back().x + 1;
	gui->DrawText(GameGui::font, Format("%d/%d", x1 + 1, pages), DTF_BOTTOM | DTF_CENTER, Color::Black, rect);
	if(x2 != pages)
		gui->DrawText(GameGui::font, Format("%d/%d", x2 + 1, pages), DTF_BOTTOM | DTF_CENTER, Color::Black, rect2);

	// strza�ki 32, 243
	if(page != 0)
		gui->DrawSprite(tArrowL, Int2(rect.Left() - 8, rect.Bottom() - 8));
	if(texts.back().x > x2)
		gui->DrawSprite(tArrowR, Int2(rect2.Right() + 8, rect.Bottom() - 8));
}

//=================================================================================================
void Journal::Update(float dt)
{
	for(pair<Mode, int>& change : changes)
	{
		if(mode == change.first)
		{
			if(mode == Quests && details)
			{
				if(change.second == open_quest)
					Build();
			}
			else
				Build();
		}
	}

	if(!focus)
		return;

	Mode new_mode = Invalid;

	if(input->Focus())
	{
		// zmiana trybu
		if(input->PressedRelease(Key::N1))
			new_mode = Quests;
		else if(input->PressedRelease(Key::N2))
			new_mode = Rumors;
		else if(input->PressedRelease(Key::N3))
			new_mode = Notes;
		// zmiana strony
		if(page != 0 && GKey.PressedRelease(GK_MOVE_LEFT))
			--page;
		if(texts.back().x > page * 2 + 1 && GKey.PressedRelease(GK_MOVE_RIGHT))
			++page;
		// do g�ry (je�li jest w que�cie to wychodzi do listy)
		if(mode == Quests && details && GKey.PressedRelease(GK_MOVE_FORWARD))
		{
			details = false;
			page = prev_page;
			Build();
		}
		// zmiana wybranego zadania
		if(mode == Quests && !quest_mgr->quests.empty())
		{
			if(GKey.PressedR(GK_ROTATE_LEFT) != Key::None)
			{
				if(!details)
				{
					// otw�rz ostatni quest na tej stronie
					open_quest = max((page + 1)*rect_lines * 2 - 1, int(quest_mgr->quests.size()) - 1);
					prev_page = page;
					page = 0;
					Build();
				}
				else
				{
					// poprzedni quest
					--open_quest;
					if(open_quest == -1)
						open_quest = quest_mgr->quests.size() - 1;
					page = 0;
					Build();
				}
			}
			if(GKey.PressedR(GK_ROTATE_RIGHT) != Key::None)
			{
				if(!details)
				{
					// pierwszy quest na stronie
					open_quest = page * rect_lines * 2;
					prev_page = page;
					page = 0;
					Build();
				}
				else
				{
					// nast�pny
					++open_quest;
					if(open_quest == (int)quest_mgr->quests.size())
						open_quest = 0;
					page = 0;
					Build();
				}
			}
		}

		if(new_mode != Invalid)
		{
			if(new_mode == mode)
			{
				if(mode == Quests && details)
				{
					// otw�rz star� stron�
					details = false;
					page = prev_page;
					Build();
				}
				else if(page != 0)
				{
					page = 0;
					Build();
				}
			}
			else
			{
				mode = new_mode;
				details = false;
				page = 0;
				Build();
			}
			new_mode = Invalid;
		}
	}

	// wyb�r zak�adki z lewej
	if(PointInRect(gui->cursor_pos, global_pos.x - 64 + 28, global_pos.y + 32, global_pos.x - 64 + 82, global_pos.y + 119))
		new_mode = Quests;
	else if(PointInRect(gui->cursor_pos, global_pos.x - 64 + 28, global_pos.y + 122, global_pos.x - 64 + 82, global_pos.y + 209))
		new_mode = Rumors;
	else if(PointInRect(gui->cursor_pos, global_pos.x - 64 + 28, global_pos.y + 212, global_pos.x - 64 + 82, global_pos.y + 299))
		new_mode = Notes;

	if(new_mode != Invalid)
	{
		gui->cursor_mode = CURSOR_HOVER;
		if(input->Focus() && input->PressedRelease(Key::LeftButton))
		{
			// zmie� zak�adk�
			if(new_mode == mode)
			{
				if(mode == Quests && details)
				{
					// otw�rz star� stron�
					details = false;
					page = prev_page;
					Build();
				}
				else if(page != 0)
				{
					page = 0;
					Build();
				}
			}
			else
			{
				mode = new_mode;
				details = false;
				page = 0;
				Build();
			}
		}
	}
	else if(mode == Quests)
	{
		if(!quest_mgr->quests.empty() && !details)
		{
			// wyb�r questa
			int what = -1;
			if(rect.IsInside(gui->cursor_pos))
				what = (gui->cursor_pos.y - rect.Top()) / font_height;
			else if(rect2.IsInside(gui->cursor_pos))
				what = (gui->cursor_pos.y - rect.Top()) / font_height + rect_lines;

			if(what != -1)
			{
				what += page * rect_lines * 2;
				if(what < int(quest_mgr->quests.size()))
				{
					gui->cursor_mode = CURSOR_HOVER;
					if(input->Focus() && input->PressedRelease(Key::LeftButton))
					{
						details = true;
						prev_page = page;
						page = 0;
						open_quest = what;
						Build();
					}
				}
			}
		}
	}
	else if(mode == Notes)
	{
		Text& last_text = texts.back();
		if(last_text.x == page * 2 || last_text.y == page * 2 + 1)
		{
			bool ok = false;
			if(last_text.x % 2 == 0)
			{
				if(gui->cursor_pos.x >= rect.Left() && gui->cursor_pos.x <= rect.Right())
					ok = true;
			}
			else
			{
				if(gui->cursor_pos.x >= rect2.Left() && gui->cursor_pos.x <= rect2.Right())
					ok = true;
			}

			if(ok && gui->cursor_pos.y >= rect.Top() + last_text.y*font_height && gui->cursor_pos.y <= rect.Top() + (last_text.y + 1)*font_height)
			{
				gui->cursor_mode = CURSOR_HOVER;
				if(input->Focus() && input->PressedRelease(Key::LeftButton))
				{
					// dodaj notatk�
					cstring names[] = { nullptr, txAdd };
					input_str.clear();
					GetTextDialogParams params(txNoteText, input_str);
					params.custom_names = names;
					params.event = delegate<void(int)>(this, &Journal::OnAddNote);
					params.limit = 255;
					params.lines = 8;
					params.multiline = true;
					params.parent = this;
					params.width = 400;
					GetTextDialog::Show(params);
				}
			}
		}
	}

	if(new_mode == Invalid)
	{
		if(page != 0)
		{
			if(PointInRect(gui->cursor_pos, rect.LeftBottom() - Int2(8, 8), Int2(16, 16)))
			{
				gui->cursor_mode = CURSOR_HOVER;
				if(input->Focus() && input->PressedRelease(Key::LeftButton))
					--page;
			}
		}
		if(texts.back().x > page * 2 + 1)
		{
			if(PointInRect(gui->cursor_pos, rect2.RightBottom() + Int2(8, -8), Int2(16, 16)))
			{
				gui->cursor_mode = CURSOR_HOVER;
				if(input->Focus() && input->PressedRelease(Key::LeftButton))
					++page;
			}
		}
	}

	if(input->Focus() && input->PressedRelease(Key::Escape))
		Hide();
}

//=================================================================================================
void Journal::Event(GuiEvent e)
{
	if(e == GuiEvent_Show || e == GuiEvent_WindowResize || e == GuiEvent_Resize || e == GuiEvent_Moved)
	{
		rect = Rect(32, 16, 238, 432);
		rect2 = Rect(259, 16, 455, 432);

		Vec2 scale = Vec2(size) / 512;
		rect = rect * scale + global_pos;
		rect2 = rect2 * scale + global_pos;

		rect_w = rect.SizeX();
		rect_lines = rect.SizeY() / font_height;

		if(e == GuiEvent_Resize)
			Build();
	}
}

//=================================================================================================
void Journal::Reset()
{
	mode = Quests;
	details = false;
	page = 0;
	notes.clear();
	rumors.clear();
}

//=================================================================================================
void Journal::Show()
{
	visible = true;
	Build();
	Event(GuiEvent_Show);
}

//=================================================================================================
void Journal::Hide()
{
	visible = false;
}

//=================================================================================================
void Journal::Build()
{
	texts.clear();
	x = 0;
	y = 0;

	if(mode == Quests)
	{
		// quests
		if(!details)
		{
			// list of quests
			if(quest_mgr->quests.empty())
				AddEntry(txNoQuests, 0, true);
			else
			{
				for(vector<Quest*>::iterator it = quest_mgr->quests.begin(), end = quest_mgr->quests.end(); it != end; ++it)
				{
					int color = 0;
					if((*it)->state == Quest::Failed)
						color = 1;
					else if((*it)->state == Quest::Completed)
						color = 2;
					AddEntry((*it)->name.c_str(), color, true);
				}
			}
		}
		else
		{
			// details of single quest
			Quest* quest = quest_mgr->quests[open_quest];
			for(vector<string>::iterator it = quest->msgs.begin(), end = quest->msgs.end(); it != end; ++it)
				AddEntry(it->c_str(), 0, false);
		}
	}
	else if(mode == Rumors)
	{
		// rumors
		if(rumors.empty())
			AddEntry(txNoRumors, 0, false);
		else
		{
			for(vector<string>::iterator it = rumors.begin(), end = rumors.end(); it != end; ++it)
				AddEntry(it->c_str(), 0, false);
		}
	}
	else
	{
		// notes
		if(notes.empty())
			AddEntry(txNoNotes, 0, false);
		else
		{
			for(vector<string>::iterator it = notes.begin(), end = notes.end(); it != end; ++it)
				AddEntry(it->c_str(), 0, false);
		}

		AddEntry(txAddNote, 0, false);
	}
}

//=================================================================================================
void Journal::AddEntry(cstring text, int color, bool singleline)
{
	if(singleline)
	{
		Text& t = Add1(texts);
		t.text = text;
		t.color = color;
		t.x = x;
		t.y = y;
		++y;
		if(y == rect_lines)
		{
			y = 0;
			++x;
		}
		return;
	}

	// ile linijek zajmuje tekst?
	Int2 osize = GameGui::font->CalculateSize(text, rect_w);
	int h = osize.y / font_height + 1;

	if(y + h >= rect_lines)
	{
		if(y == 0)
		{
			// ten tekst zajmuje ponad jedn� stron�
			assert(0);
			Text& t = Add1(texts);
			t.text = text;
			t.color = color;
			t.x = x;
			t.y = y;
			++x;
		}
		else
		{
			++x;
			y = 0;
			Text& t = Add1(texts);
			t.text = text;
			t.color = color;
			t.x = x;
			t.y = y;
			y += h;
			if(y == rect_lines)
			{
				y = 0;
				++x;
			}
		}
	}
	else
	{
		Text& t = Add1(texts);
		t.text = text;
		t.color = color;
		t.x = x;
		t.y = y;
		y += h;
		if(y == rect_lines)
		{
			y = 0;
			++x;
		}
	}
}

//=================================================================================================
void Journal::OnAddNote(int id)
{
	if(id == BUTTON_OK)
	{
		notes.push_back(Format(txAddTime, world->GetDate(), input_str.c_str()));
		sound_mgr->PlaySound2d(game_gui->messages->snd_scribble);
		Build();
		if(!Net::IsLocal())
			Net::PushChange(NetChange::ADD_NOTE);
	}
}

//=================================================================================================
void Journal::NeedUpdate(Mode at_mode, int quest_id)
{
	changes.push_back(std::make_pair(at_mode, quest_id));
}

//=================================================================================================
void Journal::LoadData()
{
	tBook = res_mgr->Load<Texture>("book.png");
	tPage[0] = res_mgr->Load<Texture>("dziennik_przyciski.png");
	tPage[1] = res_mgr->Load<Texture>("dziennik_przyciski2.png");
	tPage[2] = res_mgr->Load<Texture>("dziennik_przyciski3.png");
	tArrowL = res_mgr->Load<Texture>("strzalka_l.png");
	tArrowR = res_mgr->Load<Texture>("strzalka_p.png");
}

//=================================================================================================
void Journal::AddRumor(cstring text)
{
	assert(text);

	if(Net::IsOnline())
	{
		NetChange& c = Add1(Net::changes);
		c.type = NetChange::ADD_RUMOR;
		c.id = rumors.size();
	}

	rumors.push_back(Format(txAddTime, world->GetDate(), text));
	NeedUpdate(Journal::Rumors);
	game_gui->messages->AddGameMsg3(GMS_ADDED_RUMOR);
}

//=================================================================================================
void Journal::Save(FileWriter& f)
{
	f.WriteStringArray<uint, word>(rumors);
	f.WriteStringArray<uint, word>(notes);
}

//=================================================================================================
void Journal::Load(FileReader& f)
{
	f.ReadStringArray<uint, word>(rumors);
	f.ReadStringArray<uint, word>(notes);
}
