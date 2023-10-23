#pragma once
#include "tile.h"

struct Cur;
struct Var;
struct Faction {
	wstring name;
	dcol col;
	int cnt = 0;
	int cnt_enemy = 0;

	int id = 0;
	bool dead = false;
	bool hovered = false;
	bool selected = false;

	Faction(Var const& v);
	dvec tl(Cur& cur) const;
	drect rect(Cur& cur) const;

	void Reset(Cur& cur);
	void render(Cur& cur);
	void Update(Cur& cur);
	void Discard(Cur& cur);
	void PreUpdate(Cur& cur);
};
