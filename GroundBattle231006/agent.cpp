#include "agent.h"
#include "cur.h"
#include "bgr.h"
#include "cam.h"
#include "arm.h"
#include "cell.h"
#include "cast.h"
#include "path.h"
#include "bullet.h"
#include "faction.h"
#include "broadcast.h"
#include "draw_geo.h"
#include "draw_str.h"
#include "draw_comp.h"
#include "draw_tile.h"
#include "draw_px_seg.h"

#include "my_def.h" 

double constexpr dep_normal = 3;
double constexpr eps = 1e-2;
double constexpr ang_acc = 15;
double constexpr max_v_ang = 1.5;
double constexpr r_path = 0.3;
double constexpr max_t_path = 2;
double constexpr max_dur_found = 6;
double constexpr min_dur_found = 3;
double constexpr max_dur_not_found = 4;
double constexpr min_dur_not_found = 2;
double constexpr max_dur_dead = 1.5;
double constexpr dist_melee = 0.6;
double constexpr dif_ang_melee = 0.6;
double constexpr max_t_send = 0.12;
double max_t_receive = 0.09;
double constexpr dist_ranged = 8;
double constexpr mtp_dif_ang_ranged = 1.1;
double constexpr v_bullet = 8.4;
double constexpr max_t_bullet = 2;
double constexpr max_t_wonder = 3;

Agent::Agent(Arm* arm, Faction* fac, vec2 o) : 
	arm(arm), fac(fac), o(o) {
	health = 1;
	t_target = frnd(max_dur_dead);
	t_wonder = frnd(max_t_wonder);
}
Agent::Agent(Cur& cur, FILE* f) {
	int id = 0;
	frd(id); 
	if (id < cur.arms.size()) {
		arm = &*cur.arms[id];
	}
	frd(id);
	if (id < cur.factions.size()) {
		fac = &*cur.factions[id];
	}
	frd(o); frd(health);
	t_target = frnd(max_dur_dead);
	t_wonder = frnd(max_t_wonder);
}
void Agent::Save(FILE* f) {
	fwt(arm->id); fwt(fac->id); fwt(o); fwt(health);
}
double Agent::dep() const {
	auto out = dep_normal;
	if (hovered) { out += 2; }
	if (selected) { out += 1; } return out;
}

#define acc (cur.acc)
#define max_v (cur.max_v)
#define r_agent (cur.r_agent)

void Agent::slow_down(Cur& cur) {
	double dv = acc * rdt;
	double len = v.len();
	if (len < dv) { v = {}; }
	else { v -= v.unit() * dv; }
}
void Agent::turn_to(Cur& cur, double tar_ang, double lim) {
	// 这里用未来角度真的会变好一点吗。
	// 感觉这个版本跟随时视野转换会有小突变，不知道之前有没有。
	// 实验发现，这是角速度的加速度太大的原因。
	// 肯定是需要更好的转向算法的。
	double nxt = ang + v_ang * rdt;
	double dif = modf(tar_ang - nxt + PI, 2 * PI) - PI;
	double max_da = ang_acc * rdt;
	if (abs(dif) < lim) {
		double da = min(abs(v_ang), max_da);
		v_ang -= (v_ang > 0 ? 1 : -1) * da;
	} else {
		v_ang += dif > 0 ? max_da : -max_da;
	}
}
void Agent::move_to(Cur& cur, vec2 p) {
	// 这里假定 p 与 o 不会挨着很近了。
	// 这么做跟直接往未来目标加速有啥区别。
	// 不知道这里能不能用 verlet 方法。
	vec2 nxt = o + v * rdt;
	vec2 goal = o + max_v * (p - o).unit() * rdt;
	vec2 u = (goal - nxt).unit();

	// 像下面这么做真的有必要吗，如何避免浮点数溢出。
	double len = (goal - nxt).len();
	if (len < 1e-3) { return; }
	v += (goal - nxt) / len * acc * rdt;
}
void Agent::move_along(Cur& cur, vec2 p0, vec2 p1) {
	// 不知道使用未来的位置有什么优越性，是否合适。
	vec2 u = (p1 - p0).unit();
	if (dot(o - p1, u) > 0) { move_to(cur, p1); return; }
	vec2 nxt = o + v * rdt - p0;

	vec2 bias = nxt - u * dot(nxt, u);
	if (bias.lensqr() > r_path * r_path) {
		v -= bias * acc * rdt; return;
	}

	vec2 goal = o + u * max_v * rdt - p0;
	double len = (goal - nxt).len();
	if (len < 1e-3) { return; }
	v += (goal - nxt) / len * acc * rdt;
}

double calc_d(vec2 o, vec2 p, vec2 n, double r) {
	double d = -DBL_MAX, _d = 0;
#define TMP(x, y) _d = r - dot(n, o - p - vec2(x, y)); \
if (_d > d) { d = _d; }
	TMP(0, 0); TMP(0, 1); TMP(1, 0); TMP(1, 1); return d;
#undef TMP
}
void Agent::collide_cell(Cur& cur, dvec p) {
	bool bad = 
		insd(p.x, 0, nxgd) && insd(p.y, 0, nygd) && gtcl(p).walkable();
	if (bad) { return; }
	vec2 n, _n, po;
	double d = DBL_MAX, _d = 0;
	
	// 这里相当于是看成正方体之间的碰撞了。
	// 但这样可能在墙角产生突变。
#define TMP if (_d < d) { d = _d; n = _n; }
	_d = o.x + r_agent - p.x;
	_n = vec2(-1, 0); TMP;
	_d = o.y + r_agent - p.y;
	_n = vec2(0, -1); TMP;
	_d = p.x + 1 - o.x + r_agent;
	_n = vec2(+1, 0); TMP;
	_d = p.y + 1 - o.y + r_agent;
	_n = vec2(0, +1); TMP;
#undef TMP
#define TMP(x, y) po = o - vec2(p) - vec2(x, y); \
_n = po.unit(); \
_d = calc_d(o, (vec2)p, po.unit(), r_agent); \
if (_d < d) { d = _d; n = _n; }
	TMP(0, 0); TMP(0, 1); TMP(1, 0); TMP(1, 1);
#undef TMP

	if (d > -0.15) { v += 1 * rdt * n; }
	if (d > 0) {
		o += n * d;
		double vn = dot(v, n);
		if (vn < 0) { v -= n * vn; }
	}
}
void Agent::collide_wall(Cur& cur) {
	if (!gtcl(pgd).walkable()) {
		o = vec2(frnd(nxgd), frnd(nygd)); 
		teleported = true; return;
	}

	collide_cell(cur, pgd - dvec(1, 0));
	collide_cell(cur, pgd + dvec(1, 0));
	collide_cell(cur, pgd - dvec(0, 1));
	collide_cell(cur, pgd + dvec(0, 1));
}

void Agent::CheckDead(Cur& cur) {
	if (arm->dead || fac->dead) { dead = true; }
}
void Agent::RemoveDead(Cur& cur) {
	if (tar && tar->dead) { 
		tar = NULL; 
		t_wonder = frnd(max_t_wonder);
		t_target = frnd(max_dur_dead); 
	}
}
void Agent::RefreshGridFac(Cur& cur) {
	pgd.x = clmp((int)floor(o.x), 0, cur.nx_grid - 1);
	pgd.y = clmp((int)floor(o.y), 0, cur.ny_grid - 1);
	auto& cell = gtcl(pgd);
	cell.agents.insert(this); 
	for (auto &f : cell.facs) 
	if (f.first != fac) { f.second = true; }
	
	fac->cnt++;
	for (auto& f : cur.factions) 
	if (&*f != fac) { f->cnt_enemy++; }
}
void Agent::RenderFlat(Cur& cur) {
	if (selected) for (auto p : path) {
		auto pos = (vec2)tlcl(p) + vec2(sgd) / 2;
		draw_eclipse(scr, dscr, 1, pos, 3, 3, bgr.vp(), dcol(255, 0, 0));
	}

	dcol c = fac->col;
	vec2 p = (vec2)bgr.tl + tlgd + sgd * o;
	double r = r_agent * sgd;
	draw_eclipse(scr, dscr, dep(), p, r, r, bgr.vp(), c);
	draw_eclipse_frame(scr, dscr, dep(), p, r, r, bgr.vp(), dcol(255), 20);

	if (!selected && !hovered) { return; }
	dcol c_circ = selected ? dcol(255, 60, 60) : dcol(150, 150, 255);
	double r_circ = r + 5;
	draw_eclipse_frame(scr, dscr, dep(), p, r_circ, r_circ, bgr.vp(), c_circ, 20);
}

void Agent::Update(Cur& cur) {
	teleported = false;
	hovered = (hvd == this);
	selected = (agsel == this);
	// 各种默认满足的条件真是太乱了。
	o.x = clmp<double>(o.x, eps, nxgd - eps);
	o.y = clmp<double>(o.y, eps, nygd - eps);
	d = dot(o - cm.o, cm.vy);
	if (hovered && msc(0)) { agsel = this; }
	if (hovered && msc(2)) { dead = true; }
	if (cur.paused) { return; }

	if (selected && cur.player) {
		tar = NULL;
		path.clear();

		if (t_attack < 0 && msc(0)) {
			if (!arm->ranged) {
				t_send = max_t_send;
				t_attack = arm->max_t_attack;

				int x0 = max(0, pgd.x - 1);
				int x1 = min(nxgd, pgd.x + 2);
				int y0 = max(0, pgd.y - 1);
				int y1 = max(nygd, pgd.y + 2);

				Agent* enemy = NULL;
				double close = DBL_MAX;

				rep(i, x0, x1) rep(j, y0, y1) {
					auto& cell = gtcl(dvec(i, j));
					for (auto a : cell.agents) if (a->fac != fac) {
						vec2 ot = a->o - o;
						double dist = ot.len();

						if (dist > 1e-3) {
							ot /= dist;
							double tar_ang = atan2(ot.y, ot.x);
							double dif = modf(tar_ang - ang + PI, 2 * PI) - PI;

							bool attack = dist < close &&
								abs(dif) < dif_ang_melee&& dist < dist_melee;
							if (attack) { close = dist; enemy = a; }
						}
					}
				}

				if (enemy) {
					bool alive = (enemy->health >= 0);
					enemy->health -= arm->dmg;
					enemy->t_receive = max_t_receive;
					t_send = max_t_send;
					t_attack = arm->max_t_attack;

					if (cur.cast) {
						wstring s =
							loc(L"send") + tw2(arm->dmg) + loc(L"melee_dmg");
						bdc.add_msg(s, { 10, 250, 100 });
					}

					if (cur.cast) {
						wstring s =
							loc(L"receive") + tw2(arm->dmg) + loc(L"melee");
						bdc.add_msg(s, { 250, 150, 10 });
					}

					if (alive && tar->health < 0) { ++n_kill; }
					if (alive && enemy->health < 0 && cur.cast) {
						wstring s = loc(L"kill");
						bdc.add_msg(s, { 255, 255, 10 });
					}
				}
			} else {
				t_send = max_t_send;
				t_attack = arm->max_t_attack;

				// 这里比较乱。以后不要子弹构造函数了。
				// 用一个单独方法创建子弹。
				vec2 front = vec2(cos(ang), sin(ang));
				vec2 vb = front * v_bullet;
				double t_life = min(max_t_bullet, TimeHit(cur, o, vb));
				auto b = msh<Bullet>(o, vb, fac, t_life);
				b->owner = this;
				b->dmg = arm->dmg;
				b->t = arm->t_bullet[fac];
				cur.bullets.push_back(b);
			}
		}

		bool move = false;
		if (kbd[L'W']) { move = true; v += cm.vy * acc * rdt; }
		if (kbd[L'S']) { move = true; v -= cm.vy * acc * rdt; }
		if (kbd[L'D']) { move = true; v += cm.vx * acc * rdt; }
		if (kbd[L'A']) { move = true; v -= cm.vx * acc * rdt; }
		if (!move) { slow_down(cur); }

		move = false;
		if (kbd[L'Q']) { move = true; v_ang -= ang_acc * rdt; }
		if (kbd[L'E']) { move = true; v_ang += ang_acc * rdt; }
		if (!move) { turn_to(cur, ang, 0.1); }
		if (msd[2]) {
			double tmp = clmp(v_ang, -0.5, 0.5);
			double dv = clmp(tmp - v_ang, -ang_acc * rdt, ang_acc * rdt);
			v_ang += dv;
		}
		goto end;
	}

	if (tar) {
		// 如果这里用未来的时刻会有什么区别吗。
		vec2 ot = tar->o - o;
		double dist = ot.len();
		if (dist > 1e-3) {
			ot /= dist;
			bool reachble = true;
			bool visible = VisibleTest(cur, o, ot, dist, reachble);
			// 看得见对方和直达对方是两回事。
			// 所以之前的算法过时了，需要更精确的算法处理两种情况。
			// 本来远程单位可以隔着平地射击的。
			if (reachble) {
				if (!arm->ranged) {
					double tar_ang = atan2(ot.y, ot.x);
					double dif = modf(tar_ang - ang + PI, 2 * PI) - PI;
					turn_to(cur, tar_ang, 0.1);
					move_to(cur, tar->o);
					path.clear();

					bool attack = t_attack < 0 &&
						abs(dif) < dif_ang_melee && dist < dist_melee;
					if (attack) {
						bool alive = (tar->health >= 0);
						tar->health -= arm->dmg;
						tar->t_receive = max_t_receive;
						t_send = max_t_send;
						t_attack = arm->max_t_attack;
						if (selected && cur.cast) {
							wstring s =
								loc(L"send") + tw2(arm->dmg) + loc(L"melee_dmg");
							bdc.add_msg(s, { 10, 250, 100 });
						}

						if (tar->selected && cur.cast) {
							wstring s =
								loc(L"receive") + tw2(arm->dmg) + loc(L"melee_dmg");
							bdc.add_msg(s, { 250, 150, 10 });
						}

						if (alive && tar->health < 0) { ++n_kill; }

						if (alive && tar->health < 0 && selected && cur.cast) {
							wstring s = loc(L"kill");
							bdc.add_msg(s, { 255, 255, 10 });
						}
					}
				} else {
					vec2 tar_v = tar->o + dist / v_bullet * tar->v - o;
					double tar_ang = atan2(tar_v.y, tar_v.x);
					double dif = modf(tar_ang - ang + PI, 2 * PI) - PI;
					double lim = clmp(0.05 / dist, 0.01, 0.1);
					turn_to(cur, tar_ang, lim);
					if (dist > dist_ranged * 0.8) { move_to(cur, tar->o); }
					else { slow_down(cur); }
					path.clear();

					bool attack = t_attack < 0 &&
						abs(dif) < lim * mtp_dif_ang_ranged && dist < dist_ranged;
					if (attack) {
						t_send = max_t_send;
						t_attack = arm->max_t_attack;

						// 这里比较乱。以后不要子弹构造函数了。
						// 用一个单独方法创建子弹。
						vec2 front = vec2(cos(ang), sin(ang));
						vec2 vb = front * v_bullet;
						double t_life = min(max_t_bullet, TimeHit(cur, o, vb));
						auto b = msh<Bullet>(o, vb, fac, t_life);
						b->owner = this;
						b->dmg = arm->dmg;
						b->t = arm->t_bullet[fac];
						cur.bullets.push_back(b);
					}
				}
			} else {
				if (path.size() >= 2 && t_path >= 0) {
					auto p0 = vec2(path.back()) + vec2(0.5);
					auto p1 = vec2(*(path.end() - 2)) + vec2(0.5);
					vec2 p01 = p1 - p0;
					double tar_ang = atan2(p01.y, p01.x);
					turn_to(cur, tar_ang, 0.1);
					if ((o - p1).lensqr() < r_path * r_path) {
						path.pop_back();
						t_path = max_t_path;
					} else { move_along(cur, p0, p1); }
				} else {
					path = AStar(cur, pgd, tar->pgd);
					t_path = max_t_path;
				}
			}
		} 
	}

	if (t_target < 0) {
		if (fac->cnt_enemy) {
			//path = Dijkstra(cur, pgd, fac);
			//if (!path.empty()) {
			//	auto p = path.front();
			//	auto& cell = get_cell(cur, p);
			//	double d = DBL_MAX; tar = NULL;

			//	for (auto a : cell.agents) if (a->fac != fac) {
			//		double _d = (o - a->o).len();
			//		if (_d < d) { d = _d; tar = a; }
			//	}
			//}

			// 可以考虑增加一个 t_forget。
			if (tar) {
				tar = NULL; path.clear();
			}
			int r_sight = 12;
			double close = DBL_MAX;
			int x0 = max(0, pgd.x - r_sight);
			int x1 = min(nxgd, pgd.x + r_sight);
			int y0 = max(0, pgd.y - r_sight);
			int y1 = min(nygd, pgd.y + r_sight);
			vec2 front = vec2(cos(ang), sin(ang));

			rep(i, x0, x1) rep(j, y0, y1) {
				auto& cell = gtcl(dvec(i, j));
				for (auto a : cell.agents) {
					if (a->fac == fac) { continue; }
					auto dist = (a->o - o).len();
					if (dist >= close) { continue; }
					if (dist < 1e-3) { close = dist; tar = a; continue; }
					auto u = (a->o - o) / dist;
					if (dot(u, front) < 0.1 && a->t_send < max_t_send - 2) { continue; }
					bool reachable = true;
					if (VisibleTest(cur, o, u, dist, reachable)) { close = dist; tar = a; }
				}
			}
		}
		t_target = tar ? 
			frnd(min_dur_found, max_dur_found) :
			frnd(min_dur_not_found, max_dur_not_found);
	}

	if (!tar) {
		if (t_wonder < 0) {
			if (path.size() >= 2 && t_path >= 0) {
				auto p0 = vec2(path.back()) + vec2(0.5);
				auto p1 = vec2(*(path.end() - 2)) + vec2(0.5);
				vec2 p01 = p1 - p0;
				double tar_ang = atan2(p01.y, p01.x);
				turn_to(cur, tar_ang, 0.1);
				if ((o - p1).lensqr() < r_path * r_path) {
					path.pop_back();
					t_path = max_t_path;
				} else { move_along(cur, p0, p1); }
			} else {
				path.clear(); dvec p;
				p.x = drnd(arm->tl_guard.x, arm->br_guard.x);
				p.y = drnd(arm->tl_guard.y, arm->br_guard.y);
				if (insd(p.x, 0, nxgd) && insd(p.y, 0, nygd)) {
					path = AStar(cur, pgd, p);
					t_path = max_t_path;
				}
			}
		} else {
			slow_down(cur); 
			turn_to(cur, ang, 0.1);
		}
	}

end:
	if (health < 0) { 
		dead = true; 
		if (selected && cur.cast) {
			wstring s = loc(L"killed");
			bdc.add_msg(s, dcol(255, 0, 0));
		}
	}

	t_send -= rdt;
	t_receive -= rdt;
	t_path -= rdt;
	t_wonder -= rdt;
	t_attack -= rdt;
	t_target -= rdt;

	if (v.lensqr() > max_v * max_v) { v = v.unit() * max_v; }
	o += v * rdt;
	v_ang = clmp(v_ang, -max_v_ang, max_v_ang);
	ang += v_ang * rdt;
	collide_wall(cur);
}
void Agent::Discard(Cur& cur) { rmv; }
void Agent::PreUpdate(Cur& cur) {
	if (cur.cast) { return; }
	double r = r_agent * sgd;
	vec2 p = (vec2)bgr.tl + tlgd + sgd * o;
	bool ok = dhv <= dep() &&
		(p - (vec2)msp).lensqr() < r * r && insd(msp, bgr.vp());
	if (ok) { dhv = dep(); hvd = this; }
}

void Agent::cast_health(Cur& cur) {
	double x = dot(o - cm.o, cm.vx);
	double y = cm.h - cur.top_health;
	if (d < d_close) { return; }
	int h = cur.h_health / d * cm.scl;
	int w = cur.w_health / d * cm.scl;
	dvec pt = cm.ct + dvec(vec2(x, y) / d * cm.scl);

	int x0 = pt.x - w / 2;
	int y0 = pt.y;
	int x1 = x0 + w;
	int y1 = y0 + h;
	int da = max(0, x0);
	int db = min(nxcv, x1);
	int dc = max(0, y0);
	int dd = min(nycv, y1);
	if (db <= da || dd <= dc) { return; }

	if (dd - dc < 2) { return; }
	bool tiny = dd - dc < 5;
	int len = w * health;
	rep(i, da, db) {
		auto& rc = cur.recs[i];
		if (rc->hit && rc->d <= d) { continue; }
		int dp = dc * nxcv + i;
		rep(j, dc, dd) {
			bool border = !tiny &&
				(i == x0 || i == x1 - 1 || j == y0 || j == y1 - 1);
			dcol c =
				border ? dcol(255) :
				i - x0 > len ? dcol(0) : fac->col;
			cv[dp] = c; dp += nxcv;
		}
	}
}
void Agent::RenderCast(Cur& cur) {
	if (teleported || selected || d < 0.1) { return; }

	double dif = cm.ang - ang + PI;
	dif = modf(dif * n_dir / (2 * PI) + 0.5, n_dir);
	int idx = clmp<int>(floor(dif), 0, n_dir - 1);
	auto t = t_receive > 0 ?
		arm->imgs_receive[idx] : arm->ts_normal[fac][idx];
	if (!t) { return; }
	
	double x = dot(o - cm.o, cm.vx);
	double y = cm.h;
	if (t_send > 0) { y -= 0.04; }
	int h = cur.h_agent / d * cm.scl;
	int w = h * t->w / t->h;
	dvec pb = cm.ct + dvec(vec2(x, y) / d * cm.scl);

	int x0 = pb.x - w / 2;
	int y0 = pb.y - h;
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
	} cast_health(cur);
}

void collide_agents(Cur& cur, Agent& a0, Agent& a1) {
	vec2 u = a1.o - a0.o;
	auto len = u.len(); u /= len;
	if (len < 1e-3) { return; } 
	
	double d = 2 * r_agent - len;
	if (d > 0) {
		a0.o -= u * d / 2;
		a1.o += u * d / 2;
		auto vn0 = dot(a0.v, u);
		auto vn1 = dot(a1.v, u);
		if (vn0 > 0) { a0.v -= vn0 * u; }
		if (vn1 < 0) { a1.v -= vn1 * u; }
	}
	if (len < 0.6) {
		a0.v -= 2 * u * rdt;
		a1.v += 2 * u * rdt;
	}
}
void CollideAgents(Cur& cur) {
	rep(i, 0, nxgd) rep(j, 0, nygd) {
		auto& cell = gtcl(dvec(i, j));
		auto& as = cell.agents;
		for (auto it = as.begin(); it != as.end(); ++it) {
			auto _it = it; ++_it;
			for (; _it != as.end(); ++_it) {
				collide_agents(cur, **it, **_it);
			}

#define TMP(x, y) for (auto a : gtcl(dvec(x, y)).agents) { \
	collide_agents(cur, **it, *a); \
}
			if (i != nxgd - 1) { TMP(i + 1, j); }
			if (i != 0 && j != nygd - 1) { TMP(i - 1, j + 1); }
			if (j != nygd - 1) { TMP(i, j + 1); }
			if (i != nxgd - 1 && j != nygd - 1) { TMP(i + 1, j + 1); }
#undef TMP
		}
	}
}

void CastSign(Cur& cur, tile const& t, vec2 p, double top, double _h) {
	double d = dot(p - cm.o, cm.vy);
	if (d < 0.1) { return; }

	double x = dot(p - cm.o, cm.vx);
	double y = cm.h - top;
	int h = _h / d * cm.scl;
	int w = h * t.w / t.h;
	dvec pt = cm.ct + dvec(vec2(x, y) / d * cm.scl);
	
	int x0 = pt.x - w / 2;
	int y0 = pt.y;
	int da = max(0, x0);
	int db = min(nxcv, x0 + w);
	int dc = max(0, y0);
	int dd = min(nycv, y0 + h);
	if (db <= da || dd <= dc) { return; }

	int sa = (da - x0) * t.w / w;
	int sb = (db - x0) * t.w / w;
	int dw = db - da;
	int sw = sb - sa;
	rep(j, dc, dd) {
		int y = (j - y0) * t.h / h;
		int dp = j * nxcv + da;
		int sp = y * t.w + sa;
		int e = dw;
		bool ok = t.as[sp];
		dcol col = t.cols[sp];
		rep(i, da, db) {
			while (e <= 0) {
				e += dw; sp++;
				col = t.cols[sp]; ok = t.as[sp];
			}
			if (ok) { cv[dp] = col; }
			e -= sw; dp++;
		}
	}
}
void CastTarSel(Cur& cur) {
	if (!cur.sign_target || !agsel || !agsel->tar) { return; }
	CastSign(cur, *cur.sign_target, agsel->tar->o, cur.top_sign, cur.h_sign);
}
void DrawAgentInfoSel(Cur& cur) {
	if (!agsel) { return; }
	if (agsel->arm->ranged) {
		auto t =
			agsel->t_send > 0 ? cur.t_ranged_send :
			agsel->t_attack > 0 ? cur.t_ranged_bad : cur.t_ranged_ready;
		if (t) {
			dvec tl = bgr.tl + dvec(bgr.w, bgr.h) / 2 - dvec(t->w, t->h) / 2;
			draw_tile(scr, dscr, 20, tl, bgr.vp(), *t, t->rect());
		}

		if (cur.gun) {
			int y = agsel->t_send > 0 ? 320 : 350;
			dvec tl = bgr.tl +
				dvec(bgr.w / 2, bgr.h) - dvec(cur.gun->w / 2, cur.gun->h - y);
			draw_tile(scr, dscr, 20, tl, bgr.vp(), *cur.gun, cur.gun->rect());
		}
	} else {
		auto t =
			agsel->t_send > 0 ? cur.t_melee_send :
			agsel->t_attack > 0 ? cur.t_melee_bad : cur.t_melee_ready;
		if (t) {
			dvec tl = bgr.tl +
				dvec(bgr.w, bgr.h) / 2 - dvec(t->w, t->h) / 2 + dvec(400, 100);
			draw_tile(scr, dscr, 20, tl, bgr.vp(), *t, t->rect());
		}
	}

	int w = 600, h = 20, gap = 10;
	dvec tl = bgr.tl + dvec(bgr.w, bgr.h) - dvec(gap, gap) - dvec(w, h);
	int w_health = agsel->health * w;
	draw_rect_raw(scr, tl, w, h, bgr.vp(), {});
	draw_rect_raw(scr, tl, w_health, h, bgr.vp(), agsel->fac->col);
	draw_px_rect_frame(scr, dscr, 10, tl, w, h, bgr.vp(), dcol(255));

	tl -= dvec(0, gap + ft.h);
	wstring s = loc(L"n_kill") + tw(agsel->n_kill);
	draw_str(scr, dscr, 10, s, dcol(255), ft, tl, 0, bgr.vp());
}
