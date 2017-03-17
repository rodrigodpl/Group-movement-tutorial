#include "j1Collision.h"
#include "j1App.h"
#include "j1Input.h"
#include "p2Log.h"
#include "j1Render.h"


j1Collision::j1Collision() : j1Module()
{
	name = "collision";

	matrix[SOFT_COLLIDER][SOFT_COLLIDER] = true;
	matrix[SOFT_COLLIDER][HARD_COLLIDER] = true;
	matrix[HARD_COLLIDER][SOFT_COLLIDER] = true;
	matrix[HARD_COLLIDER][HARD_COLLIDER] = true;

}


j1Collision::~j1Collision()
{
}

bool j1Collision::Awake(pugi::xml_node &)
{
	return true;
}

bool j1Collision::Start()
{
	return true;
}

bool j1Collision::PreUpdate()
{
	for (p2List_item<Collider*>* it = colliders.start; it; it = it->next) {
		if (it->data->to_delete == true)
		{
			RELEASE(it->data);
			colliders.del(it);
		}
		else
			it->data->colliding = false;
	}

	Collider *c1;
	Collider *c2;


	for (p2List_item<Collider*>* col1 = colliders.start; col1; col1 = col1->next) {
		c1 = col1->data;

		for (p2List_item<Collider*>* col2 = colliders.start; col2; col2 = col2->next) {
			c2 = col2->data;

			if (c1->CheckCollision(c2) == true) {

				if (c1->GetEntity() != c2->GetEntity()) {

					if (matrix[c1->type][c2->type] && c1->callback)
						c1->callback->OnCollision(c1, c2);


					if (matrix[c2->type][c1->type] && c2->callback)
						c1->callback->OnCollision(c1, c2);
						
				}
			}
		}
	}
	return true;
}


bool j1Collision::Update(float dt)
{

	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN) {
		debug = !debug;
	}

	if (debug) {
		DebugDraw();
	}

	return true;
}

bool j1Collision::CleanUp()
{
	LOG("Freeing colliders");

	for (p2List_item<Collider*>* it = colliders.end; it; it = it->prev)
		RELEASE(it->data);
	

	colliders.clear();

	return true;
}

Collider * j1Collision::AddCollider(iPoint position, int radius, COLLIDER_TYPE type, Entity* assigned_entity, j1Module * callback )
{
	Collider* ret = new Collider(position, radius, type, assigned_entity, callback);
	colliders.add(ret);

	return ret;
}

void j1Collision::DeleteCollider(Collider * collider)
{
	collider->to_delete = true;
}

// TODO 2  Code the CheckCollision method

bool Collider::CheckCollision(Collider* c2) const
{
	return false; 
}

void j1Collision::DebugDraw()
{

	for (p2List_item<Collider*>* it = colliders.start; it; it = it->next)
	{
		if (it->data == nullptr) {
			continue;
		}

		if(it->data->colliding)
			App->render->DrawCircle(it->data->pos.x, it->data->pos.y, it->data->r, 255, 0, 0, 255);
		else
			App->render->DrawCircle(it->data->pos.x, it->data->pos.y, it->data->r, 0, 0, 255, 255);
	}
}
