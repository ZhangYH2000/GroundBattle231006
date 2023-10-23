#pragma once
#include "tile.h"
#include "castable.h"

extern double max_t_receive;

struct Cur;
struct Arm;
struct Faction;
struct Agent : Castable {
	Arm* arm = NULL;
	Faction* fac = NULL;
	Agent* tar = NULL;
	double d = 0;
	dvec pgd;
	vec2 o, v;
	int n_kill = 0;
	double ang = 0, v_ang = 0;
	vector<dvec> path;
	double health = 0;

	double t_send = 0;
	double t_receive = 0;
	double t_path = 0;
	double t_wonder = 0;
	double t_attack = 0;
	double t_target = 0;
	bool dead = false;
	bool hovered = false;
	bool selected = false;
	bool teleported = false;

	Agent(Arm* arm, Faction* fac, vec2 o);
	Agent(Cur& cur, FILE* f);
	void Save(FILE* f);
	double dep() const;
	double GetD() const override { return d; }
	void Cast(Cur& cur) override { RenderCast(cur); }

	void slow_down(Cur& cur);
	void turn_to(Cur& cur, double tar_ang, double lim);
	void move_to(Cur& cur, vec2 p);
	void move_along(Cur& cur, vec2 p0, vec2 p1);

	void collide_cell(Cur& cur, dvec p);
	void collide_wall(Cur& cur);

	void CheckDead(Cur& cur);
	void RemoveDead(Cur& cur);
	void RefreshGridFac(Cur& cur);
	void RenderFlat(Cur& cur);
	void Update(Cur& cur);
	void Discard(Cur& cur);
	void PreUpdate(Cur& cur);
	void cast_health(Cur& cur);
	void RenderCast(Cur& cur);
};
void collide_agents(Cur& cur, Agent& a0, Agent& a1);
void CollideAgents(Cur& cur);

void CastSign(Cur& cur, tile const& t, vec2 p, double top, double _h);
void CastTarSel(Cur& cur);
void DrawAgentInfoSel(Cur& cur);
