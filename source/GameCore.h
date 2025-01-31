#pragma once

//-----------------------------------------------------------------------------
#include <EngineCore.h>
#include <File.h>
#include <Physics.h>
#include <Sound.h>
#include <Texture.h>
#include <Key.h>
#include <Entity.h>
#include <WindowsIncludes.h>
#define far
#define IN
#define OUT
#include <slikenet/peerinterface.h>
#include <slikenet/MessageIdentifiers.h>
#include <slikenet/BitStream.h>
#undef far
#undef IN
#undef OUT

//-----------------------------------------------------------------------------
// usings
using namespace SLNet;
using namespace app;

//-----------------------------------------------------------------------------
// angel script
class asIScriptContext;
class asIScriptEngine;
class asIScriptFunction;
class asIScriptObject;
class asIScriptModule;
class asITypeInfo;

//-----------------------------------------------------------------------------
class ActionPanel;
class Arena;
class BitStreamReader;
class BitStreamWriter;
class BookPanel;
class CommandParser;
class Console;
class Controls;
class CreateCharacterPanel;
class CreateServerPanel;
class Game;
class GameComponent;
class GameGui;
class GameMenu;
class GameMessages;
class GamePanel;
class GamePanelContainer;
class GameReader;
class GameResources;
class GameStats;
class GameWriter;
class InfoBox;
class Inventory;
class InventoryPanel;
class Journal;
class Level;
class LevelAreaContext;
class LevelGui;
class LoadScreen;
class LobbyApi;
class MainMenu;
class Minimap;
class MpBox;
class MultiplayerPanel;
class Net;
class Options;
class Pathfinding;
class PickServerPanel;
class QuestManager;
class Quest_Scripted;
class SaveLoad;
class ScriptManager;
class ServerPanel;
class StatsPanel;
class Team;
class TeamPanel;
class World;
class WorldMapGui;

//-----------------------------------------------------------------------------
struct Action;
struct AIController;
struct Amulet;
struct Armor;
struct BaseObject;
struct BaseUsable;
struct Blood;
struct Book;
struct Bow;
struct Building;
struct BuildingGroup;
struct BuildingScript;
struct Bullet;
struct Camera;
struct CameraCollider;
struct Chest;
struct CityBuilding;
struct Class;
struct CollisionObject;
struct Consumable;
struct CreatedCharacter;
struct DialogContext;
struct DialogEntry;
struct DialogScripts;
struct Door;
struct Drain;
struct Electro;
struct Encounter;
struct EntityInterpolator;
struct Explo;
struct FrameInfo;
struct GameDialog;
struct GroundItem;
struct HeroData;
struct Human;
struct HumanData;
struct IdlePack;
struct Item;
struct ItemList;
struct ItemListResult;
struct ItemScript;
struct ItemSlot;
struct LevelArea;
struct LeveledItemList;
struct Light;
struct LoadingHandler;
struct LocationEventHandler;
struct LocationTexturePack;
struct Music;
struct NetChange;
struct NetChangePlayer;
struct News;
struct Object;
struct OutsideObject;
struct OtherItem;
struct PlayerController;
struct PlayerInfo;
struct Portal;
struct Quest;
struct Quest_Dungeon;
struct Quest_Encounter;
struct Quest_Event;
struct QuestDialog;
struct QuestHandler;
struct QuestInfo;
struct QuestList;
struct QuestScheme;
struct Ring;
struct Room;
struct RoomGroup;
struct SaveSlot;
struct ScriptEvent;
struct Shield;
struct SoundPack;
struct SpeechBubble;
struct Spell;
struct SpellList;
struct StatProfile;
struct Stock;
struct TerrainTile;
struct TexPack;
struct Tile;
struct TmpLevelArea;
struct Trap;
struct Unit;
struct UnitData;
struct UnitEventHandler;
struct UnitGroup;
struct UnitStats;
struct Usable;
struct Var;
struct VarsContainer;
struct Weapon;

//-----------------------------------------------------------------------------
// Locations
struct Camp;
struct Cave;
struct City;
struct InsideBuilding;
struct InsideLocation;
struct InsideLocationLevel;
struct Location;
struct MultiInsideLocation;
struct OutsideLocation;
struct SingleInsideLocation;

//-----------------------------------------------------------------------------
// Location generators
class AcademyGenerator;
class CampGenerator;
class CaveGenerator;
class CityGenerator;
class DungeonGenerator;
class EncounterGenerator;
class ForestGenerator;
class LabyrinthGenerator;
class LocationGenerator;
class LocationGeneratorFactory;
class MoonwellGenerator;
class SecretLocationGenerator;
class TutorialLocationGenerator;

//-----------------------------------------------------------------------------
enum CMD;
enum DialogOp : short;
enum Direction;
enum EncounterMode;
enum GameDirection;
enum ITEM_TYPE;
enum MATERIAL_TYPE;
enum TRAP_TYPE;
enum SpecialEncounter;
enum class AttributeId;
enum class EffectId;
enum class EffectSource;
enum class HeroType;
enum class MusicType;
enum class Perk;
enum class QuestCategory;
enum class SkillId;

//-----------------------------------------------------------------------------
typedef pair<UnitData*, int> TmpSpawn;

//-----------------------------------------------------------------------------
namespace global
{
	extern LobbyApi* api;
	extern CommandParser* cmdp;
	extern Game* game;
	extern GameGui* game_gui;
	extern GameResources* game_res;
	extern GameStats* game_stats;
	extern Level* game_level;
	extern Net* net;
	extern Pathfinding* pathfinding;
	extern CustomCollisionWorld* phy_world;
	extern QuestManager* quest_mgr;
	extern ScriptManager* script_mgr;
	extern Team* team;
	extern World* world;
}
using namespace global;
