#pragma once
#include "app.h"
#include "var.h"

struct Arm;
struct Cell;
struct Agent;
struct Bullet;
struct Builder;
struct Faction;
struct Terrain;
struct CastCam;
struct cast_rec;
struct Broadcast;

struct UI;
struct Bgr;
struct clip;
struct param;
struct Cur : App {
	ptr<UI> ui;
	ptr<Bgr> bgr;

	Scope gl;
	wstring dbstr, cmd, tmp_cmd;
	vector<ptr<param>> pars;
	map<wstring, ptr<tile>> tiles;

	vec2 tl_grid;
	int nx_grid = 0, ny_grid = 0;
	vector<ptr<Cell>> grid;

	ptr<Builder> builder;
	Terrain* terrain_sel = NULL;
	vector<ptr<Terrain>> terrains;
	Faction* faction_sel = NULL;
	vector<ptr<Faction>> factions;
	Arm* arm_sel = NULL;
	vector<ptr<Arm>> arms;

	Agent* agent_sel = NULL;
	vector<ptr<Agent>> agents;
	vector<ptr<Bullet>> bullets;

	ptr<Broadcast> broad;
	bool edit = false;
	bool player = false;
	bool paused = false;
	bool auto_follow = false;
	double real_dt = 0;
	double max_real_dt = 0;

	// 这些都是非常临时的变量。
	// 以后的版本肯定要改的。
	double r_agent = 0;
	double acc = 0;
	double max_v = 0;
	double h_agent = 0;
	double top_sign = 0;
	double top_health = 0;
	double h_sign = 0;
	double w_health = 0;
	double h_health = 0;
	double top_bullet = 0;
	double h_bullet = 0;

	ptr<tile> sign_target, gun;
	ptr<tile> t_melee_ready, t_melee_send, t_melee_bad;
	ptr<tile> t_ranged_ready, t_ranged_send, t_ranged_bad;

	bool cast = false;
	double h_wall = 0;
	int gap_ceil = 0;
	int gap_floor = 0;
	bool simple_ceil = false;
	bool simple_floor = false;
	dcol c_ceil, c_floor;
	ptr<CastCam> cam;
	vector<ptr<cast_rec>> recs;
	vector<dcol> canvas;
	int nx_canvas = 0, ny_canvas = 0;

	bool mute = false;
	ptr<clip> cl0, cl1;
	clip* cl = NULL;
	double vol = 0;

	Cur();
	void Save(wstring const& nm) const;
	void Load(wstring const& nm);
	void Reset();
	void Update() override;
	ptr<tile> get_tile(wstring s);
	void load_tile(wstring const& s, int w = 0, int h = 0);
	void refill_tile(wstring const& out, wstring const& in, dcol c);

	void save_par(FILE* f) const;
	void load_par(FILE* f);

	void set_cfg(Var const &v);
	void init_def_fun();
	void basic_update();
};
