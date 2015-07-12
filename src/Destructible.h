// See LICENSE.txt for licensing information

class Destructible : public Persistent {
public:
	float maxHp; // max health points
	float hp; // current health points
	float defense; // hit points deflected
	const char *corpseName; // actor's name when destroyed
    int xp; // XP gained when killing this

    Destructible(float maxHp, float defense, const char *corpseName, int xp);
	inline bool isDead() {return hp <= 0;}
	float takeDamage(Actor *owner, float damage);
	virtual void die(Actor *owner);
	float heal(float amount);

	void load(TCODZip &zip);
	void save(TCODZip &zip);
	static Destructible *create(TCODZip &zip);

protected:
	enum DestructibleType {
		MONSTER, PLAYER
	};
};

class MonsterDestructible : public Destructible {
public:
    MonsterDestructible(float maxHp, float defense, const char *corpseName, int xp);
	void die(Actor *owner);
	void save(TCODZip &zip);
};

class PlayerDestructible : public Destructible {
public:
	PlayerDestructible(float maxHp, float defense, const char *corpseName);
	void die(Actor *owner);
	void save(TCODZip &zip);
};