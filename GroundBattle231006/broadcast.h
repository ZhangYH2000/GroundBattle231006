#pragma once
#include "font.h"

struct msg {
	wstring txt;
	dcol c;
	double t_left = 0;

	msg() = default;
	msg(wstring const& txt, dcol c, double t_left) :
		txt(txt), c(c), t_left(t_left) {}
};

struct Cur;
struct Broadcast {
	deque<msg> msgs;
	double t_move = 0;

	void add_msg(wstring const& txt, dcol c = dcol(255));
	void render(Cur& cur);
	void Update(Cur& cur);
};
