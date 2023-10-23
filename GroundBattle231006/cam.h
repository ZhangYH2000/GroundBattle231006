#pragma once
#include "vec2.h"

struct Cur;
struct Var;
struct CastCam {
	dvec ct;
	drect vp;
	vec2 o, vx, vy;
	double ang = 0, h = 0, scl = 0;

	bool hovered = false;
	bool dragged = false;

	CastCam(drect vp);
	void calc();
	void set_cfg(Var const& v);

	void Update(Cur& cur);
	void PreUpdate(Cur& cur);
	void RenderFlat(Cur& cur);
};
