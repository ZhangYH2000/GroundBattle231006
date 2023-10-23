#pragma once
#include "tile.h"

extern int n_dir;

struct Cur;
struct Var;
struct Faction;
struct Arm {
	wstring name;
	bool ranged = false;
	double dmg = 0;
	double max_t_attack = 0;
	dvec tl_guard, br_guard;
	ptr<tile> img_bullet;
	map<Faction*, ptr<tile>> t_bullet;
	vector<ptr<tile>> imgs_normal, imgs_receive;
	map<Faction*, vector<ptr<tile>>> ts_normal;

	int id = 0;
	bool dead = false;
	bool hovered = false;
	bool selected = false;

	Arm(Cur& cur, Var const& v);
	dvec tl(Cur& cur) const;
	drect rect(Cur& cur) const;
	void refresh_ts(Cur& cur);

	void render(Cur& cur);
	void Update(Cur& cur);
	void Discard(Cur& cur);
	void PreUpdate(Cur& cur);
};
