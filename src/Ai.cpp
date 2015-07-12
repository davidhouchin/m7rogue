// See LICENSE.txt for licensing information

#include <math.h>
#include "main.h"

// number of turns monsters will chase after player after losing sight
static const int TRACKING_TURNS = 3;

const int LEVEL_UP_BASE = 200;
const int LEVEL_UP_FACTOR = 150;

Ai *Ai::create(TCODZip &zip) {
	AiType type = (AiType)zip.getInt();
	Ai *ai = NULL;
	switch (type) {
		case PLAYER: ai = new PlayerAi(); break;
		case MONSTER: ai = new MonsterAi(); break;
        case CONFUSED_MONSTER: ai = new ConfusedMonsterAi(0); break;
	}
	ai->load(zip);
	return ai;
}

PlayerAi::PlayerAi() : xpLevel(1) {}

int PlayerAi::getNextLevelXp() {
    return LEVEL_UP_BASE + xpLevel * LEVEL_UP_FACTOR;
}

void PlayerAi::update(Actor *owner) {
    int levelUpXp = getNextLevelXp();
    if (owner->destructible->xp >= levelUpXp) {
        xpLevel++;
        owner->destructible->xp -= levelUpXp;
        engine.gui->message(TCODColor::yellow, "Your skills grow stronger! You reached level %d", xpLevel);
        engine.gui->menu.clear();
        engine.gui->menu.addItem(Menu::CONSTITUTION, "Constitution (+20HP)");
        engine.gui->menu.addItem(Menu::STRENGTH, "Strength (+1 attack)");
        engine.gui->menu.addItem(Menu::AGILITY, "Agility (+1 defense)");
        Menu::MenuItemCode menuItem = engine.gui->menu.pick(Menu::PAUSE);

        switch (menuItem) {
            case Menu::CONSTITUTION:
                owner->destructible->maxHp += 20;
                owner->destructible->hp += 20;
            break;
            case Menu::STRENGTH:
                owner->attacker->power += 1;
                break;
            case Menu::AGILITY:
                owner->destructible->defense += 1;
                break;
            default: break;
        }
    }

	if (owner->destructible && owner->destructible->isDead()) {
		return;
	}

	int dx = 0, dy = 0;
	switch (engine.lastKey.vk) {
		case TCODK_UP: dy = -1; break;
		case TCODK_DOWN: dy = 1; break;
		case TCODK_LEFT: dx = -1; break;
		case TCODK_RIGHT: dx = 1; break;
		case TCODK_CHAR: handleActionKey(owner, engine.lastKey.c); break;
		default: break;
	}

	if (dx != 0 || dy != 0) {
		engine.gameStatus = Engine::NEW_TURN;
		if (moveOrAttack(owner, owner->x + dx, owner->y + dy)) {
			engine.map->computeFov();
		}
	}
}

void PlayerAi::load(TCODZip &zip) {
    xpLevel = zip.getInt();
}

void PlayerAi::save(TCODZip &zip) {
	zip.putInt(PLAYER);
    zip.putInt(xpLevel);
}

bool PlayerAi::moveOrAttack(Actor *owner, int targetx, int targety) {
	if (engine.map->isWall(targetx, targety)) return false;

	for (Actor **iterator = engine.actors.begin();
		iterator != engine.actors.end(); iterator++) {
		Actor *actor = *iterator;
		if (actor->destructible && !actor->destructible->isDead() &&
			actor->x == targetx && actor->y == targety) {
			owner->attacker->attack(owner, actor);
			return false;
		}
	}

	for (Actor **iterator = engine.actors.begin();
		iterator != engine.actors.end(); iterator++) {
		Actor *actor = *iterator;
		bool corpseOrItem = (actor->destructible && actor->destructible->isDead()) || actor->pickable;
		if (corpseOrItem &&
			actor->x == targetx && actor->y == targety) {
			engine.gui->message(TCODColor::lightGrey, "There's a %s here", actor->name);
		}
	}

	owner->x = targetx;
	owner->y = targety;
	return true;
}

void PlayerAi::handleActionKey(Actor *owner, int ascii) {
	switch (ascii) {
		case 'g':
		{
			bool found = false;
			for (Actor **iterator = engine.actors.begin();
				iterator != engine.actors.end(); iterator++) {
				Actor *actor = *iterator;
				if (actor->pickable && actor->x == owner->x && actor->y == owner->y) {
					if (actor->pickable->pick(actor, owner)) {
						found = true;
						engine.gui->message(TCODColor::lightGrey, "You pick the %s", actor->name);
						break;
					} else if (!found) {
						found = true;
						engine.gui->message(TCODColor::red, "Your inventory is full");
					}
				}
			}
			if (!found) {
				engine.gui->message(TCODColor::lightGrey, "There's nothing here to pick");
			}
			engine.gameStatus = Engine::NEW_TURN;
		}
		break;
		case 'i':
		{
			Actor *actor = choseFromInventory(owner);
			if (actor) {
				actor->pickable->use(actor, owner);
				engine.gameStatus = Engine::NEW_TURN;
			}
		}
		break;
		case 'd':
		{
			Actor *actor = choseFromInventory(owner);
			if (actor) {
				actor->pickable->drop(actor, owner);
				engine.gameStatus = Engine::NEW_TURN;
			}
		}
		break;
        case '>':
        {
            if (engine.stairs->x == owner->x && engine.stairs->y == owner->y) {
                engine.nextLevel();
            } else {
                engine.gui->message(TCODColor::lightGrey, "There are no stairs here");
            }
        }
        break;
	}
}

Actor *PlayerAi::choseFromInventory(Actor *owner) {
	static const int INVENTORY_WIDTH = 50;
	static const int INVENTORY_HEIGHT = 28;
	static TCODConsole con(INVENTORY_WIDTH, INVENTORY_HEIGHT);

	// display inventory frame
	con.setDefaultForeground(TCODColor(200, 180, 50));
	con.printFrame(0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT, true, TCOD_BKGND_DEFAULT, "inventory");

	// display items with their shortcuts
	con.setDefaultForeground(TCODColor::white);
	int shortcut = 'a';
	int y = 1;
	for (Actor **it = owner->container->inventory.begin();
		it != owner->container->inventory.end(); it++) {
		Actor *actor = *it;
		con.print(2, y, "(%c) %s", shortcut, actor->name);
		y++;
		shortcut++;
	}

	// blit inventory console to root console
	TCODConsole::blit(&con, 0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT,
		TCODConsole::root, engine.screenWidth/2 - INVENTORY_WIDTH/2,
		engine.screenHeight/2 - INVENTORY_HEIGHT/2);
	TCODConsole::flush();

	// wait for key
	TCOD_key_t key;
	TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL, true);

	if (key.vk == TCODK_CHAR) {
		int actorIndex = key.c - 'a';

		if (actorIndex >= 0 && actorIndex < owner->container->inventory.size()) {
			return owner->container->inventory.get(actorIndex);
		}
	}

	return NULL;
}

void MonsterAi::update(Actor *owner) {
	if (owner->destructible && owner->destructible->isDead()) {
		return;
	}

    moveOrAttack(owner, engine.player->x, engine.player->y);
}

void MonsterAi::moveOrAttack(Actor *owner, int targetx, int targety) {
	int dx = targetx - owner->x;
	int dy = targety - owner->y;
	float distance = sqrtf(dx * dx + dy * dy);

    if (distance < 2) {
        if (owner->attacker) {
            owner->attacker->attack(owner, engine.player);
        }
        return;
    } else if (engine.map->isInFov(owner->x, owner->y)) {
        dx = (int)(round(dx/distance));
        dy = (int)(round(dy/distance));
        if (engine.map->canWalk(owner->x + dx, owner->y+dy)) {
            owner->x += dx;
            owner->y += dy;
            return;
        }
    }

    unsigned int bestLevel = 0;
    int bestCellIndex = -1;
    static int tdx[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
    static int tdy[8] = {-1, -1, -1, 0, 0, 1, 1, 1};

    for (int i = 0; i < 8; i++) {
        int cellx = owner->x + tdx[i];
        int celly = owner->y + tdy[i];
        if (engine.map->canWalk(cellx, celly)) {
            unsigned int cellScent = engine.map->getScent(cellx, celly);
            if (cellScent > engine.map->currentScentValue - SCENT_THRESHOLD &&
                    cellScent > bestLevel) {
                bestLevel = cellScent;
                bestCellIndex = i;
            }
        }
    }

    if (bestCellIndex != -1) {
        owner->x += tdx[bestCellIndex];
        owner->y += tdy[bestCellIndex];
    }
}

void MonsterAi::load(TCODZip &zip) {}

void MonsterAi::save(TCODZip &zip) {
	zip.putInt(MONSTER);
}

TemporaryAi::TemporaryAi(int nbTurns) : nbTurns(nbTurns), oldAi(NULL) {}

void TemporaryAi::update(Actor *owner) {
    nbTurns--;
    if (nbTurns == 0) {
        owner->ai = oldAi;
        delete this;
    }
}

void TemporaryAi::applyTo(Actor *actor) {
    oldAi = actor->ai;
    actor->ai = this;
}

void TemporaryAi::save(TCODZip &zip) {
    zip.putInt(oldAi != NULL);
    if (oldAi) oldAi->save(zip);
}

void TemporaryAi::load(TCODZip &zip) {
    bool hasAi = zip.getInt();
    if (hasAi) oldAi = Ai::create(zip);
}

ConfusedMonsterAi::ConfusedMonsterAi(int nbTurns) :
    TemporaryAi(nbTurns) {}

void ConfusedMonsterAi::update(Actor *owner) {
	TCODRandom *rng = TCODRandom::getInstance();
	int dx = rng->getInt(-1, 1);
	int dy = rng->getInt(-1, 1);

	if (dx != 0 || dy != 0) {
		int destx = owner->x + dx;
		int desty = owner->y + dy;
		if (engine.map->canWalk(destx, desty)) {
			owner->x = destx;
			owner->y = desty;
		} else {
			Actor *actor = engine.getActor(destx, desty);
			if (actor) {
				owner->attacker->attack(owner, actor);
			}
		}
	}
    TemporaryAi::update(owner);
}

void ConfusedMonsterAi::save(TCODZip &zip) {
    zip.putInt(CONFUSED_MONSTER);
    zip.putInt(nbTurns);
    TemporaryAi::save(zip);
}

void ConfusedMonsterAi::load(TCODZip &zip) {
	nbTurns = zip.getInt();
    TemporaryAi::load(zip);
}
