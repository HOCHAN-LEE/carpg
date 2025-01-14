// obs�uga odkrywania minimapy
// kod przerobiony z http://www.roguebasin.com/index.php?title=Permissive-fov
#include "Pch.h"
#include "GameCore.h"
#include "InsideLocation.h"
#include "Level.h"

#define X false,
#define _ true,

const bool fov_mask[] = {
	X X X _ _ _ _ _ X X X
	X X _ _ _ _ _ _ _ X X
	X _ _ _ _ _ _ _ _ _ X
	_ _ _ _ _ _ _ _ _ _ _
	_ _ _ _ _ _ _ _ _ _ _
	_ _ _ _ _ _ _ _ _ _ _
	_ _ _ _ _ _ _ _ _ _ _
	_ _ _ _ _ _ _ _ _ _ _
	X _ _ _ _ _ _ _ _ _ X
	X X _ _ _ _ _ _ _ X X
	X X X _ _ _ _ _ X X X
};

#undef X
#undef _

namespace FOV
{
	struct Line
	{
		bool isBelow(const Int2& point) const
		{
			return relativeSlope(point) > 0;
		}

		bool isBelowOrContains(const Int2& point) const
		{
			return relativeSlope(point) >= 0;
		}

		bool isAbove(const Int2& point) const
		{
			return relativeSlope(point) < 0;
		}

		bool isAboveOrContains(const Int2& point) const
		{
			return relativeSlope(point) <= 0;
		}

		bool doesContain(const Int2& point) const
		{
			return relativeSlope(point) == 0;
		}

		// negative if the line is above the point.
		// positive if the line is below the point.
		// 0 if the line is on the point.
		int relativeSlope(const Int2& point) const
		{
			return (far.y - near.y)*(far.x - point.x)
				- (far.y - point.y)*(far.x - near.x);
		}

		Int2 near, far;
	};

	struct Bump
	{
		Bump() : parent(nullptr) {}
		Int2 location;
		Bump* parent;
	};

	struct Field
	{
		Field() : steepBump(nullptr), shallowBump(nullptr) {}
		Line steep, shallow;
		Bump* steepBump, *shallowBump;
	};

	Int2 source, extent, quadrant;
	int w;
	vector<Door*>* doors;
	Tile* mapa;
	vector<Int2>* reveal;
	list<Bump> steepBumps;
	list<Bump> shallowBumps;
	list<Field> activeFields;

	inline bool findDoorBlocking(const Int2& pt)
	{
		for(vector<Door*>::const_iterator it = doors->begin(), end = doors->end(); it != end; ++it)
		{
			if((*it)->pt == pt)
				return (*it)->IsBlockingView();
		}
		return false;
	}

	inline bool isBlocked(int x, int y)
	{
		if(x < 0 || y < 0 || x >= w || y >= w)
			return true;

		Int2 real_pt(x, y);
		Tile& p = mapa[real_pt.x + real_pt.y*w];

		return (IsBlocking(p) || (p.type == DOORS && findDoorBlocking(real_pt)));
	}

	inline void visit(int x, int y)
	{
		if(x < 0 || y < 0 || x >= w || y >= w)
			return;
		Tile& p = mapa[x + y * w];
		if(!IsSet(p.flags, Tile::F_REVEALED))
			reveal->push_back(Int2(x, w - y - 1));
	}

	inline bool doesPermissiveVisit(int x, int y)
	{
		return !fov_mask[x + 5 + (y + 5) * 11];
	}

	bool actIsBlocked(const Int2& pos)
	{
		Int2 adjustedPos(pos.x*quadrant.x + source.x, pos.y*quadrant.y + source.y);
		bool result = isBlocked(adjustedPos.x, adjustedPos.y);
		if((quadrant.x * quadrant.y == 1 && pos.x == 0 && pos.y != 0)
			|| (quadrant.x * quadrant.y == -1 && pos.y == 0 && pos.x != 0)
			|| doesPermissiveVisit(pos.x*quadrant.x, pos.y*quadrant.y))
		{
			return result;
		}
		else
		{
			visit(adjustedPos.x, adjustedPos.y);
			return result;
		}
	}

	list<Field>::iterator checkField(list<Field>::iterator currentField)
	{
		list<Field>::iterator result = currentField;
		// If the two slopes are colinear, and if they pass through either
		// extremity, remove the field of view.
		if(currentField->shallow.doesContain(currentField->steep.near)
			&& currentField->shallow.doesContain(currentField->steep.far)
			&& (currentField->shallow.doesContain(Int2(0, 1)) || currentField->shallow.doesContain(Int2(1, 0))))
		{
			result = activeFields.erase(currentField);
		}
		return result;
	}

	void addShallowBump(const Int2& point, list<Field>::iterator currentField)
	{
		// First, the far point of shallow is set to the new point.
		currentField->shallow.far = point;
		// Second, we need to add the new bump to the shallow bump list for
		// future steep bump handling.
		Bump& bump = Add1(shallowBumps);
		bump.location = point;
		bump.parent = currentField->shallowBump;
		currentField->shallowBump = &bump;
		// Now we have too look through the list of steep bumps and see if
		// any of them are below the line.
		// If there are, we need to replace near point too.
		Bump* currentBump = currentField->steepBump;
		while(currentBump != nullptr)
		{
			if(currentField->shallow.isAbove(currentBump->location))
				currentField->shallow.near = currentBump->location;
			currentBump = currentBump->parent;
		}
	}

	void addSteepBump(const Int2& point, list<Field>::iterator currentField)
	{
		currentField->steep.far = point;
		Bump& bump = Add1(steepBumps);
		bump.location = point;
		bump.parent = currentField->steepBump;
		currentField->steepBump = &bump;
		// Now look through the list of shallow bumps and see if any of them
		// are below the line.
		Bump* currentBump = currentField->shallowBump;
		while(currentBump != nullptr)
		{
			if(currentField->steep.isBelow(currentBump->location))
				currentField->steep.near = currentBump->location;
			currentBump = currentBump->parent;
		}
	}

	void visitSquare(const Int2& dest, list<Field>::iterator& currentField)
	{
		//Info("%d, %d", dest.x, dest.y);
		// The top-left and bottom-right corners of the destination square.
		Int2 topLeft(dest.x, dest.y + 1);
		Int2 bottomRight(dest.x + 1, dest.y);
		while(currentField != activeFields.end()
			&& currentField->steep.isBelowOrContains(bottomRight))
		{
			// case ABOVE
			// The square is in case 'above'. This means that it is ignored
			// for the currentField. But the steeper fields might need it.
			++currentField;
		}
		if(currentField == activeFields.end())
		{
			// The square was in case 'above' for all fields. This means that
			// we no longer care about it or any squares in its diagonal rank.
			return;
		}

		// Now we check for other cases.
		if(currentField->shallow.isAboveOrContains(topLeft))
		{
			// case BELOW
			// The shallow line is above the extremity of the square, so that
			// square is ignored.
			return;
		}
		// The square is between the lines in some way. This means that we
		// need to visit it and determine whether it is blocked.
		bool isBlocked = actIsBlocked(dest);
		if(!isBlocked)
		{
			// We don't care what case might be left, because this square does
			// not obstruct.
			return;
		}

		if(currentField->shallow.isAbove(bottomRight)
			&& currentField->steep.isBelow(topLeft))
		{
			// case BLOCKING
			// Both lines intersect the square. This current field has ended.
			currentField = activeFields.erase(currentField);
		}
		else if(currentField->shallow.isAbove(bottomRight))
		{
			// case SHALLOW BUMP
			// The square intersects only the shallow line.
			addShallowBump(topLeft, currentField);
			currentField = checkField(currentField);
		}
		else if(currentField->steep.isBelow(topLeft))
		{
			// case STEEP BUMP
			// The square intersects only the steep line.
			addSteepBump(bottomRight, currentField);
			currentField = checkField(currentField);
		}
		else
		{
			// case BETWEEN
			// The square intersects neither line. We need to split into two fields.
			list<Field>::iterator steeperField = currentField;
			list<Field>::iterator shallowerField = activeFields.insert(currentField, *currentField);
			addSteepBump(bottomRight, shallowerField);
			checkField(shallowerField);
			addShallowBump(topLeft, steeperField);
			currentField = checkField(steeperField);
		}
	}

	void calculateFovQuadrant()
	{
		steepBumps.clear();
		shallowBumps.clear();
		activeFields.clear();

		Field& field = Add1(activeFields);
		field.shallow.near = Int2(0, 1);
		field.shallow.far = Int2(extent.x, 0);
		field.steep.near = Int2(1, 0);
		field.steep.far = Int2(0, extent.y);

		Int2 dest(0, 0);

		// Visit the source square exactly once (in quadrant 1).
		if(quadrant.x == 1 && quadrant.y == 1)
			actIsBlocked(dest);

		list<Field>::iterator currentField = activeFields.begin();
		int i = 0;
		int j = 0;
		int maxI = extent.x + extent.y;
		// For each square outline
		for(i = 1; i <= maxI && !activeFields.empty(); ++i)
		{
			int startJ = max(0, i - extent.x);
			int maxJ = min(i, extent.y);
			// Visit the nodes in the outline
			for(j = startJ; j <= maxJ && currentField != activeFields.end(); ++j)
			{
				dest.x = i - j;
				dest.y = j;
				visitSquare(dest, currentField);
			}
			currentField = activeFields.begin();
		}
	}

	void calculateFov()
	{
		const Int2 quadrants[4] = {
			Int2(1, 1),
			Int2(-1, 1),
			Int2(-1, -1),
			Int2(1, -1)
		};

		for(int i = 0; i < 4; ++i)
		{
			quadrant = quadrants[i];
			calculateFovQuadrant();
		}
	}

	void DungeonReveal(const Int2& tile, vector<Int2>& revealed_tiles)
	{
		InsideLocationLevel& lvl = static_cast<InsideLocation*>(game_level->location)->GetLevelData();

		source = tile;
		extent = Int2(5, 5);
		w = lvl.w;
		doors = &lvl.doors;
		mapa = lvl.map;
		reveal = &revealed_tiles;

		// je�li gracz stoi w zamkni�tych drzwiach to nic nie odkrywaj
		if(lvl.map[tile(lvl.w)].type == DOORS && findDoorBlocking(tile))
			return;

		calculateFov();
	}
}
