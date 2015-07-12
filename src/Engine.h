// See LICENSE.txt for licensing information

class Engine {
public:
	enum GameStatus {
		STARTUP,
		IDLE,
		NEW_TURN,
		VICTORY,
		DEFEAT
	} gameStatus;

	TCODList<Actor *> actors;
	Actor *player;
    Actor *stairs;
	Map *map;
	int fovRadius;
    bool fullscreen;
	int screenWidth;
	int screenHeight;
    int level;
	Gui *gui;
	TCOD_key_t lastKey;
	TCOD_mouse_t mouse;

	Engine(int screenWidth, int screenHeight);
	~Engine();
	void init();
    void term();
	void load();
	void save();
	void update();
	void render();
	void sendToBack(Actor *actor);
	Actor *getClosestMonster(int x, int y, float range) const;
	bool pickATile(int *x, int *y, float maxRange = 0.0f);
	Actor *getActor(int x, int y) const;
    void nextLevel();
};

extern Engine engine;
