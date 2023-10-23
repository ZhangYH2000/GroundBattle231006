#include "fpn_cmd.h"
#include "cur.h"
#include "var.h"

#define cur (*(Cur*)&app)
#include "my_def.h"

RTbCmd::RTbCmd() : RichTextbox(560, 500) {}
void RTbCmd::Sync(App& app) { str = cur.tmp_cmd; }
void RTbCmd::OnDone(App& app) const { cur.tmp_cmd = str; }

BtCompile::BtCompile() : BtLan(200, L"compile") {}
void BtCompile::OnClick(App& app) {
	cur.cmd = cur.tmp_cmd;
	Execute(gl, Compile(cur.cmd));
}

BtSvCmd::BtSvCmd() : BtLan(200, L"bt_sv_cmd") {}
void BtSvCmd::OnClick(App& app) {
	cur.cmd = cur.tmp_cmd;
}


FPnCmd::FPnCmd(App& app) : FPnLan(app, 600, 600, L"fpn_cmd") {
	vector<Control*> tmp;
	mkp(rtb)();
	mkp(bt_compile)(); mkp(bt_sv_cmd)();
	tmp = { &*bt_compile, &*bt_sv_cmd };
	mkcl(clx_bt);
	tmp = { &*rtb, &*clx_bt };
	mkcl(cly); c = &*cly; Init(app);
}
