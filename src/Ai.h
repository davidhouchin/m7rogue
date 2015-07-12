// See LICENSE.txt for licensing information

// amount of turns for monster to smell player scent
static const int SCENT_THRESHOLD = 20;

class Ai : public Persistent {
public:
	virtual void update(Actor *owner) = 0;
	static Ai *create(TCODZip &zip);

protected:
	enum AiType {
		MONSTER, CONFUSED_MONSTER, PLAYER
	};
};

class PlayerAi : public Ai {
public:
	void update(Actor *owner);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
    int xpLevel;
    PlayerAi();
    int getNextLevelXp();

protected:
	bool moveOrAttack(Actor *owner, int targetx, int targety);
	void handleActionKey(Actor *owner, int ascii);
	Actor *choseFromInventory(Actor *owner);
};

class MonsterAi : public Ai {
public:
	void update(Actor *owner);
	void load(TCODZip &zip);
	void save(TCODZip &zip);

protected:
	void moveOrAttack(Actor *owner, int targetx, int targety);
};

class TemporaryAi : public Ai {
public:
    TemporaryAi(int nbTurns);
    void update(Actor *owner);
    void applyTo(Actor *actor);
    void load(TCODZip &zip);
    void save(TCODZip &zip);

protected:
    int nbTurns;
    Ai *oldAi;
};

class ConfusedMonsterAi : public TemporaryAi {
public:
    ConfusedMonsterAi(int nbTurns);
	void update(Actor *owner);
	void load(TCODZip &zip);
	void save(TCODZip &zip);
};
