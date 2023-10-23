#pragma once
#include "label.h"
#include "button.h"
#include "textbox.h"
#include "checkbox.h"
#include "ctrl_list.h"
#include "float_panel.h"

struct BtSave : BtLan {
	BtSave();
	void OnClick(App& app) override;
};
struct BtLoad : BtLan {
	BtLoad();
	void OnClick(App& app) override;
};
struct CbAutoFollow : CbLan {
	CbAutoFollow();
	void Sync(App& app) override;
	void Upload(App& app) const override;
};


struct FPnGlobal : FPnLan {
	ptr<Textbox> tb_save;
	ptr<BtSave> bt_save;
	ptr<CtrlListX> clx_save;
	ptr<Textbox> tb_load;
	ptr<BtLoad> bt_load;
	ptr<CtrlListX> clx_load;
	ptr<CbAutoFollow> cb_auto_follow;
	ptr<CtrlListY> cly;
	FPnGlobal(App& app);
};
