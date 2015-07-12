// See LICENSE.txt for licensing information

#include "main.h"

Destructible::Destructible(float maxHp, float defense, const char *corpseName, int xp) :
    maxHp(maxHp), hp(maxHp), defense(defense), corpseName(corpseName), xp(xp) {}

float Destructible::takeDamage(Actor *owner, float damage) {
	damage -= defense;
	if (damage > 0) {
		hp -= damage;
		if (hp <= 0) {
			die(owner);
		}
	} else {
		damage = 0;
	}

	return damage;
}

void Destructible::die(Actor *owner) {
	owner->ch = '%';
	owner->col = TCODColor::darkRed;
	owner->name = corpseName;
	owner->blocks = false;
	engine.sendToBack(owner);
}

float Destructible::heal(float amount) {
	hp += amount;
	if (hp > maxHp) {
		amount -= hp - maxHp;
		hp = maxHp;
	}
	return amount;
}

void Destructible::load(TCODZip &zip) {
	maxHp = zip.getFloat();
	hp = zip.getFloat();
	defense = zip.getFloat();
	corpseName = strdup(zip.getString());
    xp = zip.getInt();
}

void Destructible::save(TCODZip &zip) {
	zip.putFloat(maxHp);
	zip.putFloat(hp);
	zip.putFloat(defense);
	zip.putString(corpseName);
    zip.putInt(xp);
}

Destructible *Destructible::create(TCODZip &zip) {
	DestructibleType type = (DestructibleType)zip.getInt();
	Destructible *destructible = NULL;
	switch (type) {
        case MONSTER: destructible = new MonsterDestructible(0, 0, NULL, 0); break;
        case PLAYER: destructible = new PlayerDestructible(0, 0, NULL); break;
	}
    destructible->load(zip);

	return destructible;
}

MonsterDestructible::MonsterDestructible(float maxHp, float defense, const char *corpseName, int xp) :
    Destructible(maxHp, defense, corpseName, xp) {}

void MonsterDestructible::die(Actor *owner) {
    engine.gui->message(TCODColor::lightGrey, "%s is dead. You gain %d xp", owner->name, xp);
    engine.player->destructible->xp += xp;
	Destructible::die(owner);
}

void MonsterDestructible::save(TCODZip &zip) {
	zip.putInt(MONSTER);
	Destructible::save(zip);
}

PlayerDestructible::PlayerDestructible(float maxHp, float defense, const char *corpseName) :
    Destructible(maxHp, defense, corpseName, 0) {}

void PlayerDestructible::die(Actor *owner) {
	engine.gui->message(TCODColor::red, "You died!");
	Destructible::die(owner);
	engine.gameStatus = Engine::DEFEAT;
}

void PlayerDestructible::save(TCODZip &zip) {
	zip.putInt(PLAYER);
	Destructible::save(zip);
}
