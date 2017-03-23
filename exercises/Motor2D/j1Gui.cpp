#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Input.h"
#include "j1Window.h"
#include "j1Gui.h"

j1Gui::j1Gui() : j1Module()
{
	name = "gui";
}

// Destructor
j1Gui::~j1Gui()
{
	// Ignores last item of the list CHECK LATER
	for (list<UIElement*>::iterator it = Elements.begin(); it != Elements.end(); ++it)
	{
		Elements.erase(it);
	}

}

// Called before render is available
bool j1Gui::Awake(pugi::xml_node& conf)
{
	LOG("Loading GUI atlas");
	bool ret = true;
	atlas_file_name = conf.child("atlas").attribute("file").as_string("");

	return ret;
}

// Called before the first frame
bool j1Gui::Start()
{

	atlas = App->tex->Load(atlas_file_name.c_str());

	vector<SDL_Rect> sprites_cursor;
	sprites_cursor.push_back({ 0,   0, 70, 50 });
	sprites_cursor.push_back({ 70,   0, 70, 50 });
	sprites_cursor.push_back({ 140, 0, 70, 50 });
	sprites_cursor.push_back({ 210, 0, 70, 50 });
	sprites_cursor.push_back({ 280, 0, 70, 50 });
	sprites_cursor.push_back({ 350, 0, 70, 50 });
	sprites_cursor.push_back({ 420, 0, 70, 50 });
	sprites_cursor.push_back({ 490, 0, 70, 50 });
	sprites_cursor.push_back({ 560, 0, 70, 50 });
	sprites_cursor.push_back({ 0,   50, 70, 50 });
	sprites_cursor.push_back({ 70,  50, 70, 50 });
	sprites_cursor.push_back({ 140, 50, 70, 50 });
	sprites_cursor.push_back({ 210, 50, 70, 50 });
	sprites_cursor.push_back({ 280, 50, 70, 50 });
	sprites_cursor.push_back({ 350, 50, 70, 50 });
	sprites_cursor.push_back({ 420, 50, 70, 50 });
	sprites_cursor.push_back({ 490, 50, 70, 50 });
	sprites_cursor.push_back({ 560, 50, 70, 50 });

	App->gui->cursor = (Cursor*)CreateCursor("gui/cursor.png", sprites_cursor);

	return true;
}

// Update all guis
bool j1Gui::PreUpdate()
{
	list<UIElement*> Priority;

	for (list<UIElement*>::iterator it = Elements.begin(); Priority.size() != Elements.size(); ++it)
	{
		switch (it._Ptr->_Myval->priority) {
		case 0:
			Priority.push_front(it._Ptr->_Myval);
			break;
		case 1:
			Priority.push_back(it._Ptr->_Myval);
			break;
		}
	}

	Elements = Priority;
	return true;
}

// Called after all Updates
bool j1Gui::PostUpdate()
{
	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN) {
		for (list<UIElement*>::iterator it = Elements.begin(); it != Elements.end(); ++it)
		{
			if (!it._Ptr->_Myval->debug) it._Ptr->_Myval->debug = true;
			else it._Ptr->_Myval->debug = false;
		}
	}

	if (Elements.empty() != true)
	{
		for (list<UIElement*>::iterator it = Elements.begin(); it != Elements.end(); ++it)
		{
			if (it._Ptr->_Myval->enabled == true)
				it._Ptr->_Myval->Update();
		}
	}

	cursor->Update();

	return true;
}

// Called before quitting
bool j1Gui::CleanUp()
{
	LOG("Freeing GUI");

	if (Elements.empty() != true)
	{
		for (list<UIElement*>::iterator it = Elements.begin(); it != Elements.end(); ++it)
		{
			delete it._Ptr->_Myval;
		}
	}

	return true;
}

// const getter for atlas
SDL_Texture* j1Gui::GetAtlas() const
{
	return atlas;
}

bool j1Gui::Save(pugi::xml_node &) const
{
	return true;
}

bool j1Gui::Load(pugi::xml_node &)
{
	return true;
}

void j1Gui::ScreenMoves(pair<int, int> movement) {

	if (Elements.empty() != true)
	{
		for (list<UIElement*>::iterator it = Elements.begin(); it != Elements.end(); ++it)
		{
			if (it._Ptr->_Myval->enabled == true)
				it._Ptr->_Myval->Movement(movement);
		}
	}
}

// UI ELEMENT
// methods:

void UIElement::SetPos(int x, int y)
{
	pos.first = x;
	pos.second = y;
}

void UIElement::SetParentPos(int x, int y) {
	parent_pos.first = x;
	parent_pos.second = y;
}
void UIElement::Move(int x, int y) {
	pos.first += x;
	pos.second += y;
}

UIElement::UIElement(bool argenabled, int argx, int argy, ElementType argtype, SDL_Texture * argtexture) : enabled(argenabled), pos(argx, argy), type(argtype), texture(argtexture) {
	debug_color = DEBUG_COLOR;
	current = FREE;
}

UIElement * j1Gui::CreateImage(char* path, int x, int y, SDL_Rect section)
{
	UIElement* ret = nullptr;
	SDL_Texture* tex;
	if (path != nullptr)
		tex = App->tex->Load(path);
	else tex = GetAtlas();

	ret = new Image(section, x, y, tex);
	Elements.push_back(ret);

	return ret;
}

UIElement * j1Gui::CreateImage(char* path, int x, int y)
{
	UIElement* ret = nullptr;
	SDL_Texture* tex;
	if (path != nullptr)
		tex = App->tex->Load(path);
	else tex = GetAtlas();

	ret = new Image(x, y, tex);
	Elements.push_back(ret);

	return ret;
}

UIElement * j1Gui::CreateQuad(SDL_Rect size, SDL_Color color)
{
	UIElement* ret = nullptr;
	ret = new Quad(size, color);
	Elements.push_back(ret);
	return ret;
}

UIElement * j1Gui::CreateCursor(char* path, vector<SDL_Rect> cursor_list)
{
	UIElement* ret = nullptr;
	SDL_Texture * tex = App->tex->Load(path);
	ret = new Cursor(tex, cursor_list);
	Elements.push_back(ret);
	return ret;
}

// IMAGE
Image::Image(SDL_Rect argsection, int x, int y, SDL_Texture* argtexture) : UIElement(true, x, y, IMAGE, argtexture), section(argsection) {}
Image::Image(int x, int y, SDL_Texture* argtexture) : UIElement(true, x, y, IMAGE, argtexture) {}


void Image::Update()
{
	Draw();
	if (debug) DebugMode();
}

void Image::Draw()
{
	if (section.w>0 && section.h>0)
		App->render->Blit(texture, pos.first, pos.second, &section);
	else App->render->Blit(texture, pos.first, pos.second);
}

MouseState Image::MouseDetect()
{
	MouseState ret = FREE;

	pair<int, int> mouse_pos;
	App->input->GetMousePosition(mouse_pos.first, mouse_pos.second);
	if (mouse_pos.first >= pos.first && mouse_pos.first <= (pos.first + section.w) && mouse_pos.second >= pos.second && mouse_pos.second <= (pos.second + section.h)) {
		ret = HOVER;
	}
	if (ret == FREE && App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT) {
		ret = CLICKOUT;
	}
	else {
		if (ret == HOVER && App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT) {
			ret = CLICKIN;

		}
	}
	return ret;
}

void Image::Movement(pair<int, int> movement) {
	pos.first -= movement.first;
	pos.second -= movement.second;
}

void Image::DebugMode() {
	App->render->DrawQuad({ pos.first, pos.second, section.w, section.h }, debug_color.r, debug_color.g, debug_color.b, debug_color.a, true);
}
// LABEL 

// QUAD

Quad::Quad(SDL_Rect area, SDL_Color color) : UIElement(true, area.x, area.y, QUAD, nullptr), color(color), area(area) {
}
void Quad::Update() {
	Draw();

}
void Quad::Draw() {

	App->render->DrawQuad(area, color.r, color.g, color.b, color.a, true);

}
void Quad::SetArea(SDL_Rect area) {
	this->area = area;
	SetPos(area.x, area.y);
}
void Quad::Movement(pair<int, int> movement) {
	area.x -= movement.first;
	area.y -= movement.second;
}

// CURSOR

Cursor::Cursor(SDL_Texture* tex, vector<SDL_Rect> area) : UIElement(true, area[1].x, area[1].y, CURSOR, tex), sprite_list(area) {
	id = 0;
}

void Cursor::Update() {
	if (texture != nullptr) {
		SDL_ShowCursor(false);
		App->input->GetMousePosition(cursor_pos.first, cursor_pos.second);
		pos.first = cursor_pos.first - App->render->camera.x;
		pos.second = cursor_pos.second - App->render->camera.y;
		Draw();
		if (id == 1 || id == 2 || id == 5 || id == 7 || id == 8 || id == 12 || id == 16) blitoffset.second = 20;
	}
}
void Cursor::Draw() {

	App->render->Blit(texture, pos.first - blitoffset.first, pos.second - blitoffset.second, &sprite_list[id]);
}
void Cursor::SetCursor(int id) {
	this->id = id;
}