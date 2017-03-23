#pragma once
#ifndef __ENTITY_MANAGER__
#define __ENTITY_MANAGER__

#include "j1Module.h"
#include "Entity.h"
#include "Unit.h"

class Entity;

class EntityManager : public j1Module {
public:
	EntityManager();

	// Destructor
	virtual ~EntityManager();

	// Called when before render is available
	bool Awake(pugi::xml_node&);

	// Call before first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Update Elements
	bool Update(float dt);

	// Called after all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	Unit* CreateUnit(int posX, int posY, bool isEnemy, unitType type);
	bool IsOccupied(iPoint tile, Unit* ignore_unit = NULL);

	void DeleteUnit(Unit* unit, bool isEnemy);
	void OnCollision(Collider* c1, Collider* c2);

private:
	void DestroyEntity(Entity* entity);

private:
	list<Unit*> friendlyUnitList;
	list<Unit*> removeUnitList;
	list<Unit*> selectedUnitList;
	Unit* selectedUnit;
	Building* selectedBuilding;
	SDL_Rect multiSelectionRect = { 0,0,0,0 };
	bool drawMultiSelectionRect;

public:
	int nextID;

};

#endif // !__ENTITY_MANAGER__