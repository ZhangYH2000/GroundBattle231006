#include "builder.h"
#include "cur.h"
#include "bgr.h"
#include "arm.h"
#include "cell.h"
#include "agent.h"
#include "faction.h"
#include "terrain.h"
#include "draw_geo.h"

#include "my_def.h"

double constexpr dep_builder = 100;
double constexpr dep_tmp_cell = 0;

void Builder::render(Cur& cur) {
	switch (mode) {
	case BD_NONE:
		if (ms_in) {
			draw_rect(scr, dscr, dep_tmp_cell,
				tlcl(p_now), sgd, sgd, bgr.vp(), dcol(0, 0, 255));
		} break;
	case BD_RECT: render_rect(cur); break;
	case BD_LINE: render_line(cur); break;
	}
}
void Builder::render_rect(Cur& cur) {
	int x0 = min(p_now.x, p_rec.x);
	int x1 = max(p_now.x, p_rec.x);
	int y0 = min(p_now.y, p_rec.y);
	int y1 = max(p_now.y, p_rec.y);
	dvec tl = tlcl(dvec(x0, y0));
	draw_rect(scr, dscr, dep_tmp_cell, tl, 
		sgd * (x1 - x0 + 1), sgd * (y1 - y0 + 1), bgr.vp(), dcol(255, 0, 0));
}
void Builder::render_line(Cur& cur) {
	int x0 = min(p_now.x, p_rec.x);
	int x1 = max(p_now.x, p_rec.x);
	int y0 = min(p_now.y, p_rec.y);
	int y1 = max(p_now.y, p_rec.y);
	dcol c = dcol(255, 0, 0);
	if (y1 - y0 < x1 - x0) {
		dvec tl = (dvec)tl_cell(cur, dvec(x0, p_rec.y));
		draw_rect_raw(scr, tl, sgd * (x1 - x0 + 1), sgd, bgr.vp(), c);
	} else {
		dvec tl = (dvec)tl_cell(cur, dvec(p_rec.x, y0));
		draw_rect_raw(scr, tl, sgd, sgd * (y1 - y0 + 1), bgr.vp(), c);
	}
}
void Builder::edit_rect(Cur& cur) {
	auto t = cur.terrain_sel;
	if (!t) { return; }

	mode = BD_NONE;
	int x0 = min(p_now.x, p_rec.x);
	int x1 = max(p_now.x, p_rec.x);
	int y0 = min(p_now.y, p_rec.y);
	int y1 = max(p_now.y, p_rec.y);
	rep(x, x0, x1 + 1) 
	rep(y, y0, y1 + 1) { gtcl(dvec(x, y)).set_terrain(cur,t); }
}
void Builder::edit_line(Cur& cur) {
	auto t = cur.terrain_sel;
	if (!t) { return; }

	mode = BD_NONE;
	int x0 = min(p_now.x, p_rec.x);
	int x1 = max(p_now.x, p_rec.x);
	int y0 = min(p_now.y, p_rec.y);
	int y1 = max(p_now.y, p_rec.y);
	if (y1 - y0 < x1 - x0) {
		rep(x, x0, x1 + 1) { gtcl(dvec(x, p_rec.y)).set_terrain(cur, t); }
	} else {
		rep(y, y0, y1 + 1) { gtcl(dvec(p_rec.x, y)).set_terrain(cur, t); }
	}
}
void Builder::Update(Cur& cur) {
	if (cur.cast || !cur.edit) { return; }
	double x = (msp.x - bgr.tl.x - tlgd.x) / sgd;
	double y = (msp.y - bgr.tl.y - tlgd.y) / sgd;
	p_now = dvec(floor(x), floor(y));
	ms_in = insd(p_now.x, 0, nxgd) && insd(p_now.y, 0, nygd);
	p_now.x = clmp(p_now.x, 0, nxgd - 1);
	p_now.y = clmp(p_now.y, 0, nygd - 1);
	hovered = (hvd == this);

	if (hovered && ms_in && msc(0)) { 
		if (cur.terrain_sel) {
			p_rec = p_now; 
			mode = cur.terrain_sel->wall ? BD_LINE : BD_RECT;
		}
		if (cur.arm_sel && cur.faction_sel) {
			auto a = msh<Agent>(
				cur.arm_sel, cur.faction_sel, vec2(x, y));
			cur.agents.push_back(a);
		}
	}

	if (!msd[0]) {
		switch (mode) {
		case BD_RECT: edit_rect(cur); break;
		case BD_LINE: edit_line(cur); break;
		}
	}

	render(cur);
}
void Builder::Discard(Cur& cur) {
	mode = BD_NONE; rmv;
}
void Builder::PreUpdate(Cur& cur) {
	if (cur.cast || !cur.edit) { return; }
	bool ok = dhv < dep_builder&& insd(msp, bgr.vp());
	if (ok) { dhv = dep_builder; hvd = this; }
}
