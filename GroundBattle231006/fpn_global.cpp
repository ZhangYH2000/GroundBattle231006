#include "fpn_global.h"
#include "ui.h"
#include "cur.h"

#define cur (*(Cur*)&app)
#include "my_def.h"

BtSave::BtSave() : BtLan(80, L"bt_save") {}
void BtSave::OnClick(App& app) { cur.Save(ui.fpn_global->tb_save->str); }
BtLoad::BtLoad() : BtLan(80, L"bt_load") {}
void BtLoad::OnClick(App& app) { cur.Load(ui.fpn_global->tb_load->str); }
CbAutoFollow::CbAutoFollow() : CbLan(L"cb_auto_follow") {}
void CbAutoFollow::Sync(App& app) { checked = cur.auto_follow; }
void CbAutoFollow::Upload(App& app) const { cur.auto_follow = checked; };

FPnGlobal::FPnGlobal(App& app) : FPnLan(app, 400, 600, L"fpn_global") {
	vector<Control*> tmp;
	mkp(tb_save)(270); mkp(bt_save)();
	tmp = { &*tb_save, &*bt_save }; mkcl(clx_save);
	mkp(tb_load)(270); mkp(bt_load)();
	tmp = { &*tb_load, &*bt_load }; mkcl(clx_load);
	mkp(cb_auto_follow)();
	tmp = { &*clx_save, &*clx_load, &*cb_auto_follow };
	mkcl(cly); c = &*cly; Init(app);
}
