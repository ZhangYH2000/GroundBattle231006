#include "bgr.h"
#include "cur.h"
#include "draw_str.h"
#include "draw_tile.h"

Bgr::Bgr(Cur& cur) {
	w = 1630; h = 860;
	tl = { 170, 0 }; dep = -100000;
	black = tile(w, h, dcol{}, 255);
}

#include "my_def.h"

void Bgr::render(Cur& cur) {
	if (!cur.cast) {
		draw_tile_raw(scr, tl, scr.rect(), black, black.rect());
	}
}

void Bgr::Update(Cur& cur) {
	hovered = (hvd == this);
	wheeled = (whd == this);

	if (hovered && msc(0) && !cur.cast) { cur.agent_sel = NULL; }
	if (dragged) {
		dragged = msd[1] && !cur.cast;
		tlgd += vec2(msp - msp_old);
	} else {
		dragged = !cur.cast && wheeled && msc(1);
	}

	render(cur);
}
void Bgr::PreUpdate(Cur& cur) {
	bool ok = dhv <= dep && insd(msp, vp());
	if (ok) { dhv = dep; hvd = this; }

	ok = dwh <= dep && insd(msp, vp());
	if (ok) { dwh = dep; whd = this; }
}
