#include "cell.h"
#include "cur.h"
#include "bgr.h"
#include "builder.h"
#include "terrain.h"
#include "draw_geo.h"

#include "my_def.h"

// 很奇怪，怎么处理这里与贴图。
// 如果要改动这个量的话，应当要重置整个网格。
// 事实上，地形也会受到影响。
int h_wall = 860;
int wh_cell = 860;
int s_grid = 35;
double constexpr dep_cell = 0;

Cell::Cell(Cur& cur, dvec pgd) : pgd(pgd) {
	set_terrain(cur, &*cur.terrains[0]);
}
Cell::Cell(Cur& cur, dvec pgd, FILE* f) : pgd(pgd) {
	int id; frd(id);
	if (id < cur.terrains.size()) {
		set_terrain(cur, &*cur.terrains[id]);
	} else {
		set_terrain(cur, &*cur.terrains[0]);
	}
}
void Cell::Save(FILE* f) {
	fwt(tr->id);
}
bool Cell::walkable() const {
	return !tr->wall && tr->walkable;
}

void set_cell_tile(Cur& cur, vector<cast_tile> const& imgs_x0, ptr<tile>& tx0, bool wall = true) {
	tx0 = NULL;
	double p = frnd(1);
	for (auto& t : imgs_x0) {
		p -= t.p;
		if (p < 0) {
			tx0 = cur.get_tile(t.nm);
			if (tx0) {
				bool bad = false;
				if (wall) {
					bad = tx0->w != wh_cell || tx0->h != h_wall;
				} else {
					bad = tx0->w != wh_cell || tx0->h != wh_cell;
				}
				if (bad) { tx0 = NULL; }
			} return;
		}
	}
}
void Cell::set_terrain(Cur& cur, Terrain* _tr) {
	tr = _tr; double r = 0;
#define TMP(cx0) r = tr->cx0##_rnd; \
cx0 = dcol(tr->cx0 + col3(frnd(-r, r), frnd(-r, r), frnd(-r, r)));
	TMP(cx0); TMP(cx1); TMP(cy0); TMP(cy1); TMP(cz0); TMP(cz1);

#undef TMP
#define TMP(x0) set_cell_tile(cur, tr->imgs_##x0, t##x0);
	TMP(x0); TMP(x1); TMP(y0); TMP(y1); 
#undef TMP
#define TMP(x0) set_cell_tile(cur, tr->imgs_##x0, t##x0, false);
	TMP(z0); TMP(z1);
#undef TMP
}

void Cell::CheckDead(Cur& cur) {
	if (tr->dead) { tr = &*cur.terrains[0]; }
}
void Cell::Reset(Cur& cur) {
	agents.clear();
	facs.clear();
	for (auto f : cur.factions) { facs[&*f] = false; }
}
void Cell::RenderFlat(Cur& cur) {
	draw_rect_raw(scr, tlcl(pgd), sgd, sgd, bgr.vp(), tr->c_flat);
}

Cell& get_cell(Cur& cur, dvec pgd) {
	int idx = pgd.y * nxgd + pgd.x;
	return *gd[idx];
}
dvec tl_cell(Cur& cur, dvec pgd) {
	return bgr.tl + pgd * sgd + (dvec)tlgd;
}
void set_grid_size(Cur& cur, int nx, int ny) {
	if (nx <= 0 || ny <= 0) { return; }
	nxgd = nx; nygd = ny; gd.clear();
	rep(j, 0, nygd) rep(i, 0, nxgd) {
		gd.push_back(msh<Cell>(cur, dvec(i, j)));
	}
	mkp(cur.builder)();
}
