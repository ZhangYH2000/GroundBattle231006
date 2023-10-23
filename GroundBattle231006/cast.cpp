#include "cast.h"
#include "cam.h"
#include "cur.h"
#include "bgr.h"
#include "cell.h"
#include "terrain.h"

#include "my_def.h"

double d_close = 0.1;

void sub_ray_cast(double dx, double dy,
	double& lx, double& ly, int sx, int sy, cast_rec& rc) {
	double tx = lx / dx, ty = ly / dy;
	// 下面是为了避免 dy 和 ly 都是 0 的情形，确实会发生。
	rc.is_y = dy == 0 || tx < ty;
	if (rc.is_y) {
		lx = 1;
		ly = abs(ly - tx * dy);
		rc.pgd.x += sx; rc.t += tx;
	} else {
		ly = 1;
		lx = abs(lx - ty * dx);
		rc.pgd.y += sy; rc.t += ty;
	}
}
cast_rec RayCast(Cur& cur, vec2 v) {
	double dx = abs(v.x), dy = abs(v.y);
	int sx = v.x > 0 ? 1 : -1;
	int sy = v.y > 0 ? 1 : -1;
	// 这里假设相机落在网格里面。
	int px = cm.o.x; 
	double lx = cm.o.x - px;
	if (sx == 1) { lx = 1 - lx; }
	int py = cm.o.y;
	double ly = cm.o.y - py;
	if (sy == 1) { ly = 1 - ly; }

	cast_rec rc;
	rc.pgd = dvec(px, py);
	int bx = (sx == 1) ? nxgd : -1;
	int by = (sy == 1) ? nygd : -1;

	while (1) {
		sub_ray_cast(dx, dy, lx, ly, sx, sy, rc);
		if (rc.pgd.x != bx && rc.pgd.y != by) {
			auto& cell = get_cell(cur, rc.pgd);
			if (cell.tr->wall) {
				rc.hit = true;
#define TMP(x0) rc.c_wall = cell.c##x0; \
if (cell.t##x0) { rc.t_wall = cell.t##x0; }

				if (rc.is_y && sx == 1) { TMP(x0); }
				else if (rc.is_y && sx == -1) { TMP(x1); }
				else if (!rc.is_y && sy == 1) { TMP(y0); }
				else if (!rc.is_y && sy == -1) { TMP(y1); }
			} else { continue; }
		}

		auto p = (vec2)rc.pgd;
		double ux = 0;
		if (rc.is_y) {
			ux = (sy == sx) ? 1 - ly : ly;
			p.y += (sy == 1) ? 1 - ly : ly;
			if (sx == -1) { p.x += 1; }
		} else {
			ux = (sy != sx) ? 1 - lx : lx;
			p.x += (sx == 1) ? 1 - lx : lx;
			if (sy == -1) { p.y += 1; }
		}
		rc.d = max(1e-2, dot(p - cm.o, cm.vy));
		rc.ux = clmp(int(ux * wh_cell), 0, wh_cell - 1);
		if (rc.hit) {
			rc.y0 = cm.ct.y - (cur.h_wall - cm.h) * cm.scl / rc.d;
			rc.y1 = cm.ct.y + cm.h * cm.scl / rc.d; 
		} else { rc.y0 = rc.y1 = cm.ct.y; }
		return rc;
	}
}

void CastWalls(Cur& cur) {
	cur.recs.clear();
	rep(i, cm.vp.left(), cm.vp.right()) {
		auto v = cm.vy * cm.scl + cm.vx * (i - cm.ct.x);
		v = v.unit();
		auto rc = RayCast(cur, v);
		cur.recs.push_back(msh<cast_rec>(rc));

		if (rc.hit) {
			int top = max(cm.vp.top(), rc.y0);
			int bottom = min(cm.vp.bottom(), rc.y1);
			int dp = top * nxcv + i;

			if (rc.t_wall) {
				int dy = h_wall;
				int h = rc.y1 - rc.y0;
				int uy = h_wall * (top - rc.y0) / h;
				int sp = uy * wh_cell + rc.ux;
				int e = h; 
				dcol c = rc.t_wall->cols[sp];
				rep(j, top, bottom) {
					while (e <= 0) { 
						e += h; sp += wh_cell; 
						c = rc.t_wall->cols[sp];
					} cv[dp] = c; e -= dy; dp += nxcv;
				}
			} else {
				rep(j, top, bottom) {
					cv[dp] = rc.c_wall; dp += nxcv;
				}
			}
		}
	}
}
void CastCeils(Cur& cur) {
	if (cur.simple_ceil) {
		auto tmp = vector<dcol>(nxcv, cur.c_ceil);
		int sz = sizeof(dcol) * nxcv;
		rep(j, cm.vp.top(), cm.ct.y) {
			int dp = j * nxcv;
			memcpy(&cv[dp], tmp.data(), sz);
		}  return;
	}

	rep(j, cm.vp.top(), cm.ct.y - cur.gap_ceil) {
		// 下面是为了不除以 0。
		double t = (cur.h_wall - cm.h) / (cm.ct.y - j) * cm.scl;
		double _x = cm.vp.w / 2 * t / cm.scl;
		vec2 p0 = cm.o + t * cm.vy - _x * cm.vx;
		vec2 p1 = cm.o + t * cm.vy + _x * cm.vx;
		dvec pgd = dvec(floor(p0.x), floor(p0.y));
		int dx = abs(p1.x - p0.x) * wh_cell;
		int dy = abs(p1.y - p0.y) * wh_cell;
		int sx = p1.x > p0.x;
		int sy = p1.y > p0.y;
		int ex = (pgd.x + 1 - p0.x) * wh_cell;
		int ey = (pgd.y + 1 - p0.y) * wh_cell;
		Cell* c = get_cell_test(cur, pgd);

		// 有可能出界吗，我不太懂浮点数。
		int x = (p0.x - pgd.x) * wh_cell;
		x = clmp(x, 0, wh_cell - 1);
		int y = (p0.y - pgd.y) * wh_cell;
		y = clmp(y, 0, wh_cell - 1);
		int sp = y * wh_cell + x;
		int dp = j * nxcv;

		rep(i, cm.vp.left(), cm.vp.right()) {
			if (c) { cv[dp++] = c->tz1 ? c->tz1->cols[sp] : c->cz1; }
			else { cv[dp++] = cur.c_ceil; }
			sub_cast_floors(cur, ex, ey, x, y, dx, dy, pgd, c, sx, sy, dp, sp);
		}
	}
}
void CastCeilsClose(Cur& cur) {
	int beg = max(cm.vp.top(), cm.ct.y - cur.gap_ceil);
	rep(i, cm.vp.left(), cm.vp.right()) {
		// 在击中边界是 rec 的 y0, y1 是什么值很重要。
		int end = min(cur.recs[i]->y0, cm.ct.y);
		int dp = beg * nxcv + i;
		rep(j, beg, end) {
			double t = (cur.h_wall - cm.h) / (cm.ct.y - j) * cm.scl;
			double x = (i - cm.ct.x) * t / cm.scl;
			vec2 p = cm.o + t * cm.vy + x * cm.vx;
			dvec pgd = dvec(floor(p.x), floor(p.y));
			Cell* c = get_cell_test(cur, pgd);

			if (!c) { cv[dp] = cur.c_floor; }
			else {
				int ux = (p.x - pgd.x) * wh_cell;
				ux = clmp(ux, 0, wh_cell);
				int uy = (p.y - pgd.y) * wh_cell;
				uy = clmp(uy, 0, wh_cell);
				int sp = uy * wh_cell + ux;
				cv[dp] = c->tz1 ? c->tz1->cols[sp] : c->cz1;
			} dp += nxcv;
		}
	}
}
void CastFloors(Cur& cur) {
	if (cur.simple_floor) {
		auto tmp = vector<dcol>(nxcv, cur.c_floor);
		int sz = sizeof(dcol) * nxcv;
		rep(j, cm.ct.y, cm.vp.bottom()) {
			int dp = j * nxcv;
			memcpy(&cv[dp], tmp.data(), sz);
		} return;
	}

	rep(j, cm.ct.y + cur.gap_floor, cm.vp.bottom()) {
		// 下面是为了不除以 0。
		double t = cm.h / (j - cm.ct.y + 1) * cm.scl;
		double _x = cm.vp.w / 2 * t / cm.scl;
		vec2 p0 = cm.o + t * cm.vy - _x * cm.vx;
		vec2 p1 = cm.o + t * cm.vy + _x * cm.vx;
		dvec pgd = dvec(floor(p0.x), floor(p0.y));
		int dx = abs(p1.x - p0.x) * wh_cell;
		int dy = abs(p1.y - p0.y) * wh_cell;
		int sx = p1.x > p0.x;
		int sy = p1.y > p0.y;
		int ex = (pgd.x + 1 - p0.x) * wh_cell;
		int ey = (pgd.y + 1 - p0.y) * wh_cell;
		Cell* c = get_cell_test(cur, pgd);

		// 有可能出界吗，我不太懂浮点数。
		int x = (p0.x - pgd.x) * wh_cell;
		x = clmp(x, 0, wh_cell - 1);
		int y = (p0.y - pgd.y) * wh_cell;
		y = clmp(y, 0, wh_cell - 1);
		int sp = y * wh_cell + x;
		int dp = j * nxcv;

		rep(i, cm.vp.left(), cm.vp.right()) {
			if (c) { cv[dp++] = c->tz0 ? c->tz0->cols[sp] : c->cz0; }
			else { cv[dp++] = cur.c_floor; }
			sub_cast_floors(cur, ex, ey, x, y, dx, dy, pgd, c, sx, sy, dp, sp);
		}
	}
}
void CastFloorsClose(Cur& cur) {
	int end = min(cm.vp.bottom(), cm.ct.y + cur.gap_floor);
	rep(i, cm.vp.left(), cm.vp.right()) {
		int beg = max(cur.recs[i]->y1, cm.ct.y);
		int dp = beg * nxcv + i;
		rep(j, beg, end) {
			double t = cm.h / (j - cm.ct.y + 1) * cm.scl;
			double x = (i - cm.ct.x) * t / cm.scl;
			vec2 p = cm.o + t * cm.vy + x * cm.vx;
			dvec pgd = dvec(floor(p.x), floor(p.y));
			Cell* c = get_cell_test(cur, pgd);

			if (!c) { cv[dp] = cur.c_floor; }
			else {
				int ux = (p.x - pgd.x) * wh_cell;
				ux = clmp(ux, 0, wh_cell);
				int uy = (p.y - pgd.y) * wh_cell;
				uy = clmp(uy, 0, wh_cell);
				int sp = uy * wh_cell + ux;
				cv[dp] = c->tz0 ? c->tz0->cols[sp] : c->cz0;
			} dp += nxcv;
		}
	}
}

void DrawCanvas(Cur& cur) {
	if (bgr.w == nxcv && bgr.h == nycv) {
		int sz_row = sizeof(dcol) * nxcv;
		rep(j, 0, bgr.h) {
			int dp = (j + bgr.tl.y) * scr.w + bgr.tl.x;
			int sp = j * bgr.w;
			memcpy(scr.cols.data() + dp, cv.data() + sp, sz_row);
		} return;
	}

	int dx = nxcv, w = bgr.w;
	rep(j, 0, bgr.h) {
		int y = j * nycv / bgr.h;
		int sp = y * nxcv;
		int dp = (j + bgr.tl.y) * scr.w + bgr.tl.x;
		int e = w; dcol c = cv[sp];
		rep(i, 0, bgr.w) {
			if (e <= 0) { e += w; c = cv[++sp]; }
			e -= dx; scr.cols[dp++] = c;
		}
	}
}

Cell* get_cell_test(Cur& cur, dvec pgd) {
	if (!insd(pgd.x, 0, nxgd) || !insd(pgd.y, 0, nygd)) { return NULL; }
	return &get_cell(cur, pgd);
}
void sub_cast_floors(Cur& cur, int& ex, int& ey, int& x, int& y,
	int dx, int dy, dvec& pgd, Cell*& c, int sx, int sy, int& dp, int& sp) {
	
	// 这种这种累加 ex 的方法似乎比整数求余要快很多。
	// 我仍然不知道最快的实现是啥。
	ex -= dx; ey -= dy;
	while (ex < 0) {
		if (sx == 1) {
			++x; ++sp;
			if (x == wh_cell) {
				x = 0; ++pgd.x; sp -= wh_cell;
				c = get_cell_test(cur, pgd);
			}
		} else {
			--x; --sp;
			if (x == -1) {
				x = wh_cell - 1; --pgd.x; sp += wh_cell;
				c = get_cell_test(cur, pgd);
			}
		} ex += nxcv;
	}
	while (ey < 0) {
		if (sy == 1) {
			++y; sp += wh_cell;
			if (y == wh_cell) {
				y = 0; ++pgd.y; sp -= wh_cell * wh_cell;
				c = get_cell_test(cur, pgd);
			}
		} else {
			--y; sp -= wh_cell;
			if (y == -1) {
				y = wh_cell - 1; --pgd.y; sp += wh_cell * wh_cell;
				c = get_cell_test(cur, pgd);
			}
		} ey += nxcv;
	}
}

bool VisibleTest(Cur& cur, vec2 o, vec2 v, double t, bool& reachable) {
	double dx = abs(v.x), dy = abs(v.y);
	int sx = v.x > 0 ? 1 : -1;
	int sy = v.y > 0 ? 1 : -1;
	// 这里假设相机落在网格里面。
	int px = o.x;
	double lx = o.x - px;
	if (sx == 1) { lx = 1 - lx; }
	int py = o.y;
	double ly = o.y - py;
	if (sy == 1) { ly = 1 - ly; }

	cast_rec rc;
	rc.pgd = dvec(px, py);
	int bx = (sx == 1) ? nxgd : -1;
	int by = (sy == 1) ? nygd : -1;

	while (1) {
		sub_ray_cast(dx, dy, lx, ly, sx, sy, rc);
		if (rc.t > t) { return true; }
		bool ok = rc.pgd.x != bx && rc.pgd.y != by;
		if (!ok) { reachable = false; return false; }
		auto& cell = gtcl(rc.pgd);
		if (!cell.walkable()) { reachable = false; }
		if (cell.tr->wall) { return false; }
	}
}
double TimeHit(Cur& cur, vec2 o, vec2 v) {
	double dx = abs(v.x), dy = abs(v.y);
	int sx = v.x > 0 ? 1 : -1;
	int sy = v.y > 0 ? 1 : -1;
	// 这里假设相机落在网格里面。
	int px = o.x;
	double lx = o.x - px;
	if (sx == 1) { lx = 1 - lx; }
	int py = o.y;
	double ly = o.y - py;
	if (sy == 1) { ly = 1 - ly; }

	cast_rec rc;
	rc.pgd = dvec(px, py);
	int bx = (sx == 1) ? nxgd : -1;
	int by = (sy == 1) ? nygd : -1;

	while (1) {
		sub_ray_cast(dx, dy, lx, ly, sx, sy, rc);
		bool ok = rc.pgd.x != bx && rc.pgd.y != by;
		if (!ok || gtcl(rc.pgd).tr->wall) { return rc.t; }
	}
}
void set_resolution(Cur& cur, int nx, int ny) {
	// 忘记了是否一定要求要比背景的尺寸小了。
	nx = clmp(nx, 1, bgr.w - 1);
	ny = clmp(ny, 1, bgr.h - 1);
	nxcv = nx; nycv = ny;
	cv.clear(); cv.resize(nx * ny);
}
