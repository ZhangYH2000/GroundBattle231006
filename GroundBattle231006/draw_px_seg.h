#pragma once
#include "vec3.h"
#include "tile.h"

bool pre_draw_px_seg(dvec& pa, dvec& pb, drect vp);
void draw_px_seg(tile& dest, dbuf& ds, 
	dvec pa, dvec pb, double d, drect vp, dcol c);
void draw_px_seg(tile& dest, dbuf& ds, 
	vec2 pa, vec2 pb, double d, drect vp, dcol c);

// 这个方法是临时加的。
void draw_eclipse_frame(tile& dest, dbuf& ds, double dep,
	vec2 ct, double ax, double ay, drect vp, dcol col, int n);
