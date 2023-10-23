#pragma once
#include "dvec.h"

struct Cur;
struct Faction;
struct path_pnt {
	path_pnt* root = NULL;
	dvec p;
	int g = 0;
	int h = 0;
	int f = 0;
	path_pnt(dvec p) : p(p) {}
};
struct comp_f {
	bool operator()(ptr<path_pnt> p0, ptr<path_pnt> p1) const {
		return p0->f < p1->f;
	}
};

inline int geth(dvec p0, dvec p1) {
	int dx = abs(p0.x - p1.x);
	int dy = abs(p0.y - p1.y);
	// return dx + dy;
	int _min = min(dx, dy), _max = max(dx, dy);
	return _min * 14 + 10 * (_max - _min);
}
vector<dvec> AStar(Cur& cur, dvec p0, dvec p1);
vector<dvec> Dijkstra(Cur& cur, dvec p0, Faction* fac);
