#include "arm.h"
#include "cur.h"
#include "bgr.h"
#include "faction.h"
#include "draw_geo.h"
#include "draw_str.h"
#include "draw_comp.h"
#include "draw_px_seg.h"

#include "my_def.h"

int n_dir = 8;
int constexpr w = 100;
int constexpr h = 60;
int constexpr gap = 10;
int constexpr dep = 110;

#define found(s) (dic.find(s) != dic.end())
#define getv(nm) if (found(L#nm)) { nm = dic.at(L#nm)->num; }

Arm::Arm(Cur& cur, Var const& v) {
	// 有个问题，如果网格大小中途改变了，这里就会有问题。
	// 可以把全图游荡作为一个单独的模式设置。
	// 现在暂时不去管这些。
	br_guard = dvec(nxgd - 1, nygd - 1);

	auto const& dic = v.dic;
	if (found(L"name")) { name = dic.at(L"name")->str; }
	getv(ranged); 
	
	dmg = 0.07;
	max_t_attack = ranged ? 1.4 : 0.7;
	getv(dmg); getv(max_t_attack);

	if (found(L"tl_guard")) { tl_guard = (dvec)tv2(*dic.at(L"tl_guard")); }
	if (found(L"br_guard")) { br_guard = (dvec)tv2(*dic.at(L"br_guard")); }
	br_guard.x = max(br_guard.x, tl_guard.x);
	br_guard.y = max(br_guard.y, tl_guard.y);


	if (found(L"img_bullet")) {
		auto nm = dic.at(L"img_bullet")->str;
		auto path = L"images/" + nm + L".tile";
		mkp(img_bullet)(path);
		if (!img_bullet->n()) { img_bullet = NULL; }
	}

	if (found(L"imgs_normal")) {
		auto nm = dic.at(L"imgs_normal")->str;
		rep(i, 0, n_dir) {
			auto path = L"images/" + nm + tw(i) + L".tile";
			auto t = msh<tile>(path);
			if (!t->n()) { t = NULL; }
			imgs_normal.push_back(t);
		}
	} else { imgs_normal.resize(n_dir); }

	for (auto _t : imgs_normal) {
		if (!_t) { imgs_receive.push_back(NULL); continue; }
		auto t = msh<tile>(*_t);
		for (auto& c : t->cols) { c = {}; }
		imgs_receive.push_back(t);
	}

	refresh_ts(cur);
}

dvec Arm::tl(Cur& cur) const {
	dvec tl;
	tl.x = bgr.tl.x + (w + gap) * id + gap; 
	tl.y = bgr.tl.y + bgr.h - h - gap; return tl;
}
drect Arm::rect(Cur& cur) const {
	return { tl(cur), w, h };
}
void Arm::refresh_ts(Cur& cur) {
	ts_normal.clear();
	for (auto f : cur.factions) {
		vector<ptr<tile>> ts;
		for (auto _t : imgs_normal) {
			if (!_t) { ts.push_back(NULL); }
			auto t = msh<tile>(*_t);
			for (auto& c : t->cols)
			if (c == dcol(0, 255, 0)) { c = f->col; }
			ts.push_back(t);
		} ts_normal[&*f] = ts;
	}

	t_bullet.clear();
	for (auto f : cur.factions) {
		if (!img_bullet) { t_bullet[&*f] = NULL; continue; }
		auto t = msh<tile>(*img_bullet);
		for (auto& c : t->cols)
		if (c == dcol(0, 255, 0)) { c = f->col; }
		t_bullet[&*f] = t;
	}
}

void Arm::render(Cur& cur) {
	dvec tl_rect = tl(cur);
	dvec tl_str = tl_rect;
	auto w_str = str_wh(name, ft, 0).x;
	tl_str.x += (w - w_str) / 2;
	tl_str.y += (h - ft.h) / 2;

	if (hovered || selected) {
		dcol c = selected ? dcol(150, 60, 60) : dcol(60, 60, 150);
		draw_rect_raw(scr, tl_rect, w, h, bgr.vp(), c);
	}

	draw_px_rect_frame(scr, dscr, dep, tl_rect, w, h, bgr.vp(), dcol(255));
	draw_str(scr, dscr, dep, name, dcol(255), ft, tl_str, 0, bgr.vp());
}
void Arm::Update(Cur& cur) {
	if (cur.cast || !cur.edit) { return; }
	hovered = (hvd == this);
	selected = (this == &*cur.arm_sel);

	if (hovered && msc(0)) {
		if (cur.arm_sel != this) { cur.arm_sel = this; cur.terrain_sel = NULL; }
		else { cur.arm_sel = NULL; }
	}
	render(cur);
}
void Arm::Discard(Cur& cur) { rmv; }
void Arm::PreUpdate(Cur& cur) {
	if (cur.cast || !cur.edit) { return; }
	bool ok = dhv < dep&&
		insd(msp, bgr.vp()) && insd(msp, rect(cur));
	if (ok) { dhv = dep; hvd = this; }
}
