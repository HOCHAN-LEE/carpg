#pragma once

//-----------------------------------------------------------------------------
#include "ComboBox.h"
#include "Location.h"

//-----------------------------------------------------------------------------
class WorldMapGui : public Control
{
public:
	WorldMapGui();
	void LoadLanguage();
	void LoadData(ResourceManager& res_mgr);
	void Draw(ControlDrawData* cdd = nullptr) override;
	void Update(float dt) override;
	bool NeedCursor() const override { return true; }
	void Event(GuiEvent e) override;
	void Save(FileWriter& f);
	void Load(FileReader& f);
	void Clear();
	Vec2 WorldPosToScreen(const Vec2& pt) const;
	void ShowEncounterMessage(cstring text);
	void StartTravel()
	{
		follow = true;
		tracking = -1;
	}
	bool HaveFocus() const { return !combo_box.focus; }

	cstring txWorldDate, txCurrentLoc, txCitizens, txAvailable, txTarget, txDistance, txTravelTime, txDay, txDays, txOnlyLeaderCanTravel;
	int picked_location;
	DialogBox* dialog_enc;

private:
	void AppendLocationText(Location& loc, string& s);
	void GetCityText(City& city, string& s);
	void CenterView(float dt, const Vec2* target = nullptr);
	Vec2 GetCameraCenter() const;

	Game& game;
	TexturePtr tMapBg, tWorldMap, tMapIcon[LI_MAX], tEnc, tSelected[2], tMover, tSide, tMagnifyingGlass, tTrackingArrow;
	cstring txBuildings;
	ComboBox combo_box;
	Vec2 offset, c_pos;
	float zoom;
	int tracking;
	bool clicked, follow, c_pos_valid;
};
