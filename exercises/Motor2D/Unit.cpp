#include "Unit.h"
#include "j1Render.h"
#include "j1App.h"
#include "EntityManager.h"
#include "j1Pathfinding.h"
#include "p2Log.h"
#include "math.h"
#include "j1Map.h"
#include "j1Collision.h"
#include "j1Textures.h"
#include "p2Defs.h"
#include "j1Scene.h"
#include "j1Gui.h"

Unit::Unit(int posX, int posY, bool isEnemy, unitType type)
{
	entityPosition.x = posX;
	entityPosition.y = posY;
	this->type = type;

	pugi::xml_document gameDataFile;
	pugi::xml_node gameData;
	pugi::xml_node unitNodeInfo;

	gameData = App->LoadGameData(gameDataFile);

	if (gameData.empty() == false)
	{
		pugi::xml_node unit;
		int i = 0;
		for (unit = gameData.child("Units").child("Unit"); unit; unit = unit.next_sibling("Unit")) {
			i++;
			if (unit.child("Info").child("ID").attribute("value").as_int() == type) {
				unitNodeInfo = unit;
				break;
			}
			LOG("%d", unit.child("Info").child("ID").attribute("value").as_int());
		}
	}

	string idleTexturePath = unitNodeInfo.child("Textures").child("Idle").attribute("value").as_string();
	string moveTexturePath = unitNodeInfo.child("Textures").child("Move").attribute("value").as_string();
	string attackTexturePath = unitNodeInfo.child("Textures").child("Attack").attribute("value").as_string();
	string dieTexturePath = unitNodeInfo.child("Textures").child("Die").attribute("value").as_string();

	faction = (unitFaction)unitNodeInfo.child("Stats").child("Faction").attribute("value").as_int();
	attackSpeed = 1 / unitNodeInfo.child("Stats").child("AttackSpeed").attribute("value").as_float();
	unitLife = unitNodeInfo.child("Stats").child("Life").attribute("value").as_int();
	unitMaxLife = unitLife;
	unitAttack = unitNodeInfo.child("Stats").child("Attack").attribute("value").as_int();
	unitDefense = unitNodeInfo.child("Stats").child("Defense").attribute("value").as_int();
	unitPiercingDamage = unitNodeInfo.child("Stats").child("PiercingDamage").attribute("value").as_int();
	unitMovementSpeed = unitNodeInfo.child("Stats").child("MovementSpeed").attribute("value").as_float();

	pugi::xml_node animationNode;
	int width;
	int height;
	int rows;
	int columns;

	//Anim Idle//
	animationNode = unitNodeInfo.child("Animations").child("Idle");
	width = animationNode.child("Width").attribute("value").as_int();
	height = animationNode.child("Height").attribute("value").as_int();
	rows = animationNode.child("Rows").attribute("value").as_int();
	columns = animationNode.child("Columns").attribute("value").as_int();
	for (int i = 0; i < rows; i++) {
		Animation idle;
		for (int j = 0; j < columns; j++) {
			idle.PushBack({ width*j,height*i,width,height });
		}
		idle.speed = animationNode.child("Speed").attribute("value").as_float();
		idleAnimations.push_back(idle);
		if (i != 0 && i != rows - 1) {
			idle.flip = SDL_FLIP_HORIZONTAL;
			idleAnimations.push_back(idle);
		}
	}

	//Anim Moving//
	animationNode = unitNodeInfo.child("Animations").child("Move");
	width = animationNode.child("Width").attribute("value").as_int();
	height = animationNode.child("Height").attribute("value").as_int();
	rows = animationNode.child("Rows").attribute("value").as_int();
	columns = animationNode.child("Columns").attribute("value").as_int();
	for (int i = 0; i < rows; i++) {
		Animation move;
		for (int j = 0; j < columns; j++) {
			move.PushBack({ width*j,height*i,width,height });
		}
		move.speed = animationNode.child("Speed").attribute("value").as_float();
		movingAnimations.push_back(move);
		if (i != 0 && i != rows - 1) {
			move.flip = SDL_FLIP_HORIZONTAL;
			movingAnimations.push_back(move);
		}
	}

	//Anim Attacking//
	animationNode = unitNodeInfo.child("Animations").child("Attack");
	width = animationNode.child("Width").attribute("value").as_int();
	height = animationNode.child("Height").attribute("value").as_int();
	rows = animationNode.child("Rows").attribute("value").as_int();
	columns = animationNode.child("Columns").attribute("value").as_int();
	for (int i = 0; i < rows; i++) {
		Animation attack;
		for (int j = 0; j < columns; j++) {
			attack.PushBack({ width*j,height*i,width,height });
		}
		attack.speed = animationNode.child("Speed").attribute("value").as_float();
		attackingAnimations.push_back(attack);
		if (i != 0 && i != rows - 1) {
			attack.flip = SDL_FLIP_HORIZONTAL;
			attackingAnimations.push_back(attack);
		}
	}

	//Anim Dying//
	animationNode = unitNodeInfo.child("Animations").child("Die");
	width = animationNode.child("Width").attribute("value").as_int();
	height = animationNode.child("Height").attribute("value").as_int();
	rows = animationNode.child("Rows").attribute("value").as_int();
	columns = animationNode.child("Columns").attribute("value").as_int();
	for (int i = 0; i < rows; i++) {
		Animation die;
		for (int j = 0; j < columns; j++) {
			die.PushBack({ width*j,height*i,width,height });
		}
		die.speed = animationNode.child("Speed").attribute("value").as_float();
		dyingAnimations.push_back(die);
		if (i != 0 && i != rows - 1) {
			die.flip = SDL_FLIP_HORIZONTAL;
			dyingAnimations.push_back(die);
		}
	}

	unitIdleTexture = App->tex->Load(idleTexturePath.c_str());
	unitMoveTexture = App->tex->Load(moveTexturePath.c_str());
	unitAttackTexture = App->tex->Load(attackTexturePath.c_str());
	unitDieTexture = App->tex->Load(dieTexturePath.c_str());

	entityTexture = unitIdleTexture;

	currentDirection = RIGHT; // starting direction
	SetAnim(currentDirection);

	SDL_Rect r = currentAnim->GetCurrentFrame();

	iPoint col_pos(entityPosition.x, entityPosition.y + (r.h / 2));
	COLLIDER_TYPE colliderType;

	colliderType = SOFT_COLLIDER;
	soft_collider = App->collision->AddCollider(col_pos, 15, colliderType, (Entity*) this, App->entityManager);

	colliderType = HARD_COLLIDER;
	hard_collider = App->collision->AddCollider(col_pos, 8, colliderType, (Entity*) this, App->entityManager );

	isSelected = false;
	isVisible = true;

	hpBarWidth = 40;
}

Unit::~Unit()
{
}

bool Unit::Update(float dt)
{
	switch (state) {
	case UNIT_MOVING:
		Move(dt);
		break;
	case UNIT_DEAD:
		if (currentAnim->Finished()) {
			App->entityManager->DeleteUnit(this, isEnemy);
		}
		break;
	}

	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP) {
		int x;
		int y;
		App->input->GetMousePosition(x, y);
		x -= App->render->camera.x;
		y -= App->render->camera.y;
		if (x < entityPosition.x + (hard_collider->r) && x > entityPosition.x - (hard_collider->r) &&
			y < entityPosition.y + (hard_collider->r) && y > entityPosition.y - (hard_collider->r)) {
			if (isVisible) {
				isSelected = true;
			}
		}
		else {
			if (isSelected) {
				isSelected = false;
			}
		}
	}

	return true;
}


bool Unit::Draw()
{
	SDL_Rect r = currentAnim->GetCurrentFrame();
	iPoint col_pos(entityPosition.x, entityPosition.y + (r.h / 2));
	soft_collider->pos = col_pos;
	hard_collider->pos = col_pos;

	if (isSelected) 
		App->render->DrawCircle(col_pos.x, col_pos.y, 12, 255, 255, 255, 255);

	App->render->Blit(entityTexture, entityPosition.x - (r.w / 2), entityPosition.y - (r.h / 2), &r, currentAnim->flip);

	return true;
}

unitType Unit::GetType() const
{
	return type;
}

int Unit::GetLife() const
{
	return unitLife;
}

void Unit::SetPos(int posX, int posY)
{
	entityPosition.x = posX;
	entityPosition.y = posY;
}

void Unit::SetSpeed(int amount)
{
	unitMovementSpeed = amount;
}

void Unit::SetDestination()
{
	iPoint target;
	App->input->GetMousePosition(target.x, target.y);
	target = App->map->WorldToMap(target.x - App->render->camera.x, target.y - App->render->camera.y);

	iPoint origin = App->map->WorldToMap(entityPosition.x, entityPosition.y);

	if (App->pathfinding->CreatePath(origin, target)) {
		const p2DynArray<iPoint>* last_path = App->pathfinding->GetLastPath();
		int elem_num = last_path->Count();
		for (int i = 0; i < elem_num; i++)
			path.push_back(*(last_path->At(i)));
	}

	if (path.size() > 0) {
		SetState(UNIT_MOVING);
		if (path.front() == origin) {
			if (path.size() > 1) {
				destinationTile = path.begin()._Ptr->_Next->_Myval;
				path.remove(path.begin()._Ptr->_Next->_Myval);
			}
		}
		else {
			destinationTile = path.front();
		}
		path.erase(path.begin());
	}
}

void Unit::Move(float dt)
{

	CalculateVelocity();
	LookAt();

	fPoint vel = (velocity * (unitMovementSpeed * 1.5));
	roundf(vel.x);
	roundf(vel.y);


	entityPosition.x += int(vel.x);
	entityPosition.y += int(vel.y);

	if (entityPosition.DistanceNoSqrt(destinationTileWorld) < 4) {

		if (path.size() > 0) {
			destinationTile = path.front();
			path.pop_front();
		}
		else
			SetState(UNIT_IDLE);
	}
}

void Unit::CalculateVelocity()
{
	destinationTileWorld = App->map->MapToWorld(destinationTile.x + 1, destinationTile.y);

	velocity.x = destinationTileWorld.x - entityPosition.x;
	velocity.y = destinationTileWorld.y - entityPosition.y;

	if(velocity.x != 0 || velocity.y != 0)
		velocity.Normalize();
}


void Unit::LookAt()
{
	if(velocity.x == 0 && velocity.y == 0)
		direction = DOWN_LEFT;
	else {

		float angle = atan2f(velocity.y, velocity.x) * RADTODEG;

		if (angle < 22.5 && angle > -22.5)
			direction = RIGHT;
		else if (angle >= 22.5 && angle <= 67.5)
			direction = DOWN_RIGHT;
		else if (angle > 67.5 && angle < 112.5)
			direction = DOWN;
		else if (angle >= 112.5 && angle <= 157.5)
			direction = DOWN_LEFT;
		else if (angle > 157.5 || angle < -157.5)
			direction = LEFT;
		else if (angle >= -157.5 && angle <= -112.5)
			direction = UP_LEFT;
		else if (angle > -112.5 && angle < -67.5)
			direction = UP;
		else if (angle >= -67.5 && angle <= -22.5)
			direction = UP_RIGHT;

	}

	if (direction != currentDirection)
	{
		currentDirection = direction;
		SetAnim(currentDirection);
	}

}


void Unit::Dead() {
	SetState(UNIT_DEAD);
}

void Unit::SetState(unitState newState)
{
	switch (newState) {
	case UNIT_IDLE:
		this->state = UNIT_IDLE;
		SetAnim(currentDirection);
		entityTexture = unitIdleTexture;
		break;
	case UNIT_MOVING:
		this->state = UNIT_MOVING;
		SetAnim(currentDirection);
		entityTexture = unitMoveTexture;
		break;
	case UNIT_ATTACKING:
		this->state = UNIT_ATTACKING;
		SetAnim(currentDirection);
		entityTexture = unitAttackTexture;
		break;
	case UNIT_DEAD:
		this->state = UNIT_DEAD;
		SetAnim(currentDirection);
		entityTexture = unitDieTexture;
		break;
	}
}

bool Unit::Load(pugi::xml_node & node)
{
	return true;
}

bool Unit::Save(pugi::xml_node & node) const
{
	return true;
}

void Unit::SetAnim(unitDirection currentDirection) {

	switch (state) {
	case UNIT_IDLE:
		currentAnim = &idleAnimations[currentDirection];
		break;
	case UNIT_MOVING:
		currentAnim = &movingAnimations[currentDirection];
		break;
	case UNIT_ATTACKING:
		currentAnim = &attackingAnimations[currentDirection];
		break;
	case UNIT_DEAD:
		currentAnim = &dyingAnimations[currentDirection];
		break;
	}
}

pugi::xml_node Unit::LoadUnitInfo(unitType type)
{
	return pugi::xml_node();
}
