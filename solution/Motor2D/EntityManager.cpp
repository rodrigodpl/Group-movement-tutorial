#include "EntityManager.h"
#include "p2Log.h"
#include "j1Collision.h"
#include "j1App.h"
#include "j1Scene.h"
#include "Unit.h"
#include "j1Render.h"
#include "j1Pathfinding.h"

EntityManager::EntityManager() : j1Module()
{
	name = "entityManager";
	nextID = 1;
}

EntityManager::~EntityManager()
{
}

bool EntityManager::Awake(pugi::xml_node & config)
{

	return true;
}

bool EntityManager::Start()
{
	LOG("Starting EntityManager");
	bool ret = true;
	drawMultiSelectionRect = false;
	return ret;
}

bool EntityManager::PreUpdate()
{
	return true;
}

bool EntityManager::IsOccupied(iPoint tile, Unit* ignore_unit) {

	for (list<Unit*>::iterator it = friendlyUnitList.begin(); it != friendlyUnitList.end(); it++) {
		if ((*it) != ignore_unit) {
			if (tile == App->map->WorldToMap((*it)->entityPosition.x, (*it)->entityPosition.y) && (*it)->state == UNIT_IDLE)
				return true;
		}
	}

	return false;

}

bool EntityManager::Update(float dt)
{
	int mouseX;
	int mouseY;
	App->input->GetMousePosition(mouseX, mouseY);
	mouseX -= App->render->camera.x;
	mouseY -= App->render->camera.y;


	for (list<Unit*>::iterator it = friendlyUnitList.begin(); it != friendlyUnitList.end(); it++) {
		(*it)->Update(dt);
		(*it)->Draw();
	}

	if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN) {		
		for (list<Unit*>::iterator it = friendlyUnitList.begin(); it != friendlyUnitList.end(); it++) {

			if ((*it)->isSelected) 
				(*it)->SetDestination();
		}
	}

	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN) {
		multiSelectionRect.x = mouseX;
		multiSelectionRect.y = mouseY;
	}

	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT) {
		multiSelectionRect.w = mouseX - multiSelectionRect.x;
		multiSelectionRect.h = mouseY - multiSelectionRect.y;
		if (drawMultiSelectionRect == false) {
			drawMultiSelectionRect = true;
		}
	}

	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP) {
		if (drawMultiSelectionRect == true) {
			drawMultiSelectionRect = false;

			for (list<Unit*>::iterator it = friendlyUnitList.begin(); it != friendlyUnitList.end(); it++) {
				if ((*it)->entityPosition.x > multiSelectionRect.x && (*it)->entityPosition.x < multiSelectionRect.x + multiSelectionRect.w &&
					(*it)->entityPosition.y > multiSelectionRect.y && (*it)->entityPosition.y <  multiSelectionRect.y + multiSelectionRect.h) {
					(*it)->isSelected = true;
				}
			}
			multiSelectionRect = { 0,0,0,0 };
		}
	}

	if (drawMultiSelectionRect) {
		App->render->DrawQuad(multiSelectionRect, 255, 255, 255, 255, false);
	}

	return true;
}

bool EntityManager::PostUpdate()
{
	if (removeUnitList.size() > 0) {
		list<Unit*>::iterator i = removeUnitList.begin();

		while (i != removeUnitList.end())
		{
			list<Unit*>::iterator unitToDestroy = i;
			++i;
			DestroyEntity((*unitToDestroy));
		}

		removeUnitList.clear();
	}

	return true;
}

bool EntityManager::CleanUp()
{
	LOG("Freeing EntityManager");

	for (list<Unit*>::iterator it = friendlyUnitList.begin(); it != friendlyUnitList.end(); it++) {
		RELEASE((*it));
	}
	friendlyUnitList.clear();


	for (list<Unit*>::iterator it = removeUnitList.begin(); it != removeUnitList.end(); it++) {
		RELEASE((*it));
	}
	removeUnitList.clear();

	return true;
}

Unit* EntityManager::CreateUnit(int posX, int posY, bool isEnemy, unitType type)
{
	Unit* unit = new Unit(posX, posY, isEnemy, type);
	unit->entityID = nextID;
	nextID++;

	friendlyUnitList.push_back(unit);


	return unit;
}


void EntityManager::DeleteUnit(Unit* unit, bool isEnemy)
{
	if (unit != nullptr) {
		removeUnitList.push_back(unit);
		friendlyUnitList.remove(unit);
	}
}

void EntityManager::OnCollision(Collider * c1, Collider * c2)
{
	c1->colliding = true; c2->colliding = true;
	Unit* unit_to_move = c1->GetUnit(); Unit* unit2 = c2->GetUnit();
	// if buildings are added, here it should be checked if c1 and c2 belong to units before continuing

	if (unit_to_move->state == UNIT_MOVING && unit2->state == UNIT_IDLE) {

		if (!unit_to_move->path.empty())
			unit_to_move->path.pop_front();

		unit_to_move->path.push_front(App->pathfinding->FindNearestAvailable(unit_to_move));
		unit_to_move->SetState(UNIT_MOVING);
	}
}


void EntityManager::DestroyEntity(Entity * entity)
{
	if (entity != nullptr) {
		list<Unit*>::iterator it = removeUnitList.begin();

		while (it != removeUnitList.end())
		{
			if (*it == entity)
			{
				removeUnitList.remove(*it);
				delete entity;
				return;
			}
			++it;
		}
	}
}
