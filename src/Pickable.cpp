// See LICENSE.txt for licensing information

#include "main.h"

TargetSelector::TargetSelector(SelectorType type, float range) :
    type(type), range(range) {}

void TargetSelector::selectTargets(Actor *wearer, TCODList<Actor *> &list) {
    switch (type) {
        case SELF:
        {
            list.push(wearer);
        }
        break;
        case CLOSEST_MONSTER:
        {
            Actor *closestMonster = engine.getClosestMonster(wearer->x, wearer->y, range);
            if (closestMonster) {
                list.push(closestMonster);
            }
        }
        break;
        case SELECTED_MONSTER:
        {
            int x, y;
            engine.gui->message(TCODColor::cyan, "Left-click to select an enemy,\nor right-click to cancel ");
            if (engine.pickATile(&x, &y, range)) {
                Actor *actor = engine.getActor(x, y);
                if (actor) {
                    list.push(actor);
                }
            }
        }
        break;
        case WEARER_RANGE:
        {
            for (Actor **iterator = engine.actors.begin();
                iterator != engine.actors.end(); iterator++) {
                Actor *actor = *iterator;
                if (actor != wearer && actor->destructible && !actor->destructible->isDead() &&
                    actor->getDistance(wearer->x, wearer->y) <= range) {
                    list.push(actor);
                }
            }
        }
        break;
        case SELECTED_RANGE:
        {
            int x, y;
            engine.gui->message(TCODColor::cyan, "Left-click to select an enemy,\nor right-click to cancel ");
            if (engine.pickATile(&x, &y, range)) {
                for (Actor **iterator = engine.actors.begin();
                     iterator != engine.actors.end(); iterator++) {
                    Actor *actor = *iterator;
                    if (actor->destructible && !actor->destructible->isDead() &&
                            actor->getDistance(x, y) <= range) {
                        list.push(actor);
                    }
                }
            }
        }
        break;
    }

    if (list.isEmpty()) {
        engine.gui->message(TCODColor::lightGrey, "No enemy is close enough");
    }
}

void TargetSelector::save(TCODZip &zip) {
    zip.putInt(type);
    zip.putInt(range);
}

void TargetSelector::load(TCODZip &zip) {
    type = (SelectorType)zip.getInt();
    range = zip.getInt();
}

Effect *Effect::create(TCODZip &zip) {
    EffectType type = (EffectType)zip.getInt();
    Effect *effect = NULL;
    switch (type) {
        case HEALTH: effect = new HealthEffect(0, NULL); break;
        case AI_CHANGE: effect = new AiChangeEffect(NULL, NULL); break;
    }
    effect->load(zip);
    return effect;
}

HealthEffect::HealthEffect(float amount, const char *message) :
    amount(amount), message(message) {}

bool HealthEffect::applyTo(Actor *actor) {
    if (!actor->destructible) return false;

    if (amount > 0) {
        float pointsHealed = actor->destructible->heal(amount);

        if (pointsHealed > 0) {
            if (message) {
                engine.gui->message(TCODColor::lightGrey, message, actor->name, pointsHealed);
            }

            return true;
        }
    } else {
        if (message && -amount - actor->destructible->defense > 0) {
            engine.gui->message(TCODColor::lightGrey, message, actor->name,
                                -amount - actor->destructible->defense);
        }
        if (actor->destructible->takeDamage(actor, -amount) > 0) {
            return true;
        }
    }

    return false;
}

void HealthEffect::save(TCODZip &zip) {
    zip.putInt(Effect::HEALTH);
    zip.putInt(amount);
    zip.putString(message);
}

void HealthEffect::load(TCODZip &zip) {
    amount = zip.getInt();
    message = strdup(zip.getString());
}

AiChangeEffect::AiChangeEffect(TemporaryAi *newAi, const char *message) :
    newAi(newAi), message(message) {}

bool AiChangeEffect::applyTo(Actor *actor) {
    newAi->applyTo(actor);
    if (message) {
        engine.gui->message(TCODColor::lightGrey, message, actor->name);
    }
    return true;
}

void AiChangeEffect::save(TCODZip &zip) {
    zip.putInt(Effect::AI_CHANGE);
    newAi->save(zip);
    zip.putString(message);
}

void AiChangeEffect::load(TCODZip &zip) {
    newAi = dynamic_cast<TemporaryAi*>(Ai::create(zip));
    message = strdup(zip.getString());
}

Pickable::Pickable(TargetSelector *selector, Effect *effect) :
    selector(selector), effect(effect) {}

Pickable::~Pickable() {
    if (selector) delete selector;
    if (effect) delete effect;
}

bool Pickable::pick(Actor *owner, Actor *wearer) {
	if (wearer->container && wearer->container->add(owner)) {
		engine.actors.remove(owner);
		return true;
	}
	return false;
}

bool Pickable::use(Actor *owner, Actor *wearer) {
    TCODList<Actor *> list;
    if (selector) {
        selector->selectTargets(wearer, list);
    }

    bool succeed = false;
    for (Actor **it = list.begin(); it != list.end(); it++) {
        if (effect->applyTo(*it)) {
            succeed = true;
        }
    }

    if (succeed) {
        if (wearer->container) {
            wearer->container->remove(owner);
            delete owner;
        }
    }

    return succeed;
}

void Pickable::drop(Actor *owner, Actor *wearer) {
	if (wearer->container) {
		wearer->container->remove(owner);
		engine.actors.push(owner);
		owner->x = wearer->x;
		owner->y = wearer->y;
		engine.gui->message(TCODColor::lightGrey, "%s drops a %s",
			wearer->name, owner->name);
	}
}

void Pickable::save(TCODZip &zip) {
    selector->save(zip);
    effect->save(zip);
}

void Pickable::load(TCODZip &zip) {
    selector = new TargetSelector(TargetSelector::SELF, 0);
    selector->load(zip);
    effect = Effect::create(zip);
}
