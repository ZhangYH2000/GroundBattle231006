#pragma once
#include "tile.h"

extern double d_close;

struct Cur;
struct Cell;
struct cast_rec {
	bool hit = false;
	bool is_y = false;
	dvec pgd;
	double t = 0;
	double d = 0;
	int ux = 0;
	int y0 = 0, y1 = 0;
	dcol c_wall;
	ptr<tile> t_wall;
};

void sub_ray_cast(double dx, double dy,
	double& lx, double& ly, int sx, int sy, cast_rec& rc);
cast_rec RayCast(Cur& cur, vec2 v);
void CastWalls(Cur& cur);
void CastCeils(Cur& cur);
void CastCeilsClose(Cur& cur);
void CastFloors(Cur& cur);
void CastFloorsClose(Cur& cur);
void DrawCanvas(Cur& cur);

Cell* get_cell_test(Cur& cur, dvec pgd);
void sub_cast_floors(Cur& cur, int& ex, int& ey, int& x, int& y,
	int dx, int dy, dvec& pgd, Cell*& c, int sx, int sy, int& dp, int& sp);

bool VisibleTest(Cur& cur, vec2 o, vec2 v, double t, bool &reachable);
double TimeHit(Cur& cur, vec2 o, vec2 v);
void set_resolution(Cur& cur, int nx, int ny);
