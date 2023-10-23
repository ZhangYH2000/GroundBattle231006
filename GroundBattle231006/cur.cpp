#include "cur.h"
#include "ui.h"
#include "arm.h"
#include "bgr.h"
#include "cam.h"
#include "par.h"
#include "clip.h"
#include "cell.h"
#include "cast.h"
#include "agent.h"
#include "bullet.h"
#include "faction.h"
#include "builder.h"
#include "terrain.h"
#include "draw_str.h"
#include "broadcast.h"

#define found(s) (dic.find(s) != dic.end())
#define getv(nm) if (found(L#nm)) { nm = dic.at(L#nm)->num; }

void prepare_tile(ptr<tile>& t, wstring nm, int w = 0) {
	wstring path = L"images/" + nm + L".tile";
	mkp(t)(path);
	if (!t->n()) { t = NULL; return; }
	if (w) {
		int h = w * t->h / t->w;
		mkp(t)(w, h, *t, t->rect());
	}
}
void prepare_tile(ptr<tile>& t, dcol c, wstring nm, int w = 0) {
	prepare_tile(t, nm, w);
	if (!t) { return; }

	for (auto& _c : t->cols) 
	if (_c == dcol(0, 255, 0)) { _c = c; }
}
Cur::Cur() {
	w = 1800; h = 900; set_locale();
	wv.len_block *= 3;
	print_console(L"正在加载字体...");
	App::Init();
	print_console(L"字体加载完成.", true);
	print_console(L"正在加载控件...");
	mkp(ui)(*this); ui->Init(*this);
	mkp(bgr)(*this); Reset();
	print_console(L"控件加载完成.", true);

	mkp(sign_target)(L"images/sign.tile");
	for (auto& c : sign_target->cols) 
	if (c == dcol(0, 255, 0)) { c = dcol(255, 255, 0); }
	if (!sign_target->n()) { sign_target = NULL; }

	prepare_tile(gun, L"gun", 320);
	prepare_tile(sign_target, dcol(255, 255, 0), L"sign");
	prepare_tile(t_melee_ready, dcol(255, 185, 100), L"sign_melee", 400);
	prepare_tile(t_melee_send, dcol(255), L"sign_melee", 400);
	prepare_tile(t_melee_bad, dcol(65), L"sign_melee", 400);
	prepare_tile(t_ranged_ready, dcol(255, 0, 0), L"sign_ranged", 160);
	prepare_tile(t_ranged_send, dcol(255), L"sign_ranged", 160);
	prepare_tile(t_ranged_bad, dcol(65), L"sign_ranged", 160);

	print_console(L"正在加载音乐...");
	bool ok = false;
	mkp(cl0)(L"music/bgm0.clip", &ok);
	if (!ok) { cl0 = NULL; }
	mkp(cl1)(L"music/bgm1.clip", &ok);
	if (!ok) { cl1 = NULL; }
	print_console(L"音乐加载完成.", true);
	vol = 1;
	cl = frnd(1) < 0.5 ? &*cl0 : &*cl1;
	edit = true;
}

#define cur (*this)
#include "my_def.h"

void Cur::Save(wstring const& nm) const {
	FILE* f = wfopen(nm, L"wb");
	if (!f) { return; }
	int sz = 0; fwtv(cmd); save_par(f);

	fwt(nxgd); fwt(nygd);
	for (auto c : grid) { c->Save(f); }
	sz = agents.size(); fwt(sz);
	for (auto a : agents) { a->Save(f); }
	fclose(f);
}
void Cur::Load(wstring const& nm) {
	FILE* f = wfopen(nm, L"rb");
	if (!f) { return; } Reset();
	int sz = 0; frdv(cmd); 
	tmp_cmd = cmd; load_par(f);
	Execute(gl, Compile(cmd)); 
	
	// 有点奇怪，这里全局命令是预先执行的。
	// 可能预先执行才是符合实际的安排吧。
	// 这一套保存读取系统肯定要改进的。

	// 背景数据应该是在 UI 中填写的，不能搞成全局命令的形式。
	// 以后要改的。关键点是涉及到增删了。以及依赖性？

	// 可以做的一件事是 cmd 分节，或者增加一个只保存的功能。

	frd(nxgd); frd(nygd);
	grid.clear();
	rep(j, 0, nygd) rep(i, 0, nxgd) {
		grid.push_back(msh<Cell>(*this, dvec(i, j), f));
	}
	frd(sz);
	rep(i, 0, sz) {
		agents.push_back(msh<Agent>(*this, f));
	}

	fclose(f);
}
void Cur::Reset() {
	gl.clear(); init_def_fun(); pars.clear();

	max_real_dt = 0.02;
	cast = false;
	gap_ceil = 10;
	gap_floor = 10;
	simple_ceil = true;
	simple_floor = false;
	c_ceil = dcol(0, 100, 255);
	c_floor = dcol(0, 120, 0);
	
	terrain_sel = NULL;
	terrains.clear();
	terrains.push_back(msh<Terrain>());
	faction_sel = NULL;
	factions.clear();
	arm_sel = NULL;
	arms.clear();

	r_agent = 0.14;
	acc = 4;
	max_v = 1;
	h_agent = 0.75;
	top_sign = 0.9;
	h_sign = 0.15;
	top_health = 0.72;
	w_health = 0.3;
	h_health = 0.02;
	top_bullet = 0.4;
	h_bullet = 0.05;

	//r_agent = 0.22;
	//acc = 7;
	//max_v = 1.8;
	//h_agent = 1.5;
	//top_sign = 1.7;
	//h_sign = 0.3;
	//top_health = 1.4;
	//w_health = 0.6;
	//h_health = 0.04;
	//top_bullet = 1;
	//h_bullet = 0.1;

	set_grid_size(*this, 40, 20);
	tl_grid = {};
	mkp(broad)();

	agents.clear();
	bullets.clear();


	h_wall = 1;
	nx_canvas = bgr.w * 0.33;
	ny_canvas = bgr.h * 0.33;
	recs.clear(); canvas.clear();
	canvas.resize(nx_canvas * ny_canvas);

	mkp(cam)(drect(nx_canvas, ny_canvas));
	cam->o = vec2(nx_grid, ny_grid) / 2;
	cam->calc();
}
void Cur::Update() {
	bgr.PreUpdate(*this);
	if (!cast) {
		cam->PreUpdate(*this);
		builder->PreUpdate(*this);
		for (auto a : agents) { a->PreUpdate(*this); }
		for (auto t : terrains) { t->PreUpdate(*this); }
		for (auto f : factions) { f->PreUpdate(*this); }
		for (auto a : arms) { a->PreUpdate(*this); }
	}
	ui.PreUpdate(*this);
	basic_update();

	gl[L"dt"] = msh<Var>(rdt);
	gl[L"paused"] = msh<Var>(paused);
	gl[L"n_agents"] = msh<Var>(agents.size());
	gl[L"n_arms"] = msh<Var>(arms.size());
	gl[L"n_factions"] = msh<Var>(factions.size());
	gl[L"w_bgr"] = msh<Var>(bgr.w);
	gl[L"h_bgr"] = msh<Var>(bgr.h);
	gl[L"nx_grid"] = msh<Var>(nxgd);
	gl[L"ny_grid"] = msh<Var>(nygd);
	vector<ptr<Var>> ns_enemy;
	for (auto f : factions) {
		ns_enemy.push_back(msh<Var>(f->cnt_enemy));
	}
	gl[L"ns_enemy"] = msh<Var>(ns_enemy);


	if (gl.find(L"update") != gl.end()) {
		auto tmp = *gl[L"update"];
		Execute(gl, tmp.procs);
	}

	if (cl0 && cl1) {
		if (cl->csp >= cl->n()) {
			cl->csp = 0;
			cl = (cl == &*cl0) ? &*cl1 : &*cl0;
		}
		if (!mute) { cl->play(wv.wvin); }
	}

	real_dt = min(dt, max_real_dt);
	terrains[0]->name = loc(L"default_terrain");
	if (!kb) {
		if (kbc(L'I')) { agsel = NULL; }
		if (kbc(L'T')) { edit = !edit; builder->Discard(*this); }
		if (kbc(L'U')) { player = !player; }
		if (kbc(L'C')) { cast = !cast; builder->Discard(*this); }
		if (kbc(L'P')) { auto_follow = !auto_follow; }
		if (kbc(L' ')) { paused = !paused; }
		if (kbc(L'O')) { 
			if (!agents.empty()) {
				agent_sel = &*agents[drnd(agents.size())];
			}
		}
	}

	if (auto_follow && !agsel && !agents.empty()) {
		agent_sel = &*agents[drnd(agents.size())];
	}

	
	if (agsel && agsel->dead) { agsel = NULL; }
	for (auto a : agents) { a->CheckDead(*this); }
	for (auto b : bullets) { b->CheckDead(*this); }
	for (auto c : grid) { c->CheckDead(*this); }
	for (auto a : agents) { a->RemoveDead(*this); }
	for (auto b : bullets) { b->RemoveDead(*this); }
	agents.erase(remove_if(agents.begin(), agents.end(),
		[](ptr<Agent> a) { return a->dead; }), agents.end());
	bullets.erase(remove_if(bullets.begin(), bullets.end(),
		[](ptr<Bullet> b) { return b->dead; }), bullets.end());

	for (auto a : agents) if (a->dead) { a->Discard(*this); }
	for (auto a : arms) if (a->dead) { a->Discard(*this); }
	for (auto f : factions) if (f->dead) { f->Discard(*this); }
	for (auto t : terrains) if (t->dead) { t->Discard(*this); }
	arms.erase(remove_if(arms.begin(), arms.end(),
		[](ptr<Arm> a) { return a->dead; }), arms.end());
	factions.erase(remove_if(factions.begin(), factions.end(),
		[](ptr<Faction> f) { return f->dead; }), factions.end());
	terrains.erase(remove_if(terrains.begin(), terrains.end(),
		[](ptr<Terrain> t) { return t->dead; }), terrains.end());

	//static double t_remain = 0;
	//if (t_remain < 0 && arms.size() >= 2) {
	//	if (cast) {
	//		wstring s = L"新的战士到来了.";
	//		bdc.add_msg(s, dcol(255));
	//	}

	//	t_remain = 25;
	//	int n = 50;
	//	int total = (factions.size() - 1) * agents.size();
	//	for (auto f : factions) {
	//		int m = total > 0 ? 
	//			f->cnt_enemy * n / total : n / factions.size();
	//		rep(i, 0, m) {
	//			auto arm = frnd(1) < 0.5 ? &*arms[1] : &*arms[0];
	//			vec2 o = vec2(frnd(nxgd), frnd(nygd));
	//			auto a = msh<Agent>(arm, &*f, o);
	//			agents.push_back(a);
	//		}
	//	}
	//}
	//dbstr = L"Reinforcement: " + tw2(t_remain);
	//if (!paused) { t_remain -= rdt; }

	for (auto c : grid) { c->Reset(*this); }
	for (auto f : factions) { f->Reset(*this); }
	for (auto a : agents) { a->RefreshGridFac(*this); }
	if (!paused) { CollideAgents(*this); }

	bgr.Update(*this);
	for (auto a : agents) { a->Update(*this); }
	for (auto b : bullets) { b->Update(*this); }
	cm.Update(*this);
	if (!cast) {
		for (auto c : grid) { c->RenderFlat(*this); }
		for (auto a : agents) { a->RenderFlat(*this); }
		cam->RenderFlat(*this);
		// 现在看是否分离 Render 跟 Update 是比较好的
		rep(i, 0, terrains.size()) {
			terrains[i]->id = i;
			terrains[i]->Update(*this);
		}
		rep(i, 0, factions.size()) {
			factions[i]->id = i;
			factions[i]->Update(*this);
		}
		rep(i, 0, arms.size()) {
			arms[i]->id = i;
			arms[i]->Update(*this);
		} 
		builder->Update(*this);
	} else {
		CastCeils(*this); CastFloors(*this); CastWalls(*this);
		if (!simple_ceil) { CastCeilsClose(*this); }
		if (!simple_floor) { CastFloorsClose(*this); }
		multiset<Castable*, comp_cast> cs;
		for (auto a : agents) { cs.insert(&*a); }
		for (auto b : bullets) { cs.insert(&*b); }
		for (auto c : cs) { c->Cast(*this); }
		CastTarSel(*this); 
		DrawCanvas(*this);
		DrawAgentInfoSel(*this);
	}
	broad->Update(*this);
	draw_str(scr, dscr, 999, dbstr,
		dcol(255), ft, bgr.tl + dvec(10, 10), bgr.w - 20, bgr.vp());
	ui.Update(*this);
}

ptr<tile> Cur::get_tile(wstring s) {
	auto it = tiles.find(s);
	if (it == tiles.end()) { return NULL; }
	return it->second;
}
void Cur::load_tile(wstring const& s, int w, int h) {
	auto nm = L"images/" + s + L".tile";
	auto t = tile(nm);
	if (!t.n()) { return; }

	if (w == 0) { w = t.w; h = t.h; }
	w = t.w;
	if (h == 0) { h = w * t.h / t.w; }
	tiles[s] = msh<tile>(w, h, t, t.rect());
}
void Cur::refill_tile(wstring const& out, wstring const& in, dcol c) {
	auto t = get_tile(in);
	if (!t) { return; }

	auto _t = msh<tile>(*t);
	for (auto& _c : _t->cols) 
	if (_c == dcol(0, 255, 0)) { _c = c; }
	tiles[out] = _t;
}

void Cur::save_par(FILE* f) const {
	int sz = pars.size(); fwt(sz);
	for (auto p : pars) { p->save(f); }
}
void Cur::load_par(FILE* f) {
	int sz = 0; frd(sz);
	rep(i, 0, sz) { pars.push_back(msh<param>(f)); }
}
void Cur::set_cfg(Var const& v) {
	// 这是早期版本，缺少异常检查，数据越界可能会有问题。
	auto& dic = v.dic;
	getv(max_real_dt); getv(r_agent);
	getv(acc); getv(max_v); getv(h_agent);
	getv(top_sign); getv(top_health);
	getv(h_sign); getv(w_health); getv(h_health);
	getv(top_bullet); getv(h_bullet);
	getv(h_wall); getv(gap_ceil); getv(gap_floor);
	getv(simple_ceil); getv(simple_floor); getv(vol);

	if (found(L"c_ceil")) { c_ceil = (dcol)tv3(*dic.at(L"c_ceil")); }
	if (found(L"c_floor")) { c_floor = (dcol)tv3(*dic.at(L"c_floor")); }
}
void Cur::init_def_fun() {
	auto f0 = [this](vector<ptr<Var>> const& in) {
		if (in.size() >= 1) { terrains.push_back(msh<Terrain>(*in[0])); }
		return msh<Var>();
	};
	gl[L"add_terrain"] = msh<Var>(f0);

	// f1 被删了。

	auto f2 = [this](vector<ptr<Var>> const& in) {
		if (in.size() >= 1) {
			auto id = in[0]->num;
			if (insd<int>(id, 1, terrains.size())) {
				terrains[id]->dead = true;
			}
		} return msh<Var>();
	};
	gl[L"del_terrain"] = msh<Var>(f2);

	auto f3 = [this](vector<ptr<Var>> const& in) {
		if (in.size() >= 2) { set_grid_size(*this, in[0]->num, in[1]->num); }
		return msh<Var>();
	};
	gl[L"set_grid_size"] = msh<Var>(f3);

	auto f4 = [this](vector<ptr<Var>> const& in) {
		if (in.size() >= 3) { 
			load_tile(in[0]->str, in[1]->num, in[2]->num); 
		} return msh<Var>();
	};
	gl[L"load_tile"] = msh<Var>(f4);

	auto f5 = [this](vector<ptr<Var>> const& in) {
		if (in.size() >= 3) {
			refill_tile(in[0]->str, in[1]->str, (dcol)tv3(*in[2]));
		} return msh<Var>();
	}; 
	gl[L"refill_tile"] = msh<Var>(f5);

	auto f6 = [this](vector<ptr<Var>> const& in) {
		if (in.size() >= 1) { factions.push_back(msh<Faction>(*in[0])); }
		return msh<Var>();
	};
	gl[L"add_faction"] = msh<Var>(f6);

	// f7 被删了。

	auto f8 = [this](vector<ptr<Var>> const& in) {
		if (in.size() >= 1) {
			auto id = in[0]->num;
			if (insd<int>(id, 0, factions.size())) {
				factions[id]->dead = true;
			}
		} return msh<Var>();
	};
	gl[L"del_terrain"] = msh<Var>(f8);

	auto f9 = [this](vector<ptr<Var>> const& in) {
		if (in.size() >= 1) { arms.push_back(msh<Arm>(*this, *in[0])); }
		return msh<Var>();
	};
	gl[L"add_arm"] = msh<Var>(f9);

	// f10 被删了。

	// 这些删除的方法都要重写。
	auto f11 = [this](vector<ptr<Var>> const& in) {
		if (in.size() >= 1) {
			auto id = in[0]->num;
			if (insd<int>(id, 0, arms.size())) {
				arms[id]->dead = true;
			}
		} return msh<Var>();
	};
	gl[L"del_arm"] = msh<Var>(f11);

	auto f12 = [this](vector<ptr<Var>> const& in) {
		if (in.size() >= 2) { set_resolution(*this, in[0]->num, in[1]->num); }
		return msh<Var>();
	};
	gl[L"set_resolution"] = msh<Var>(f12);

	auto f13 = [this](vector<ptr<Var>> const& in) {
		if (in.size() >= 1) { set_cfg(*in[0]); }
		return msh<Var>();
	};
	gl[L"set_cfg"] = msh<Var>(f13);

	auto f14 = [this](vector<ptr<Var>> const& in) {
		if (in.size() >= 2) { bdc.add_msg(in[0]->str, (dcol)tv3(*in[1])); }
		return msh<Var>();
	};

	auto f15 = [this](vector<ptr<Var>> const& in) {
		if (in.size() >= 3) { 
			if (factions.empty() || arms.empty()) { return msh<Var>(); }
			int id = 0;
			id = clmp<int>(in[1]->num, 0, factions.size() - 1);
			auto fac = &*factions[id];
			id = clmp<int>(in[2]->num, 0, arms.size() - 1);
			auto arm = &*arms[id];
			vec2 o = tv2(*in[0]);
			auto a = msh<Agent>(arm, fac, o);
			agents.push_back(a);
		} return msh<Var>();
	};
	gl[L"create_agent"] = msh<Var>(f15);
}
void Cur::basic_update() {
	title = loc(L"title");
	if (gl.find(L"dbstr") != gl.end()) { dbstr = gl[L"dbstr"]->str; }
	if (gl.find(L"update") != gl.end()) {
		auto f = gl[L"update"];  Execute(gl, f->procs);
	}

	pars.erase(remove_if(pars.begin(), pars.end(),
		[](ptr<param> p) { return p->del; }), pars.end());
	for (auto& p : pars) { gl[p->nm] = msh<Var>(p->val); }
}
