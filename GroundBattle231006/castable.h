#pragma once

struct Cur;
struct Castable {
	virtual ~Castable() {}
	virtual double GetD() const { return 0; }
	virtual void Cast(Cur& cur) {}
};
struct comp_cast {
	bool operator()(Castable* c0, Castable* c1) const {
		return c0->GetD() > c1->GetD();
	}
};
