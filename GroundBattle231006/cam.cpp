#include "cam.h"
#include "cur.h"
#include "bgr.h"
#include "var.h"
#include "cell.h"
#include "agent.h"
#include "draw_geo.h"
#include "draw_px_seg.h"

#include "my_def.h"

#define found(s) (dic.find(s) != dic.end())
#define getv(nm) if (found(L#nm)) { nm = dic.at(L#nm)->num; }

double dep = 25;
double r_cam = 0.3;

CastCam::CastCam(drect vp) : vp(vp) {
	h = 0.55;
	scl = vp.h / 2; calc();
}
void CastCam::calc() {
	ct = vp.tl + dvec(vp.w, vp.h) / 2;
	vy = vec2(cos(ang), sin(ang));
	vx = vec2(cos(ang + PI / 2), sin(ang + PI / 2));
}
void CastCam::set_cfg(Var const& v) {
	auto& dic = v.dic;
	// 数据可能会超过范围。
	getv(scl); getv(h); getv(ang);
	if (found(L"o")) { o = tv2(*dic.at(L"o")); }
}


void CastCam::Update(Cur& cur) {
	hovered = (hvd == this);
	if (dragged) {
		// 不管跟随那些了。
		dragged = msd[0] && !cur.cast;
		o = ((vec2)msp - tlgd - (vec2)bgr.tl) / sgd;
	} else {
		// 不管那些刁钻的情形了，没啥影响。
		dragged = hovered && msc(0);
	}

	if (!kb) {
		if (kbd[L'W']) { o += vy * 4 * rdt; }
		if (kbd[L'S']) { o -= vy * 4 * rdt; }
		if (kbd[L'D']) { o += vx * 4 * rdt; }
		if (kbd[L'A']) { o -= vx * 4 * rdt; }
		if (kbd[L'Q']) { ang -= 1.5 * rdt; }
		if (kbd[L'E']) { ang += 1.5 * rdt; }
	}
	if (cur.cast) {
		if (kbd[L'R']) { h += 0.35 * rdt; }
		if (kbd[L'F']) { h -= 0.35 * rdt; }
		if (bgr.wheeled && msw) { scl = exp(log(scl) + 0.05 * msw); }
	}

	if (agsel) {
		// 不知道这个后退有多大作用。
		// 这样做之后视角平滑了很多，不知道是不是错觉。
		// 我怀疑这是因为这里 vy 是上一步计算的。
		o = agsel->o - 0.2 * vy;
		ang = agsel->ang;
	}

	double eps = 1e-2;
	h = clmp<double>(h, eps, cur.h_wall - eps);
	o.x = clmp<double>(o.x, eps, nxgd - eps);
	o.y = clmp<double>(o.y, eps, nygd - eps);
	ang = modf(ang, 2 * PI); calc();
}
void CastCam::PreUpdate(Cur& cur) {
	if (agsel) { return; }
	double r = r_cam * sgd;
	vec2 p = (vec2)bgr.tl + tlgd + sgd * o;
	bool ok = dhv <= dep &&
		(p - (vec2)msp).lensqr() < r * r && insd(msp, bgr.vp());
	if (ok) { dhv = dep; hvd = this; }
}
void CastCam::RenderFlat(Cur& cur) {
	if (agsel) { return; }

	dcol c =
		dragged ? dcol(180, 0, 0) :
		hovered ? dcol(0, 0, 180) : dcol(0, 100, 0);
	vec2 p = (vec2)bgr.tl + tlgd + sgd * o;
	double r = r_cam * sgd;
	draw_eclipse(scr, dscr, dep, p, r, r, bgr.vp(), c);
	draw_eclipse_frame(scr, dscr, dep, p, r, r, bgr.vp(), dcol(255), 20);

	vec2 p_front = p + r * vec2(cos(ang), sin(ang));
	draw_px_seg(scr, dscr, p, p_front, dep, bgr.vp(), dcol(255));
}
