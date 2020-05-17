#ifndef ACTOR_H_
#define ACTOR_H_

#include <iostream>
#include "GraphObject.h"

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

//Movement Strategy
class IMovementBehavior{
public:
	virtual void moveThatAss() = 0;
};

class FreeMovement : public IMovementBehavior{
private:
	
public:
	//This is the default movement for NPC
	void moveThatAss() override;
};

class FallMovement : public IMovementBehavior{
private:

public:
	//This is for the Boulder
	void moveThatAss() override;
};

class ControlledMovement : public IMovementBehavior{
private:

public:
	//This is for the player
	void moveThatAss() override;
};

class PursuingMovement : public IMovementBehavior{
private:

public:
	//This is for whenever NPC chasing after a location
	void moveThatAss() override;
};

class SquirtMovement :public IMovementBehavior {
private:

public:
	//This is for the Squirt to travel
	void moveThatAss() override;
};

//////////////////////////

//Collision Strategy
class IActorResponse{
public:
	virtual void response() = 0;
};

class Block : public IActorResponse {
private:
	shared_ptr<Actor>target;
public:
	Block(shared_ptr<Actor>& t_target) : target(t_target) {}
	//Object will be force to stand still
	void response() override;
};

class Destroy : public IActorResponse {
private:
	shared_ptr<Actor>target;
	int dmgTaken;
public:
	Destroy(shared_ptr<Actor>& t_target, int dmgTook) : target(t_target), dmgTaken(dmgTook) {};
	//Object will be force to reduce their health by an amount
	void response() override;
};

/////////////////////////

/////////////////////////
//Existence Strategy
class IExistenceBehavior{
private:
public:
	virtual void showYourself() = 0;
};

class ExistTemporary : public IExistenceBehavior{
private:
	int timeTillDeath;
public:
	void killMeNow();
};

class ExistPermanently : public IExistenceBehavior{
private:
	int lifeIsAPain;
public:
};

/////////////////////////
//Detection Strategy
class IDetectionBehavior{
protected:
	bool radarEnable;
	int radarRange;
	bool inLineOfSight;
public:
	//Function see if the actor should have the radar detect like feature on or off
	IDetectionBehavior(bool radarEnable = true);
	virtual void behaveBitches() = 0;
};

class DetectPlayerBehavior : public IDetectionBehavior{
private:

public:
	DetectPlayerBehavior(bool radar) : IDetectionBehavior(radar) {};
	//What the character do when detect the player
	virtual void behaveBitches() override;
};

class DetectInanimateBehavior : public IDetectionBehavior{
private:

public:
	DetectInanimateBehavior() : IDetectionBehavior(true) {};
	//What the character do when detect an inanimate object
	virtual void behaveBitches() override;
};

/////////////////////////

class Actor : public GraphObject{
private:
	int hitpoints;
	int strength;

protected:
	//Type
	enum ActorType { player, npc, worldStatic, hazard, ice, dropByPlayer, collect };
	ActorType type;

	std::unique_ptr<IMovementBehavior> movementBehavior;
	//Calculate the collision result base on the type that collided into
	std::unique_ptr<IActorResponse> collide(shared_ptr<Actor>& receiver);
	//Store the result in this
	std::unique_ptr<IActorResponse> collisionResult;

public:
	Actor(ActorType type, bool visibility, int imgID, int startX, int startY, Direction dir = right, double size = 1.0, unsigned int depth = 0, int t_hp = 1, int t_strength = 0) : GraphObject(imgID, startX, startY, dir, size, depth), hitpoints(t_hp), strength(t_strength){
		setVisible(visibility);
	};
	virtual ~Actor() {};

	bool isAlive() {
		return hitpoints > 0;
	}

	//Don't use it before derived
	virtual void doSomething() = 0;

	int getStrength() {
		return strength;
	}

	void dmgActor(int amt) {
		hitpoints -= amt;
	}

	void changeActorType(ActorType t_type) {
		type = t_type;
	}
};

//People
class Characters : public Actor {
protected:
	std::unique_ptr<IDetectionBehavior>detectBehavior;
public:
	Characters(ActorType type, int imgID, int startX, int startY, Direction dir, int t_hp, int t_str = 0) : Actor(type, true, imgID, startX, startY, dir, 1.0, 0, t_hp, t_str) {};
	virtual ~Characters() {};
};

class IceMan : public Characters{
private:
	
public:
	IceMan(int startX = 30, int startY = 60) : Characters(player, IID_PLAYER, startX, startY, right, 10, 1) {
		movementBehavior = std::make_unique<ControlledMovement>();
		detectBehavior = std::make_unique<DetectInanimateBehavior>(true);
	};
	void doSomething() override;
};

class Protesters : public Characters{
private:
	bool outOfField = false;
public:
	Protesters(int imgID = IID_PROTESTER, int startX = 60, int startY = 60, int hp = 5, int t_str = 2) : Characters(npc, imgID, startX, startY, left, hp, t_str){
		movementBehavior = std::make_unique<FreeMovement>();
		detectBehavior = std::make_unique<DetectPlayerBehavior>(false);
	}
	//Functions
	void rest() {
		movementBehavior.reset();
	}
	void chase() {
		movementBehavior = std::make_unique<PursuingMovement>();
	}
	void shout(Direction dir);
	bool annoyed() {
		if (isAlive())
			outOfField = true;
		return outOfField;
	}
	void bribed() {
		//Pursuing toward the exit
		movementBehavior = std::make_unique<PursuingMovement>();
		detectBehavior.reset();
	}
	void doSomething() override;
};

class HardcoreProtesters: public Protesters{
private:

public:
	HardcoreProtesters(int startX = 60, int startY = 60, int hp = 20, int strength = 2) : Protesters(IID_HARD_CORE_PROTESTER, startX, startY, hp, strength) {
		movementBehavior = std::make_unique<FreeMovement>();
		detectBehavior = std::make_unique<DetectPlayerBehavior>(true);
	}
};

//Stuffs
class Inanimated : public Actor {
private:

public:
	Inanimated(ActorType t_type, bool visibility, int imgID, int startX, int startY, Direction dir = right, double size = 1.0, unsigned int depth = 2, int hp = 1, int strength = 0) : Actor(t_type, visibility, imgID, startX, startY, dir, size, depth, hp, strength) {};

	virtual ~Inanimated() {};
};

class Collectable : public Inanimated{
private:
	bool isHidden;
protected:
	std::unique_ptr<IExistenceBehavior> existBehavior;
public:
	Collectable(bool visibility, int imgID, int startX, int startY) : Inanimated(collect, visibility, imgID, startX, startY), isHidden(!visibility) {};
	virtual ~Collectable() {};
	void showSelf();
};

class OilBarrels : public Collectable{
private:

	//Functions
	void doSomething() override;
public:
	OilBarrels(int startX, int startY) : Collectable(false, IID_BARREL, startX, startY) {
		existBehavior = std::make_unique<ExistPermanently>();
	};

};

class GoldNuggets : public Collectable{
private:
	bool pickableByPlayer = true;
	//Function
	//Determine if the time for the Temporary gold exist ran out
	bool tempTimeEnd();
public:
	GoldNuggets(int startX, int startY, bool t_pickable = true) : Collectable(false, IID_GOLD, startX, startY), pickableByPlayer(t_pickable) {
		if(pickableByPlayer)
			existBehavior = std::make_unique<ExistPermanently>();
		else {
			existBehavior = std::make_unique<ExistTemporary>();
			changeActorType(dropByPlayer);
		}
	};
	void drop();
	bool getStage() {
		return pickableByPlayer;
	}
	void doSomething() override;
};

class SonarKit : public Collectable{
private:
	int getAmmo();
	int increaseAmmo(int amount);
	void useSonar();
public:
	SonarKit(int startX = 0, int startY = 60) : Collectable(true, IID_SONAR, startX, startY) {
		existBehavior = std::make_unique<ExistTemporary>();
	};
	void doSomething() override;
};

class Water : public Collectable{
private:
public:
	Water(int startX, int startY) : Collectable(true, IID_WATER_POOL, startX, startY) {
		existBehavior = std::make_unique<ExistTemporary>();
	}
	void doSomething() override;
};

//Hazard is anything that moves into "your" position
//So make a function to determine if a world static is going to "overlap" any characters, then it will turn into "hazard" type
class Hazard : public Inanimated{
private:

public:
	Hazard(bool visibility, int imgID, int startX, int startY, Direction dir, int dmgDeal = 0) : Inanimated(hazard, visibility, imgID, startX, startY, dir, 1.0, 1) {};
	virtual ~Hazard() {};
};

class Squirt : public Hazard{
private:
	int ammo;
	//Functions
	int getAmmo();
	void increaseAmmo(int amount);
	void shoot();
	void doSomething() override;
public:
	Squirt(int startX, int startY, Direction dir, int strength = 2) : Hazard(true, IID_WATER_SPURT, startX, startY, dir, strength) {};

};

class Boulder : public Hazard{
public:
	enum BoudlderState { stable, waiting, falling };
private:
	BoudlderState state = stable;
	//Functions
	void falling();
	bool checkIceBelow();
	void doSomething() override;
public:
	Boulder(int startX, int startY, Direction dir = down, int strength = 9999) : Hazard(true, IID_BOULDER, startX, startY, dir, strength){
		movementBehavior = std::make_unique<FallMovement>(); 
	}
};

class Ice : public Inanimated{
private:
	
public:
	Ice(bool visibility, int startX, int startY, Direction dir = right, int hp = 1) : Inanimated(ice, visibility, IID_ICE, startX, startY, dir, 0.25, 3, hp) {};
	void doSomething() override;
};

#endif // ACTOR_H_
