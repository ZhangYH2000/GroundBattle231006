#include "terrain.h"
#include "var.h"
#include "cur.h"
#include "bgr.h"
#include "draw_geo.h"
#include "draw_str.h"
#include "draw_comp.h"

#include "my_def.h"

// 这些东西还是用下拉选单的方式保存比较好。

int constexpr w = 160;
int constexpr h = 40;
int constexpr wh_sign = 20;
int constexpr gap = 10;
int constexpr dep = 110;

Terrain::Terrain() {
	c_flat = dcol(120); 
}

#define found(s) (dic.find(s) != dic.end())
#define getv(nm) if (found(L#nm)) { nm = dic.at(L#nm)->num; }

void init_imgs(wstring const& nm, Var const& v, vector<cast_tile>& imgs) {
	auto const& dic = v.dic;
	if (found(nm)) {
		auto& vec = dic.at(nm)->vec;
		for (auto t : vec) if (t->vec.size() >= 2) {
			imgs.push_back({ t->vec[0]->num, t->vec[1]->str });
		}
	}
}
Terrain::Terrain(Var const& v) {
	auto const &dic = v.dic;
	if (found(L"name")) { name = dic.at(L"name")->str; }
#define TMP(cx0) if (found(L#cx0)) { cx0 = tv3(*dic.at(L#cx0)); }
	TMP(cx0); TMP(cx1); TMP(cy0); TMP(cy1); TMP(cz0); TMP(cz1);
	if (found(L"c_flat")) { c_flat = (dcol)tv3(*dic.at(L"c_flat")); }
	getv(cx0_rnd); getv(cx1_rnd); 
	getv(cy0_rnd); getv(cy1_rnd); getv(cz0_rnd); getv(cz1_rnd);
	getv(wall); getv(walkable);

#undef TMP
#define TMP(x0) init_imgs(L"imgs_" L#x0, v, imgs_##x0)

	TMP(x0); TMP(x1); TMP(y0); TMP(y1); TMP(z0); TMP(z1);
}
dvec Terrain::tl(Cur& cur) const {
	dvec tl;
	tl.x = bgr.tl.x + gap;
	tl.y = (h + gap) * id + gap; return tl;
}
drect Terrain::rect(Cur& cur) const {
	return { tl(cur), w, h };
}

void Terrain::render(Cur& cur) {
	dvec tl_rect = tl(cur);
	dvec tl_sign = tl_rect;
	tl_sign.x += gap;
	tl_sign.y += (h - wh_sign) / 2;
	dvec tl_str = tl_rect;
	tl_str.x += wh_sign + 2 * gap;
	tl_str.y += (h - ft.h) / 2;

	if (hovered || selected) {
		dcol c = selected ? dcol(150, 60, 60) : dcol(60, 60, 150);
		draw_rect_raw(scr, tl_rect, w, h, bgr.vp(), c);
	}

	draw_px_rect_frame(scr, dscr, dep, tl_rect, w, h, bgr.vp(), dcol(255));
	draw_px_rect_framed(scr, dscr, dep, tl_sign, wh_sign, wh_sign, bgr.vp(), c_flat, dcol(255));
	draw_str(scr, dscr, dep, name, dcol(255), ft, tl_str, 0, bgr.vp());
}
void Terrain::Update(Cur& cur) {
	if (cur.cast || !cur.edit) { return; }
	hovered = (hvd == this);
	selected = (this == &*cur.terrain_sel);

	if (hovered && msc(0)) {
		if (cur.terrain_sel != this) { cur.terrain_sel = this; cur.arm_sel = NULL; }
		else { cur.terrain_sel = NULL; }
	}
	render(cur);
}
void Terrain::Discard(Cur& cur) { rmv; }
void Terrain::PreUpdate(Cur& cur) {
	if (cur.cast || !cur.edit) { return; }
	bool ok = dhv < dep && 
		insd(msp, bgr.vp()) && insd(msp, rect(cur));
	if (ok) { dhv = dep; hvd = this; }
}
