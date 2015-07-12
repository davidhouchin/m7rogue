// See LICENSE.txt for licensing information

class Actor : public Persistent {
public:
	int x, y; // position
	int ch; // ascii character code
	TCODColor col; // color
	const char *name; // actor's name
	bool blocks; // can be walked on or not
    bool fovOnly; // only display when in fov
	Attacker *attacker; // can deal damage
	Destructible *destructible; // can be damaged
	Ai *ai; // updates self
	Pickable *pickable; // something that can be picked up
	Container *container; // something that can contain actors

	Actor(int x, int y, int ch, const char *name, const TCODColor &col);
	virtual ~Actor();
	void update();
	void render() const;
	void load(TCODZip &zip);
	void save(TCODZip &zip);
	float getDistance(int cx, int cy) const;
};
