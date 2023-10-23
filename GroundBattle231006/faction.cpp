#include "faction.h"
#include "cur.h"
#include "bgr.h"
#include "draw_geo.h"
#include "draw_str.h"
#include "draw_comp.h"
#include "draw_px_seg.h"

#include "my_def.h"

int constexpr w = 160;
int constexpr h = 40;
int constexpr r_sign = 10;
int constexpr gap = 10;
int constexpr dep = 110;

#define found(s) (dic.find(s) != dic.end())
#define getv(nm) if (found(L#nm)) { nm = dic.at(L#nm)->num; }

Faction::Faction(Var const& v) {
	auto const& dic = v.dic;
	if (found(L"name")) { name = dic.at(L"name")->str; }
	if (found(L"col")) { col = (dcol)tv3(*dic.at(L"col")); }
}
dvec Faction::tl(Cur& cur) const {
	dvec tl;
	tl.x = bgr.tl.x + bgr.w - w - gap;
	tl.y = (h + gap) * id + gap; return tl;
}
drect Faction::rect(Cur& cur) const {
	return { tl(cur), w, h };
}

void Faction::Reset(Cur& cur) {
	cnt_enemy = 0; cnt = 0;
}
void Faction::render(Cur& cur) {
	dvec tl_rect = tl(cur);
	vec2 o_sign = (vec2)tl_rect;
	o_sign.x += gap + r_sign;
	o_sign.y += h / 2;
	dvec tl_str = tl_rect;
	tl_str.x += 2 * r_sign + 2 * gap;
	tl_str.y += (h - ft.h) / 2;

	if (hovered || selected) {
		dcol c = selected ? dcol(150, 60, 60) : dcol(60, 60, 150);
		draw_rect_raw(scr, tl_rect, w, h, bgr.vp(), c);
	}

	draw_px_rect_frame(scr, dscr, dep, tl_rect, w, h, bgr.vp(), dcol(255));
	draw_eclipse(scr, dscr, dep, o_sign, r_sign, r_sign, bgr.vp(), col);
	draw_eclipse_frame(scr, dscr, dep, o_sign, r_sign, r_sign, bgr.vp(), dcol(255), 20);
	draw_str(scr, dscr, dep, name, dcol(255), ft, tl_str, 0, bgr.vp());
}
void Faction::Update(Cur& cur) {
	if (cur.cast || !cur.edit) { return; }
	hovered = (hvd == this);
	selected = (this == &*cur.faction_sel);

	if (hovered && msc(0)) {
		cur.faction_sel = selected ? NULL : this;
	}
	render(cur);
}
void Faction::Discard(Cur& cur) { rmv; }
void Faction::PreUpdate(Cur& cur) {
	if (cur.cast || !cur.edit) { return; }
	bool ok = dhv < dep&&
		insd(msp, bgr.vp()) && insd(msp, rect(cur));
	if (ok) { dhv = dep; hvd = this; }
}
