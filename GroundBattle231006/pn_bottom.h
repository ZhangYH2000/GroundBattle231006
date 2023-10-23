#pragma once
#include "panel.h"
#include "label.h"
#include "ctrl_list.h"

struct LbFPS : Label {
	LbFPS();
	void Sync(App& app) override;
};
struct LbCam : Label {
	LbCam();
	void Sync(App& app) override;
};
struct LbGrid : Label {
	LbGrid();
	void Sync(App& app) override;
};

struct PnBottom : Panel {
	ptr<LbFPS> lb_fps;
	ptr<LbCam> lb_cam;
	ptr<LbGrid> lb_grid;
	ptr<CtrlListX> clx;
	PnBottom(App& app);
};
