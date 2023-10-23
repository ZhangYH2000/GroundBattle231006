#pragma once
#include "panel.h"
#include "button.h"
#include "ctrl_list.h"

struct BtPause : Button {
	BtPause();
	void Update(App& app) override;
	void OnClick(App& app) override;
};
struct BtMute : Button {
	BtMute();
	void Update(App& app) override;
	void OnClick(App& app) override;
};
struct BtRender : Button {
	BtRender();
	void Update(App& app) override;
	void OnClick(App& app) override;
};
struct BtControl : Button {
	BtControl();
	void Update(App& app) override;
	void OnClick(App& app) override;
};
struct BtMode : Button {
	BtMode();
	void Update(App& app) override;
	void OnClick(App& app) override;
};

struct PnMenu : Panel {
	ptr<BtPause> bt_pause;
	ptr<BtMute> bt_mute;
	ptr<BtRender> bt_render;
	ptr<BtControl> bt_control;
	ptr<BtMode> bt_mode;
	ptr<BtFPn> bt_global;
	ptr<BtFPn> bt_cmd;
	ptr<BtFPn> bt_par;
	ptr<BtFPn> bt_lang;
	ptr<BtFPn> bt_about;
	ptr<CtrlListY> cly;
	PnMenu(App& app);
};
