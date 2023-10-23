#include "bullet.h"
#include "cur.h"
#include "cam.h"
#include "cell.h"
#include "cast.h"
#include "agent.h"
#include "faction.h"
#include "broadcast.h"

#include "my_def.h"

#define r_agent (cur.r_agent)
#define top_bullet (cur.top_bullet)

Bullet::Bullet(vec2 o, vec2 v, Faction* fac, double t_life) :
	o(o), v(v), fac(fac), t_life(t_life) {}
void Bullet::Cast(Cur& cur) {
	if (!t || d < d_close) { return; }

	double x = dot(o - cm.o, cm.vx);
	double y = cm.h - top_bullet;
	int h = cur.h_bullet / d * cm.scl;
	int w = h * t->w / t->h;
	dvec pc = cm.ct + dvec(vec2(x, y) / d * cm.scl);

	int x0 = pc.x - w / 2;
	int y0 = pc.y - h / 2;
	int da = max(0, x0);
	int db = min(nxcv, x0 + w);
	int dc = max(0, y0);
	int dd = min(nycv, y0 + h);
	if (db <= da || dd <= dc) { return; }

	int sc = (dc - y0) * t->h / h;
	int sd = (dd - y0) * t->h / h;
	int dh = dd - dc;
	int sh = sd - sc;
	rep(i, da, db) {
		auto& rc = cur.recs[i];
		if (rc->hit && rc->d <= d) { continue; }
		int x = (i - x0) * t->w / w;
		int dp = dc * nxcv + i;
		int sp = sc * t->w + x;
		int e = dh;
		bool ok = t->as[sp];
		dcol col = t->cols[sp];
		rep(j, dc, dd) {
			while (e <= 0) {
				e += dh; sp += t->w;
				col = t->cols[sp]; ok = t->as[sp];
			}
			if (ok) { cv[dp] = col; }
			e -= sh; dp += nxcv;
		}
	}
};

void Bullet::CheckDead(Cur& cur) {
	if (fac->dead) { dead = true; }
}
void Bullet::RemoveDead(Cur& cur) {
	if (owner && owner->dead) { owner = NULL; }
}
void Bullet::Update(Cur& cur) {
	d = dot(o - cm.o, cm.vy);
	if (cur.paused) { return; }

	vec2 nxt = o + rdt * v;
	int x0 = min(nxt.x, o.x);
	int x1 = max(nxt.x, o.x) + 1;
	int y0 = min(nxt.y, o.y);
	int y1 = max(nxt.y, o.y) + 1;
	x0 = clmp(x0, 0, nxgd);
	x1 = clmp(x1, 0, nxgd);
	y0 = clmp(y0, 0, nygd);
	y1 = clmp(y1, 0, nygd);

	double len = (nxt - o).len();
	vec2 u = v.unit();
	Agent* a = NULL;
	double t = len;
	rep(i, x0, x1) rep(j, y0, y1) {
		auto& cell = gtcl(dvec(i, j));
		if (!cell.facs[fac]) { continue; }
		for (auto _a : cell.agents) {
			if (_a->fac == fac) { continue; }

			vec2 oa = _a->o - o;
			double _t = dot(oa, u);
			double bias = (oa - _t * u).lensqr();
			if (bias < r_agent * r_agent)
			if (insd(_t, 0.0, t)) { t = _t; a = _a; }
		}
	}

	if (a) {
		dead = true;
		bool alive = (a->health >= 0);
		a->health -= dmg;
		a->t_receive = max_t_receive;

		if (owner && owner->selected && cur.cast) {
			wstring s =
				loc(L"send") + tw2(dmg) + loc(L"ranged_dmg");
			bdc.add_msg(s, { 10, 250, 100 });
		}

		if (a->selected && cur.cast) {
			wstring s =
				loc(L"receive") + tw2(dmg) + loc(L"ranged_dmg");
			bdc.add_msg(s, { 250, 150, 10 });
		}

		if (alive && owner && a->health < 0) { ++owner->n_kill; }
		if (alive && a->health < 0 && owner && owner->selected && cur.cast) {
			wstring s = loc(L"kill");
			bdc.add_msg(s, { 255, 255, 10 });
		}
	}

	o = nxt;
	t_life -= rdt;
	if (t_life < 0) { dead = true; }
};
