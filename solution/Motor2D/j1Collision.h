#pragma once
#ifndef __j1Collision_H__
#define __j1Collision_H__

#include "j1Module.h"
#include "Entity.h"
#include "p2Point.h"
#include "Unit.h"
#include "p2List.h"

enum COLLIDER_TYPE
{
	COLLIDER_NONE = -1,
	SOFT_COLLIDER,
	HARD_COLLIDER,

	COLLIDER_MAX
};


struct Collider
{
	iPoint pos;
	int r;
	bool to_delete = false;
	bool colliding = false;
	COLLIDER_TYPE type;
	j1Module* callback = nullptr;
	Entity* entity = NULL;

	Collider(iPoint position, int radius, COLLIDER_TYPE type, Entity* assigned_entity, j1Module* callback = nullptr ) :
		pos(position),
		r(radius),
		type(type),
		callback(callback),
		entity(assigned_entity)
	{}

	void SetPos(int x, int y)
	{
		pos.x = x;
		pos.y = y;
	}

	bool CheckCollision(Collider* c2) const;

	Entity* GetEntity() 
	{
		return entity;
	}

	Unit* GetUnit()
	{
		return (Unit*)entity;
	}
};



class j1Collision : public j1Module
{
public:

	j1Collision();
	virtual ~j1Collision();

	// Called when before render is available
	bool Awake(pugi::xml_node&);

	// Call before first frame
	bool Start();

	// Called before all updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before quitting
	bool CleanUp();
	Collider* AddCollider(iPoint position, int radius, COLLIDER_TYPE type, Entity* assigned_entity, j1Module * callback);
	void DeleteCollider(Collider* collider);
	void DebugDraw();

private:

	p2List<Collider*> colliders;
	bool debug = false;

public:
	bool matrix[COLLIDER_MAX][COLLIDER_MAX];
};

#endif // __ModuleCollision_H__

