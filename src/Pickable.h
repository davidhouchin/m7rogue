// See LICENSE.txt for licensing information

class TargetSelector : public Persistent {
public:
    enum SelectorType {
        SELF,
        CLOSEST_MONSTER,
        SELECTED_MONSTER,
        WEARER_RANGE,
        SELECTED_RANGE
    };

    TargetSelector(SelectorType type, float range);
    void selectTargets(Actor *wearer, TCODList<Actor *> & list);
    void load(TCODZip &zip);
    void save(TCODZip &zip);

protected:
    SelectorType type;
    float range;
};

class Effect : public Persistent {
public:
    enum EffectType {
        HEALTH,
        AI_CHANGE
    };

    virtual bool applyTo(Actor *actor) = 0;
    static Effect *create(TCODZip &zip);
};

class HealthEffect : public Effect {
public:
    float amount;
    const char *message;

    HealthEffect(float amount, const char *message);
    bool applyTo(Actor *actor);

    void load(TCODZip &zip);
    void save(TCODZip &zip);
};

class AiChangeEffect : public Effect {
public:
    TemporaryAi *newAi;
    const char *message;

    AiChangeEffect(TemporaryAi *newAi, const char *message);
    bool applyTo(Actor *actor);

    void load(TCODZip &zip);
    void save(TCODZip &zip);
};

class Pickable : public Persistent {
public:
    Pickable(TargetSelector *selector, Effect *effect);
    virtual ~Pickable();
	bool pick(Actor *owner, Actor *wearer);
	virtual bool use(Actor *owner, Actor *wearer);
	void drop(Actor *owner, Actor *wearer);
    void load(TCODZip &zip);
    void save(TCODZip &zip);

protected:
    TargetSelector *selector;
    Effect *effect;
};
