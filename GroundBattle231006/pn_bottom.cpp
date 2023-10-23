#include "pn_bottom.h"
#include "cur.h"
#include "bgr.h"
#include "cam.h"
#include "cell.h"

#define cur (*(Cur*)&app)
#include "my_def.h"

LbFPS::LbFPS() { w = 450; fixed_w = true; }
void LbFPS::Sync(App& app) {
	txt = loc(L"frm_tm") + tw2(cur.fps.frm_time * 1000) +
		L", FPS: " + tw2(cur.fps.fps);
}

LbCam::LbCam() { w = 600; fixed_w = true; }
void LbCam::Sync(App& app) {
	txt = loc(L"lb_cam") +
		L"o=" + tw2(cm.o) +
		L",ang=" + tw2(cm.ang) + 
		L",h=" + tw2(cm.h) + L",scl=" + tw2(cm.scl);
}

LbGrid::LbGrid() { w = 450; fixed_w = true; }
void LbGrid::Sync(App& app) {
	vec2 p = ((vec2)msp - tlgd - (vec2)bgr.tl) / sgd;
	dvec pgd(floor(p.x), floor(p.y));
	txt = loc(L"grid_pos") + tw(pgd);
}


PnBottom::PnBottom(App& app) : Panel(Panel::bottom(app)) {
	vector<Control*> tmp;
	mkp(lb_fps)(); mkp(lb_cam)(); mkp(lb_grid)();
	tmp = { &*lb_fps, &*lb_cam, &*lb_grid };
	mkcl(clx); c = &*clx; Init(app);
}
