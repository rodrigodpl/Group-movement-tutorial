// ----------------------------------------------------
// j1Module.h
// Interface for all engine modules
// ----------------------------------------------------

#ifndef __j1MODULE_H__
#define __j1MODULE_H__

#include "p2Defs.h"
#include "p2SString.h"
#include "p2DynArray.h"
#include "PugiXml\src\pugixml.hpp"
#include "SDL\include\SDL.h"
#include <list>
#include <string>
#include <vector>
#include <iostream>

class j1App;
class Gui;
struct Command;
enum GuiEvents;
struct CVar;
struct Collider;

class j1Module
{
public:

	j1Module() : active(false)
	{}

	virtual ~j1Module()
	{}

	void Init()
	{
		active = true;
	}

	// Called before render is available
	virtual bool Awake(pugi::xml_node&)
	{
		return true;
	}

	// Called before the first frame
	virtual bool Start()
	{
		return true;
	}

	// Called each loop iteration
	virtual bool PreUpdate()
	{
		return true;
	}

	// Called each loop iteration
	virtual bool Update(float dt)
	{
		return true;
	}

	// Called each loop iteration
	virtual bool PostUpdate()
	{
		return true;
	}

	// Called before quitting
	virtual bool CleanUp()
	{
		return true;
	}

	virtual bool Load(pugi::xml_node&)
	{
		return true;
	}

	virtual bool Save(pugi::xml_node&) const
	{
		return true;
	}

	virtual void OnGui(Gui* ui, GuiEvents event)
	{}

	virtual bool OnCommand(const Command*, const p2DynArray<p2SString>& arguments, p2SString& return_message)
	{
		return true;
	}

	virtual void OnCVar(const CVar* var)
	{}

	virtual void OnCollision(Collider* c1, Collider* c2)
	{}

public:

	p2SString	name;
	bool		active;

};

#endif // __j1MODULE_H__