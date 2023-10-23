#pragma once
#include "tile.h"

extern int h_wall;
extern int wh_cell;
extern int s_grid;

struct Cur; 
struct Agent;
struct Faction;
struct Terrain;
struct Cell {
	dvec pgd;
	Terrain* tr = NULL;
	dcol cx0, cx1, cy0, cy1, cz0, cz1;
	ptr<tile> tx0, tx1, ty0, ty1, tz0, tz1;

	set<Agent*> agents;
	map<Faction*, bool> facs;

	Cell(Cur& cur, dvec pgd);
	Cell(Cur& cur, dvec pgd, FILE* f);
	// 这里保存的话肯定不能直接还原颜色了，还是会有随机性。
	// 这里先不考虑这些，主要是贴图不太好处理。
	void Save(FILE* f);
	bool walkable() const;
	void set_terrain(Cur& cur, Terrain* _tr);

	void CheckDead(Cur& cur);
	void Reset(Cur& cur);
	void RenderFlat(Cur& cur);
};
Cell& get_cell(Cur& cur, dvec pgd);
dvec tl_cell(Cur& cur, dvec pgd);
void set_grid_size(Cur& cur, int nx, int ny);
