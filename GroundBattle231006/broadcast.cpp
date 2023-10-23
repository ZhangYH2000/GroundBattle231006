#include "broadcast.h"
#include "cur.h"
#include "bgr.h"
#include "draw_str.h"

#include "my_def.h"

double constexpr max_t_move = 0.1;
double constexpr max_t_left = 3.5;

void Broadcast::add_msg(wstring const& txt, dcol c) {
	msgs.push_back(msg(txt, c, max_t_left));
}
void Broadcast::render(Cur& cur) {
	rep(i, 0, msgs.size()) {
		dvec tl =
			bgr.tl + dvec(0, bgr.h) -
			dvec(0, ft.h * (i + 1)) +
			dvec(0, ft.h * t_move / max_t_move) + dvec(10, -10);
		draw_str(scr, dscr, 20, msgs[i].txt, msgs[i].c, ft, tl, 0, bgr.vp());
	}
}
void Broadcast::Update(Cur& cur) {
	if (!cur.paused)
	for (auto& m : msgs) { m.t_left -= rdt; }
	bool move =
		!msgs.empty()
		&& msgs.front().t_left <= 0;
	if (move) {
		t_move += rdt;
		if (t_move > max_t_move) {
			t_move = 0;
			msgs.pop_front();
		}
	} render(cur);
}
