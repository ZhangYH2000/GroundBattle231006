#pragma once
#include "vec2.h"

int constexpr BD_NONE = 0;
int constexpr BD_RECT = 1;
int constexpr BD_LINE = 2;

struct Cur;
struct Builder {
	int mode = 0;
	dvec p_now, p_rec;
	bool ms_in = false;
	bool hovered = false;

	void render(Cur& cur);
	void render_rect(Cur& cur);
	void render_line(Cur& cur);
	void edit_rect(Cur& cur);
	void edit_line(Cur& cur);
	void Update(Cur& cur);
	void Discard(Cur& cur);
	void PreUpdate(Cur& cur);
};
