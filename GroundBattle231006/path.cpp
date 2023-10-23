#include "path.h"
#include "cur.h"
#include "cell.h"

#include "my_def.h"

typedef multiset<ptr<path_pnt>, comp_f> path_list;

void sub_a_star(Cur& cur, dvec p, int dg, dvec p1,
	ptr<path_pnt> ppo, path_list& open, path_list& closed) {
	auto it = find_if(closed.begin(), closed.end(),
		[p](ptr<path_pnt> pp) { return pp->p == p; });
	if (it != closed.end()) { return; }

	it = find_if(open.begin(), open.end(),
		[p](ptr<path_pnt> pp) { return pp->p == p; });
	if (it != open.end()) {
		auto pp = *it;
		if (pp->g > ppo->g + dg) {
			open.erase(it);
			pp->g = ppo->g + dg; 
			pp->f = pp->g + pp->h;
			pp->root = &*ppo; open.insert(pp);
		} return;
	}

	auto pp = msh<path_pnt>(p);
	pp->h = geth(pp->p, p1);
	pp->g = ppo->g + dg;
	pp->f = pp->g + pp->h;
	pp->root = &*ppo; open.insert(pp);
}
vector<dvec> AStar(Cur& cur, dvec p0, dvec p1) {
	path_list open, closed;
	auto pp = msh<path_pnt>(p0);
	pp->h = geth(pp->p, p1);
	pp->f = pp->g + pp->h; open.insert(pp);

	path_pnt* end = NULL;
	while (1) {
		if (open.empty()) { return {}; }
		auto it = open.begin(); 
		auto ppo = *it;
		if (ppo->p == p1) { end = &*ppo; break; }

		open.erase(it);
		closed.insert(ppo);
#define TMP(x, y) dvec p = ppo->p + dvec(x, y); \
if (gtcl(p).walkable()) { \
		sub_a_star(cur, p, 10, p1, ppo, open, closed); \
}
		if (ppo->p.x != 0) { TMP(-1, 0); }
		if (ppo->p.x != nxgd - 1) { TMP(1, 0); }
		if (ppo->p.y != 0) { TMP(0, -1); }
		if (ppo->p.y != nygd - 1) { TMP(0, 1); }
#undef TMP
#define TMP(x, y) dvec p = ppo->p + dvec(x, y); \
dvec pa = ppo->p + dvec(x, 0), pb = ppo->p + dvec(0, y); \
if (gtcl(p).walkable() && gtcl(pa).walkable() && gtcl(pb).walkable()) { \
		sub_a_star(cur, p, 14, p1, ppo, open, closed); \
}
		if (ppo->p.x != 0 && ppo->p.y != 0) { TMP(-1, -1); }
		if (ppo->p.x != 0 && ppo->p.y != nygd - 1) { TMP(-1, 1); }
		if (ppo->p.x != nxgd - 1 && ppo->p.y != 0) { TMP(1, -1); }
		if (ppo->p.x != nxgd - 1 && ppo->p.y != nygd - 1) { TMP(1, 1); }
#undef TMP
	}

	vector<dvec> out = { end->p };
	while (end->root) {
		end = end->root; out.push_back(end->p);
	} return out;
}

void sub_dijkstra(Cur& cur, dvec p, int df,
	ptr<path_pnt> ppo, path_list& open, path_list& closed) {
	auto it = find_if(closed.begin(), closed.end(),
		[p](ptr<path_pnt> pp) { return pp->p == p; });
	if (it != closed.end()) { return; }

	it = find_if(open.begin(), open.end(),
		[p](ptr<path_pnt> pp) { return pp->p == p; });
	if (it != open.end()) {
		auto pp = *it;
		if (pp->f > ppo->f + df) {
			open.erase(it);
			pp->f = ppo->f + df;
			pp->root = &*ppo; open.insert(pp);
		} return;
	}

	auto pp = msh<path_pnt>(p);
	pp->f = ppo->f + df;
	pp->root = &*ppo; open.insert(pp);
}
vector<dvec> Dijkstra(Cur& cur, dvec p0, Faction* fac) {
	path_list open, closed;
	auto pp = msh<path_pnt>(p0); open.insert(pp);

	path_pnt* end = NULL;
	while (1) {
		if (open.empty()) { return {}; }
		auto it = open.begin();
		auto ppo = *it;
		if (gtcl(ppo->p).facs[fac]) { end = &*ppo; break; }

		open.erase(it);
		closed.insert(ppo);
#define TMP(x, y) dvec p = ppo->p + dvec(x, y); \
if (gtcl(p).walkable()) { \
		sub_dijkstra(cur, p, 10, ppo, open, closed); \
}
		if (ppo->p.x != 0) { TMP(-1, 0); }
		if (ppo->p.x != nxgd - 1) { TMP(1, 0); }
		if (ppo->p.y != 0) { TMP(0, -1); }
		if (ppo->p.y != nygd - 1) { TMP(0, 1); }
#undef TMP
#define TMP(x, y) dvec p = ppo->p + dvec(x, y); \
dvec pa = ppo->p + dvec(x, 0), pb = ppo->p + dvec(0, y); \
if (gtcl(p).walkable() && gtcl(pa).walkable() && gtcl(pb).walkable()) { \
		sub_dijkstra(cur, p, 14, ppo, open, closed); \
}
		if (ppo->p.x != 0 && ppo->p.y != 0) { TMP(-1, -1); }
		if (ppo->p.x != 0 && ppo->p.y != nygd - 1) { TMP(-1, 1); }
		if (ppo->p.x != nxgd - 1 && ppo->p.y != 0) { TMP(1, -1); }
		if (ppo->p.x != nxgd - 1 && ppo->p.y != nygd - 1) { TMP(1, 1); }
	}

	vector<dvec> out = { end->p };
	while (end->root) {
		end = end->root; out.push_back(end->p);
	} return out;
}
