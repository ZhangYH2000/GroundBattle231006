#pragma once
#include "tile.h"
#include "castable.h"

struct Agent;
struct Faction;
struct Bullet : Castable {
	Agent* owner = NULL;
	Faction* fac = NULL;
	double dmg = 0;
	double d = 0;
	vec2 o, v;
	ptr<tile> t;

	double t_life = 0;
	bool dead = false;
	
	Bullet(vec2 o, vec2 v, Faction* fac, double t_life);
	void Cast(Cur& cur) override;
	double GetD() const override { return d; }

	void CheckDead(Cur& cur);
	void RemoveDead(Cur& cur);
	void Update(Cur& cur);
};
