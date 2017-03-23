#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "EntityManager.h"
#include "j1PathFinding.h"

j1PathFinding::j1PathFinding() : j1Module(), map(NULL), last_path(DEFAULT_PATH_LENGTH), width(0), height(0), last_direction({ 0,0 })
{
	name.create("pathfinding");
}

// Destructor
j1PathFinding::~j1PathFinding()
{
	RELEASE_ARRAY(map);
}

// Called before quitting
bool j1PathFinding::CleanUp()
{
	LOG("Freeing pathfinding library");

	RELEASE_ARRAY(map);
	return true;
}

// Sets up the walkability map
void j1PathFinding::SetMap(uint width, uint height, uchar* data)
{
	this->width = width;
	this->height = height;

	RELEASE_ARRAY(map);
	map = new uchar[width*height];
	memcpy(map, data, width*height);
}

// Utility: return true if pos is inside the map boundaries
bool j1PathFinding::CheckBoundaries(const iPoint& pos) const
{
	return (pos.x >= 0 && pos.x <= (int)width &&
			pos.y >= 0 && pos.y <= (int)height);
}


// Utility: returns true is the tile is walkable
bool j1PathFinding::IsWalkable(const iPoint& pos) const
{
	uchar t = GetTileAt(pos);
	return t != INVALID_WALK_CODE && t > 0;
}

// Utility: return the walkability value of a tile
uchar j1PathFinding::GetTileAt(const iPoint& pos) const
{
	if(CheckBoundaries(pos))
		return map[(pos.y*width) + pos.x];

	return INVALID_WALK_CODE;
}

// To request all tiles involved in the last generated path
const p2DynArray<iPoint>* j1PathFinding::GetLastPath() const
{
	return &last_path;
}

// PathList ------------------------------------------------------------------------
// Looks for a node in this list and returns it's list node or NULL
// ---------------------------------------------------------------------------------
p2List_item<PathNode>* PathList::Find(const iPoint& point) const
{
	p2List_item<PathNode>* item = list.start;
	while(item)
	{
		if(item->data.pos == point)
			return item;
		item = item->next;
	}
	return NULL;
}

// PathList ------------------------------------------------------------------------
// Returns the Pathnode with lowest score in this list or NULL if empty
// ---------------------------------------------------------------------------------
p2List_item<PathNode>* PathList::GetNodeLowestScore() const
{
	p2List_item<PathNode>* ret = NULL;
	int min = 65535;

	p2List_item<PathNode>* item = list.end;
	while(item)
	{
		if(item->data.Score() < min)
		{
			min = item->data.Score();
			ret = item;
		}
		item = item->prev;
	}
	return ret;
}

// PathNode -------------------------------------------------------------------------
// Convenient constructors
// ----------------------------------------------------------------------------------
PathNode::PathNode() : g(-1), h(-1), pos(-1, -1), parent(NULL)
{}

PathNode::PathNode(int g, int h, const iPoint& pos, const PathNode* parent) : g(g), h(h), pos(pos), parent(parent)
{}

PathNode::PathNode(const PathNode& node) : g(node.g), h(node.h), pos(node.pos), parent(node.parent)
{}

// PathNode -------------------------------------------------------------------------
// Fills a list (PathList) of all valid adjacent pathnodes
// ----------------------------------------------------------------------------------
uint PathNode::FindWalkableAdjacents(PathList& list_to_fill) const
{
	iPoint cell;

	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {

			if (!(i == 0 && j == 0)) {

				cell.create(pos.x + i, pos.y + j);
				if (App->pathfinding->IsWalkable(cell) && !App->entityManager->IsOccupied(cell))
					list_to_fill.list.add(PathNode(-1, -1, cell, this));

			}
		}
	}

	// Needs optimization for diagonals if sides are not walkable

	return list_to_fill.list.count();
}

// PathNode -------------------------------------------------------------------------
// Calculates this tile score
// ----------------------------------------------------------------------------------
int PathNode::Score() const
{
	return g + h;
}

// PathNode -------------------------------------------------------------------------
// Calculate the F for a specific destination tile
// ----------------------------------------------------------------------------------
int PathNode::CalculateF(const iPoint& destination)
{
	g = parent->g + 1;
	h = pos.DistanceTo(destination);

	return g + h;
}

// ----------------------------------------------------------------------------------
// Actual A* algorithm: return number of steps in the creation of the path or -1 ----
// ----------------------------------------------------------------------------------
int j1PathFinding::CreatePath(const iPoint& origin, const iPoint& destination)
{
	int ret = -1;

	int iterations = 0;

	if (IsWalkable(origin) && IsWalkable(destination))
	{
		PathList open;
		PathList closed;
		PathList adjacent;

		// Start pushing the origin in the open list
		open.list.add(PathNode(0, 0, origin, NULL));

		// Iterate while we have open destinations to visit
		do
		{
			// Move the lowest score cell from open list to the closed list
			p2List_item<PathNode>* lowest = open.GetNodeLowestScore();
			p2List_item<PathNode>* node = closed.list.add(lowest->data);
			
			open.list.del(lowest);

			// If destination was added, we are done!
			if (node->data.pos == destination)
			{
				last_path.Clear();
				Waypoints.Clear();
				// Backtrack to create the final path
				const PathNode* path_node = &node->data;

				while (path_node)
				{
					last_path.PushBack(path_node->pos);
					path_node = path_node->parent;
				}

				last_path.Flip();
				ret = last_path.Count();
				LOG("Created path of %d waypoints in %d iterations", ret, iterations);
				break;
			}

			// Fill a list with all adjacent nodes
			adjacent.list.clear();
			node->data.FindWalkableAdjacents(adjacent);

			p2List_item<PathNode>* item = adjacent.list.start;
			for (; item; item = item->next)
			{
				if (closed.Find(item->data.pos) != NULL)
					continue;

				p2List_item<PathNode>* adjacent_in_open = open.Find(item->data.pos);
				if (adjacent_in_open == NULL)
				{
					item->data.CalculateF(destination);
					open.list.add(item->data);
				}
				else
				{
					if (adjacent_in_open->data.g > item->data.g + 1)
					{
						adjacent_in_open->data.parent = item->data.parent;
						adjacent_in_open->data.CalculateF(destination);
					}
				}
			}

			++iterations;
		} while (open.list.count() > 0);
	}

	return ret;
}


iPoint j1PathFinding::FindNearestAvailable(Unit* unit) const {

	iPoint pos = App->map->WorldToMap(unit->entityPosition.x, unit->entityPosition.y);
	iPoint ret = pos;
	iPoint adj;

	bool found = false;

	int dist = 0;

	while (!found) {

		dist++;

		for (int i = -dist; i < (dist + 1); i++) {
			for (int j = -dist; j <  (dist + 1); j++) {

				adj.create(pos.x + i, pos.y + j);

				if (App->pathfinding->IsWalkable(adj) && !App->entityManager->IsOccupied(adj)) {

					if (unit->path.empty()) {

						ret = adj;
						found = true;
					}
					else if (adj.DistanceManhattan(unit->path.front()) < ret.DistanceManhattan(unit->path.front())) {
						ret = adj;
						found = true;
					}
				}
			}
		}

		if (dist > 5)
			return pos;
	}

	return ret;
}
