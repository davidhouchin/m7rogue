// See LICENSE.txt for licensing information

#include <math.h>
#include "main.h"

Actor::Actor(int x, int y, int ch, const char *name, const TCODColor &col) :
	x(x), y(y), ch(ch), col(col), name(name),
    blocks(true), fovOnly(true), attacker(NULL), destructible(NULL), ai(NULL),
	pickable(NULL), container(NULL) {}

Actor::~Actor() {
	if (attacker) delete attacker;
	if (destructible) delete destructible;
	if (ai) delete ai;
	if (pickable) delete pickable;
	if (container) delete container;
}

void Actor::render() const {
	TCODConsole::root->setChar(x, y, ch);
	TCODConsole::root->setCharForeground(x, y, col);
}

void Actor::update() {
	if (ai) ai->update(this);
}

void Actor::save(TCODZip &zip) {
	zip.putInt(x);
	zip.putInt(y);
	zip.putInt(ch);
	zip.putColor(&col);
	zip.putString(name);
	zip.putInt(blocks);
    zip.putInt(fovOnly);

	zip.putInt(attacker != NULL);
	zip.putInt(destructible != NULL);
	zip.putInt(ai != NULL);
	zip.putInt(pickable != NULL);
	zip.putInt(container != NULL);

	if (attacker) attacker->save(zip);
	if (destructible) destructible->save(zip);
	if (ai) ai->save(zip);
	if (pickable) pickable->save(zip);
	if (container) container->save(zip);
}

void Actor::load(TCODZip &zip) {
	x = zip.getInt();
	y = zip.getInt();
	ch = zip.getInt();
	col = zip.getColor();
    name = strdup(zip.getString());
	blocks = zip.getInt();
    fovOnly = zip.getInt();

	bool hasAttacker = zip.getInt();
	bool hasDestructible = zip.getInt();
	bool hasAi = zip.getInt();
	bool hasPickable = zip.getInt();
	bool hasContainer = zip.getInt();

	if (hasAttacker) {
		attacker = new Attacker(0.0f);
		attacker->load(zip);
	}

	if (hasDestructible) {
		destructible = Destructible::create(zip);
	}

	if (hasAi) {
		ai = Ai::create(zip);
	}

	if (hasPickable) {
        pickable = new Pickable(NULL, NULL);
        pickable->load(zip);
	}

	if (hasContainer) {
		container = new Container(0);
		container->load(zip);
	}
}

float Actor::getDistance(int cx, int cy) const {
	int dx = x - cx;
	int dy = y - cy;
	return sqrtf(dx * dx + dy * dy);
}
