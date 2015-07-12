// See LICENSE.txt for licensing information

class Persistent {
public:
	virtual void load(TCODZip &zip) = 0;
	virtual void save(TCODZip &zip) = 0;
};
