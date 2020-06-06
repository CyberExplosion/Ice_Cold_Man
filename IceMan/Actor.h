#ifndef ACTOR_H_
#define ACTOR_H_

#include <iostream>
#include "StudentWorld.h"
#include "GraphObject.h"
#include "GameController.h"
#include <vector>
// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

class IMovementBehavior;
class IActorResponse;
class IDetectionBehavior;
class IExistenceBehavior;
class RadarLikeDetection;
class SonarKit;
class GoldNuggets;
class Squirt;
class Boulder;
class StudentWorld;
class Ice;

class Actor : public GraphObject, public std::enable_shared_from_this<Actor> {
public:
	//Type
	enum ActorType { player, npc, worldStatic, hazard, ice, dropByPlayer, collect };
	ActorType type;

private:
	int hitpoints;
	int strength;
	double collisionRange; 
	double detectionRange; // If you enter this object's range then it will trigger other behaviors. Eg. If a player walks near gold the it will appear.
	StudentWorld* m_sw;
	int death_sound;
	int score;
	int size;
public:
	Actor(StudentWorld* world, ActorType t_type, bool visibility, int imgID, int startX, int startY, Direction dir = right, double t_size = 1.0, unsigned int depth = 0, int t_hp = 1, int t_strength = 0, double col_range = 0, double detect_range = 0, int t_sound = SOUND_NONE, int t_score = 0) : GraphObject(imgID, startX, startY, dir, t_size, depth), hitpoints(t_hp), strength(t_strength), collisionRange(col_range), detectionRange(detect_range), type(t_type), m_sw(world), death_sound(t_sound), size(t_size), score(t_score) {
		setVisible(visibility);
	};
	virtual ~Actor() {};

	//Represents the movement
	std::unique_ptr<IMovementBehavior> movementBehavior;

	//Store the result in this
	std::unique_ptr<IDetectionBehavior>collisionDetection;

	//Don't think about this
	std::unique_ptr<IActorResponse> collisionResult;

	//Determine what method of detection the actor would use
	std::unique_ptr<IDetectionBehavior>detectBehavior;

	//This is for all the usage of shared_ptr in them behaviors. This is bad but I blame due date
	virtual void resetAllBehaviors();

	StudentWorld* getWorld() {
		return m_sw;
	}

	bool isAlive() {
		return hitpoints > 0;
	}

	//Don't use it before derived
	virtual void doSomething() = 0;

	//added by nelson
	void setDetectRange(int range) {
		detectionRange = range;
	}

	int getDetectRange() {
		return detectionRange;
	}

	int getCollisionRange() {
		return collisionRange;
	}

	int getHealth() {
		return hitpoints;
	}

	int getStrength() {
		return strength;
	}

	virtual void dmgActor(int amt) {
		hitpoints -= amt;
	}

	int getDeathSound() {
		return death_sound;
	}

	void setDeathSound(int sound) {
		death_sound = sound;
	}

	int getScore() {
		return score;
	}

	void setScore(int val) {
		score = val;
	}

	void changeActorType(ActorType t_type) {
		type = t_type;
	}

};

//Movement Strategy
class IMovementBehavior {
protected:
	int key;
	bool allowMovement = true;
public:
	IMovementBehavior(int t_key, bool move = true) : key(t_key), allowMovement(move) {};
	virtual ~IMovementBehavior() {};
	virtual void moveThatAss() = 0;
	virtual void resetBehavior() = 0;

	void enableMove(bool flip) {
		allowMovement = flip;
	}

	bool canMove() {
		return allowMovement;
	}

	int getKey() {
		return key;
	}
	void setKey(int& t_key) {
		key = t_key;
	}
};

class FreeMovement : public IMovementBehavior {
private:

public:
	FreeMovement() : IMovementBehavior(INVALID_KEY) {};
	//This is the default movement for NPC
	void moveThatAss() override;
	void resetBehavior() override;
};

class FallMovement : public IMovementBehavior {
private:
	std::weak_ptr<Actor> pawn;
	//Variables
	int t = 30; // Adjust this to change how long before boulder falls.
	bool checkIceBelow();
	void fall(std::shared_ptr<Actor> actor);
public:
	FallMovement(std::weak_ptr<Actor> t_pawn, int key = INVALID_KEY) : IMovementBehavior(key), pawn(t_pawn) {};
	//This is for the Boulder
	void moveThatAss() override;
	void resetBehavior() override;
};

class ControlledMovement : public IMovementBehavior {
private:
	std::weak_ptr<Actor> pawn;
public:
	void resetBehavior() override;
	ControlledMovement(std::weak_ptr<Actor> t_pawn, int t_key) : IMovementBehavior(t_key), pawn(t_pawn) {}
	//This is for the player
	void moveThatAss() override;
};

class PursuingMovement : public IMovementBehavior {
private:
	std::weak_ptr<Actor>pawn;
	std::pair<int, int> destination;
public:
	PursuingMovement(std::weak_ptr<Actor> target, std::pair<int, int> t_dest) : IMovementBehavior(INVALID_KEY), pawn(target), destination(t_dest) {};
	//This is for whenever NPC chasing after a location
	void moveThatAss() override;
	void resetBehavior() override;
};

class SquirtMovement : public IMovementBehavior {
private:
	int travelDist;
	std::weak_ptr<Actor> squirt;
public:
	SquirtMovement(std::weak_ptr<Actor> t_s, int distTravel = 4) : IMovementBehavior(INVALID_KEY), squirt(t_s), travelDist(distTravel) {};
	//This is for the Squirt to travel
	void moveThatAss() override;
	void resetBehavior() override;
};

//////////////////////////

//Collision Strategy
class IActorResponse {
public:
	enum ResponseType { block, destroy, appear };
	ResponseType type;
protected:
	GraphObject::Direction facing;
public:
	IActorResponse(ResponseType type, GraphObject::Direction t_dir = GraphObject::Direction::none) : facing(t_dir) {};
	GraphObject::Direction getFacing() {
		return facing;
	}
	void setFacing(GraphObject::Direction t_dir) {
		facing = t_dir;
	}
	virtual void response() = 0;
	virtual void resetBehavior() = 0;
};

class Block : public IActorResponse {
private:
	std::weak_ptr<Actor>wp_target;
public:
	void resetBehavior() override;
	Block(std::weak_ptr<Actor> t_target) : IActorResponse(block), wp_target(t_target) {
		std::shared_ptr<Actor>temp = wp_target.lock();
		if (temp)
			facing = temp->getDirection();
	}
	//Object will be force to stand still
	void response() override;
};

class Destroy : public IActorResponse {
private:
	std::weak_ptr<Actor>wp_target;
	int dmgTaken;
public:
	void resetBehavior() override;
	Destroy(std::weak_ptr<Actor> t_target, int dmgTook) : IActorResponse(destroy), wp_target(t_target), dmgTaken(dmgTook) {};
	//Object will be force to reduce their health by an amount
	void response() override;
};

//This is only use for radar type collision
class Appear : public IActorResponse {
private:
	std::weak_ptr<Actor>target;
public:
	Appear(std::weak_ptr<Actor> wp_target) : IActorResponse(appear), target(wp_target) {};
	void resetBehavior() override;
	void response() override;
};

/////////////////////////

/////////////////////////
//Existence Strategy
class IExistenceBehavior {
private:
public:
	virtual void showYourself() = 0;
	virtual void resetBehavior() = 0;
};

class ExistTemporary : public IExistenceBehavior {
private:
	int deathTimer;
	std::weak_ptr<Actor> pawn;
public:
	ExistTemporary();
	ExistTemporary(std::weak_ptr<Actor> target, int time) : deathTimer(time), pawn(target) {}
	void showYourself() override;
	void resetBehavior() override;
	//void setTimeTillDeath(int time) { timeTillDeath = time; }
};

class ExistPermanently : public IExistenceBehavior {
private:
	std::weak_ptr<Actor>pawn;
public:
	ExistPermanently();
	ExistPermanently(std::weak_ptr<Actor> target) : pawn(target) {
		std::shared_ptr<Actor> s_target = pawn.lock();
		if (s_target)
			s_target->setVisible(true);
		s_target.reset();
	}
	void showYourself() override;
	void resetBehavior() override;
};

/////////////////////////
//Detection Strategy
//Collision detection and radar detection are the same type, just have different range
class IDetectionBehavior {
protected:

public:

	//The source is npc or objects. Intruders will be others actors that inside the range
	std::weak_ptr<Actor> wp_source;
	std::vector<std::weak_ptr<Actor>> wp_intruders;
	//Constructor
	IDetectionBehavior(std::weak_ptr<Actor> t_source) : wp_source(t_source) {};
	IDetectionBehavior(std::weak_ptr<Actor> t_source, std::vector<std::weak_ptr<Actor>> t_intruder) : wp_source(t_source), wp_intruders(t_intruder) {};

	//All of this is bad and I'm meant it. I wish whoever look at this in the future best of luck
	virtual void behaveBitches() = 0;
	void virtual resetBehavior() {
		wp_source.reset();
		for (auto& val : wp_intruders)
			val.reset();
	}
};

class LineOfSightDetection : public IDetectionBehavior {
private:
	bool seePlayer();
public:
	LineOfSightDetection(std::weak_ptr<Actor> t_source) : IDetectionBehavior(t_source) {};
	void behaveBitches() override;
};

class RadarLikeDetection : public IDetectionBehavior {
protected:
	std::vector<std::weak_ptr<Actor>> sensedIce();
	std::vector<std::weak_ptr<Actor>> sensedOthers(bool radarMode = false);
	void checkSurrounding(std::weak_ptr<Actor>t_source, bool radarMode = false);
public:
	RadarLikeDetection(std::weak_ptr<Actor> t_source, bool radarMode = false) : IDetectionBehavior(t_source) {
		if (radarMode)
			checkSurrounding(t_source, true);
		else
			checkSurrounding(t_source);
	};
	virtual bool collisionHappen();
	virtual void collide(std::weak_ptr<Actor> source, std::weak_ptr<Actor> receiver);
	void behaveBitches() override;
};

class CollisionDetection : public RadarLikeDetection {
private:
	//This functions determine the result of collision between 2 actors. The source will have <Block> or <Destroy> behavior
	//depend on what it collides into
	void collide(std::weak_ptr<Actor> source, std::weak_ptr<Actor> receiver) override;
	//bool collisionHappen() override;
public:
	CollisionDetection(std::weak_ptr<Actor> t_source) : RadarLikeDetection(t_source) {}
	void behaveBitches() override;
};

/////////////////////////

//People
class Characters : public Actor {
public:
	Characters(StudentWorld* world, ActorType type, int imgID, int startX, int startY, Direction dir, int t_hp, int t_str, double col_ran, double detect_range, int t_sound, int t_score) : Actor(world, type, true, imgID, startX, startY, dir, 1.0, 0, t_hp, t_str, col_ran, detect_range, t_sound, t_score) {};
	virtual ~Characters() {};
};

class IceMan : public Characters {
private:
	int dmgSound = SOUND_PLAYER_ANNOYED;
	int numGold = 10;
	int numSquirt = 5;
	int numSonar = 1;
	bool shootSquirt();
public:
	IceMan(StudentWorld* world, int startX = 30, int startY = 60) : Characters(world, player, IID_PLAYER, startX, startY, right, 10, 999, 3, 4, SOUND_PLAYER_GIVE_UP, 0) {};
	void doSomething() override;
	bool useGoodies(int key);
	void dmgActor(int amt) override;

	//Function for goodies
	void pickUpWater(int amt = 1) {
		numSquirt += amt;
	}
	void pickUpSonar(int amt = 1) {
		numSonar += amt;
	}
	void pickUpGold(int amt = 1) {
		numGold += amt;
	}
	int getWater() {
		return numSquirt;
	}
	int getGold() {
		return numGold;
	}
	int getSonar() {
		return numSonar;
	}

};

class Protesters : public Characters {
private:
	bool outOfField = false;
	int annoyed_sound = SOUND_PROTESTER_GIVE_UP;
	int yell_sound = SOUND_PROTESTER_YELL;
protected:
	int numSquareToMoveCurrentDir;
	int ticksWaitBetweenMoves;
public:
	Protesters(StudentWorld* world, int imgID = IID_PROTESTER, int startX = 60, int startY = 60, int hp = 5, int t_str = 2, double col_range = 4, double detect_range = 0, int t_sound = SOUND_PROTESTER_ANNOYED, int t_score = 0);
	virtual ~Protesters() override {};

	//Functions
	void rest() {
		movementBehavior.reset();
	}
	void chase() {
	}
	void shout(Direction dir);
	bool annoyed() {
		outOfField = true;
		return outOfField;
	}
	void bribed() {
		//Pursuing toward the exit
	}
	void doSomething() override;
};

class HardcoreProtesters : public Protesters {
private:

public:
	HardcoreProtesters(StudentWorld* world, int imgID = IID_HARD_CORE_PROTESTER, int startX = 60, int startY = 60, int hp = 20, int strength = 2, double col_range = 4, double detect_range = 0, int t_sound = SOUND_PROTESTER_ANNOYED, int t_score = 0) : Protesters(world, IID_HARD_CORE_PROTESTER, startX, startY, hp, strength, col_range, detect_range, t_sound, t_score) {
		movementBehavior = std::make_unique<FreeMovement>();
	}
	~HardcoreProtesters() override {};
};

//Stuffs
class Inanimated : public Actor {
private:

public:
	Inanimated(StudentWorld* world, ActorType t_type, bool visibility, int imgID, int startX, int startY, Direction dir = right, double size = 1.0, unsigned int depth = 2, int hp = 1, int strength = 0, double col_range = 4, double detect_range = 0, int t_sound = SOUND_NONE, int t_score = 0) : Actor(world, t_type, visibility, imgID, startX, startY, dir, size, depth, hp, strength, col_range, detect_range, t_sound, t_score) {};

	virtual ~Inanimated() {};
};

class Collectable : public Inanimated {
private:
	bool isHidden;
protected:
	std::unique_ptr<IExistenceBehavior>existBehavior;
	bool pickableByPlayer;
	bool isPickable() {
		return pickableByPlayer;
	}
	void setToPickalbe(bool flag) {
		pickableByPlayer = flag;
	}
public:
	Collectable(StudentWorld* world, bool visibility, int imgID, int startX, int startY, Direction dir, double size, unsigned int depth, int hp, int strength, double col_range, double detect_range, int t_sound = SOUND_GOT_GOODIE, int t_score = 0, bool pickable = true) : Inanimated(world, collect, visibility, imgID, startX, startY, dir, size, depth, hp, strength, col_range, detect_range, t_sound, t_score), isHidden(!visibility), pickableByPlayer(pickable) {};
	virtual ~Collectable() {};
	void showSelf();
};

class OilBarrels : public Collectable {
private:
	//Functions
	void doSomething() override;
public:
	OilBarrels(StudentWorld* world, int startX, int startY, Direction dir = right, double size = 1.0, unsigned depth = 2.0, int hp = 1, int strength = 0, double col_range = 3, double detect_range = 4, int t_sound = SOUND_FOUND_OIL, int t_score = 1000) : Collectable(world, true, IID_BARREL, startX, startY, dir, size, depth, hp, strength, col_range, detect_range, t_sound, t_score) {
	};
};

class GoldNuggets : public Collectable {
private:
	//Function
	//Determine if the time for the Temporary gold exist ran out
	IceMan * m_IM;
public:
	GoldNuggets(StudentWorld* world, int startX, int startY, Direction dir = right, double size = 1.0, unsigned depth = 2.0, int hp = 1, int strength = 0, double col_range = 3, double detect_range = 4, bool t_pickable = true, int t_sound = SOUND_GOT_GOODIE, int t_score = 25) : Collectable(world, false, IID_GOLD, startX, startY, dir, size, depth, hp, strength, col_range, detect_range, t_sound, t_score) {};

	void drop();

	void doSomething() override;

	IceMan* getIceMan() {
		return m_IM;
	}

	void setPickable(bool temp) {
		pickableByPlayer = temp;
	}


};

class SonarKit : public Collectable {
private:
	int getAmmo();
	int increaseAmmo(int amount);
	void useSonar();
public:
	SonarKit(StudentWorld* world, int startX = 0, int startY = 60, Direction dir = right, double size = 1.0, unsigned depth = 2.0, int hp = 1, int strength = 0, double col_range = 3, double detect_range = 3, int t_sound = SOUND_GOT_GOODIE, int t_score = 75) : Collectable(world, true, IID_SONAR, startX, startY, dir, size, depth, hp, strength, col_range, detect_range, t_sound, t_score) {
		/*existBehavior = std::make_unique<ExistTemporary>();*/
	};

	void doSomething() override;
};

class Water : public Collectable {
private:
public:
	Water(StudentWorld* world, int startX, int startY, Direction dir = right, double size = 1.0, unsigned depth = 2.0, int hp = 1, int strength = 0, double col_range = 3, double detect_range = 3, int t_sound = SOUND_GOT_GOODIE, int t_score = 100) : Collectable(world, true, IID_WATER_POOL, startX, startY, dir, size, depth, hp, strength, col_range, detect_range, t_sound, t_score) {
		/*existBehavior = std::make_unique<ExistTemporary>();*/
	}

	void doSomething() override;
};

//Hazard is anything that moves into "your" position
//So make a function to determine if a world static is going to "overlap" any characters, then it will turn into "hazard" type
class Hazard : public Inanimated {
private:

public:
	Hazard(StudentWorld* world, ActorType t_type, bool visibility, int imgID, int startX, int startY, Direction dir, double size = 1.0, unsigned depth = 1.0, int hp = 1, int strength = 0, double col_range = 0, double detect_range = 9999, int t_sound = -1) : Inanimated(world, t_type, visibility, imgID, startX, startY, dir, 1.0, 1, hp, strength, col_range, detect_range, t_sound) {

	};
	virtual ~Hazard() {};
};

class Squirt : public Hazard {
private:
	//Functions
	void increaseAmmo(int amount);
	void shoot();
	void doSomething() override;
public:
	Squirt(StudentWorld* world, int startX, int startY, Direction dir, double size = 1.0, unsigned depth = 1.0, int hp = 1, int strength = 2, double col_range = 3, double detect_range = 0, int ded_sound = SOUND_PLAYER_SQUIRT) : Hazard(world, hazard, true, IID_WATER_SPURT, startX, startY, dir, size, depth, hp, strength, col_range, detect_range, ded_sound) {};

};

class Boulder : public Hazard {
public:
	enum BoulderState { stable, waiting, falling };
private:

	BoulderState state = stable;
	int fall_sound = SOUND_FALLING_ROCK;

	//Functions
	void doSomething() override;
public:
	Boulder(StudentWorld* world, int startX, int startY, Direction dir = down, double size = 1.0, unsigned depth = 1.0, int hp = 1, int strength = 9999, double col_range = 3, double detect_range = 0) : Hazard(world, worldStatic, true, IID_BOULDER, startX, startY, dir, size, depth, hp, strength, col_range, detect_range) {
		//Not hazard yet when first spawn
	}
};

class Ice : public Inanimated {
private:

public:
	Ice(StudentWorld* world, bool visibility, int startX, int startY, Direction dir = right, double size = 1.0, unsigned depth = 3.0, int hp = 1, int strength = 9999, double col_range = 0, double detect_range = 0, int t_sound = SOUND_DIG) : Inanimated(world, ActorType::ice, visibility, IID_ICE, startX, startY, dir, 0.25, 3, hp, strength, col_range, detect_range, t_sound) {};
	void doSomething() override;
};

#endif // ACTOR_H_

