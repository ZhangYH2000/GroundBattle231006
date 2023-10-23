#include "pn_menu.h"
#include "ui.h"
#include "cur.h"
#include "fpn_global.h"
#include "fpn_cmd.h"
#include "fpn_par.h"
#include "fpn_lang.h"
#include "fpn_about.h"

#define cur (*(Cur*)&app)
#include "my_def.h"
#define mkbt(nm) mkp(bt_##nm)(&*ui.fpn_##nm);

BtPause::BtPause() : Button(150) {}
void BtPause::Update(App& app) {
	txt = cur.paused ? loc(L"start") : loc(L"pause");
	Button::Update(app);
}
void BtPause::OnClick(App& app) { cur.paused = !cur.paused; }

BtMute::BtMute() : Button(150) {}
void BtMute::Update(App& app) {
	txt = cur.mute ? loc(L"unmute") : loc(L"mute");
	Button::Update(app);
}
void BtMute::OnClick(App& app) { cur.mute = !cur.mute; }

BtRender::BtRender() : Button(150) {}
void BtRender::Update(App& app) {
	txt = cur.cast ? loc(L"render_3d") : loc(L"render_2d");
	Button::Update(app);
}
void BtRender::OnClick(App& app) { cur.cast = !cur.cast; }

BtControl::BtControl() : Button(150) {}
void BtControl::Update(App& app) {
	txt = cur.player ? loc(L"control_player") : loc(L"control_ai");
	Button::Update(app);
}
void BtControl::OnClick(App& app) { cur.player = !cur.player; }

BtMode::BtMode() : Button(150) {}
void BtMode::Update(App& app) {
	txt = cur.edit ? loc(L"mode_edit") : loc(L"mode_observe");
	Button::Update(app);
}
void BtMode::OnClick(App& app) { cur.edit = !cur.edit; }


PnMenu::PnMenu(App& app) : Panel(Panel::menu(app)) {
	vector<Control*> tmp;
	mkp(bt_pause)(); mkp(bt_mute)();
	mkp(bt_render)(); mkp(bt_control)(); mkp(bt_mode)();
	mkbt(global);
	mkbt(cmd); mkbt(par);
	mkbt(lang); mkbt(about);
	tmp = { &*bt_pause, &*bt_mute, &*bt_render, &*bt_control, &*bt_mode,
		&*bt_global, &*bt_cmd, &*bt_par, &*bt_lang, &*bt_about };
	mkcl(cly); c = &*cly; Init(app);
}
