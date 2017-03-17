#ifndef __SCENE_H__
#define __SCENE_H__

#include "j1Module.h"
#include "j1Timer.h"

struct SDL_Texture;
class Unit;

class j1Scene : public j1Module
{
public:

	j1Scene();

	// Destructor
	virtual ~j1Scene();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

private:
	SDL_Texture* debug_tex;
	bool debug = false;
	Unit* elvenArcher;
	Unit* troll;
};

#endif // __SCENE_H__