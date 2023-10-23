#pragma once
#include "tile.h"

struct Cur;
struct Var;
struct cast_tile {
	double p = 0;
	wstring nm;
};

struct Terrain {
	wstring name;
	dcol c_flat;
	col3 cx0, cx1, cy0, cy1, cz0, cz1;
	double cx0_rnd = 0, cx1_rnd = 0;
	double cy0_rnd = 0, cy1_rnd = 0;
	double cz0_rnd = 0, cz1_rnd = 0;
	bool wall = false;
	// wall 不一定要 walkable。
	bool walkable = true;
	vector<cast_tile> imgs_x0, imgs_x1, imgs_y0, imgs_y1, imgs_z0, imgs_z1;

	int id = 0;
	bool dead = false;
	bool hovered = false;
	bool selected = false;

	Terrain();
	Terrain(Var const &v);
	dvec tl(Cur& cur) const;
	drect rect(Cur& cur) const;

	void render(Cur& cur);
	void Update(Cur& cur);
	void Discard(Cur& cur);
	void PreUpdate(Cur& cur);
};
