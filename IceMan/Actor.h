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
	double detectionRange;
	StudentWorld* m_sw;
	int sound;
	//double centerX = 0;
	//double centerY = 0;
	int size;
public:
	Actor(StudentWorld* world, ActorType t_type, bool visibility, int imgID, int startX, int startY, Direction dir = right, double t_size = 1.0, unsigned int depth = 0, int t_hp = 1, int t_strength = 0, double col_range = 0, double detect_range = 0, int t_sound = SOUND_NONE) : GraphObject(imgID, startX, startY, dir, t_size, depth), hitpoints(t_hp), strength(t_strength), collisionRange(col_range), detectionRange(detect_range), type(t_type), m_sw(world), sound(t_sound), size(t_size) {
		setVisible(visibility);
	};
	virtual ~Actor() {};

	std::unique_ptr<IExistenceBehavior> displayBehavior;
	std::unique_ptr<IMovementBehavior> movementBehavior;
	//Store the result in this
	std::unique_ptr<IActorResponse> collisionResult;
	//Determine what method of detection the actor would use
	std::unique_ptr<IDetectionBehavior>detectBehavior;
	std::unique_ptr<IDetectionBehavior>collisionDetection;

	//This is for all the usage of shared_ptr in them behaviors. This is bad but I blame due date
	virtual void resetAllBehaviors();

	//int getCenterX() {
	//	centerX = (size * OBJECT_LENGTH) / 2 + getX() - 1;	//Example: The graph takes 4 slot: 52 53 54 55 -> The center will be 53
	//	return centerX;
	//}

	//int getCenterY() {
	//	centerY = (size * OBJECT_LENGTH) / 2 + getY() - 1;
	//	return centerY;
	//}

	int getSound() {
		return sound;
	}

	void setSound(int t_sound) {
		sound = t_sound;
	}

	StudentWorld* getWorld() {
		return m_sw;
	}

	bool isAlive() {
		return hitpoints > 0;
	}

	//Don't use it before derived
	virtual void doSomething() = 0;

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

	void dmgActor(int amt) {
		hitpoints -= amt;
	}

	void changeActorType(ActorType t_type) {
		type = t_type;
	}
};

//Movement Strategy
class IMovementBehavior {
public:
	virtual void moveThatAss() = 0;
	virtual void resetBehavior() = 0;
};

class FreeMovement : public IMovementBehavior{
private:
	
public:
	//This is the default movement for NPC
	void moveThatAss() override;
	void resetBehavior() override;
};

class FallMovement : public IMovementBehavior{
private:

public:
	//This is for the Boulder
	void moveThatAss() override;
	void resetBehavior() override;
};

class ControlledMovement : public IMovementBehavior{
private:
	int key = INVALID_KEY;
	std::weak_ptr<Actor> pawn;
public:
	void resetBehavior() override;
	ControlledMovement(std::weak_ptr<Actor> t_pawn) : pawn(t_pawn) {}
	//This is for the player
	void moveThatAss() override;
};

class PursuingMovement : public IMovementBehavior{
private:

public:
	//This is for whenever NPC chasing after a location
	void moveThatAss() override;
	void resetBehavior() override;
};

class SquirtMovement : public IMovementBehavior {
private:

public:
	//This is for the Squirt to travel
	void moveThatAss() override;
	void resetBehavior() override;
};

//////////////////////////

//Collision Strategy
class IActorResponse{
public:
	virtual void response() = 0;
	virtual void resetBehavior() = 0;
};

class Block : public IActorResponse {
private:
	std::weak_ptr<Actor>wp_target;
	int targetFacing;
public:
	void resetBehavior() override;
	Block(std::weak_ptr<Actor> t_target) : wp_target(t_target){
		std::shared_ptr<Actor>temp = wp_target.lock();
		if (temp)
			targetFacing = temp->getDirection();
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
	Destroy(std::weak_ptr<Actor> t_target, int dmgTook) : wp_target(t_target), dmgTaken(dmgTook) {};
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
	virtual void resetBehavior() = 0;
};

class ExistTemporary : public IExistenceBehavior{
private:
	int timeTillDeath;
public:
	void showYourself() override;
	void resetBehavior() override;
};

class ExistPermanently : public IExistenceBehavior{
private:
	int lifeIsAPain;
public:
	void showYourself() override;
	void resetBehavior() override;
};

/////////////////////////
//Detection Strategy
//Collision detection and radar detection are the same type, just have different range
class IDetectionBehavior{
protected:

public:
	//The source is npc or objects. Intruder will be the player
	std::weak_ptr<Actor> wp_source;
	std::vector<std::weak_ptr<Actor>> wp_intruders;
	IDetectionBehavior(std::weak_ptr<Actor> t_source) : wp_source(t_source) {};
	IDetectionBehavior(std::weak_ptr<Actor> t_source, std::vector<std::weak_ptr<Actor>> t_intruder) : wp_source(t_source), wp_intruders(t_intruder) {};
	
	//All of this is bad and I'm meant it. I wish whoever look at this in the future best of luck
	virtual void behaveBitches() = 0;
	void virtual resetBehavior() {
		wp_source.reset();
		for (auto& val : wp_intruders)
			val.reset();
	}
	//std::weak_ptr<Actor> getSource() {
	//	return wp_source;
	//}
	//std::vector<std::weak_ptr<Actor>> getIntruders() {
	//	return wp_intruders;
	//}
	//void setSource(std::weak_ptr<Actor> t_source) {
	//	wp_source.reset();
	//	wp_source = t_source;
	//}
	//void setIntruder(std::vector<std::weak_ptr<Actor>> t_intruder) {
	//	for (auto& val : wp_intruders)
	//		val.reset();
	//	wp_intruders = std::move(t_intruder);
	//}
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
	int range;
	std::vector<std::weak_ptr<Actor>> sensedIce();
	std::vector<std::weak_ptr<Actor>> sensedOthers();
public:
	RadarLikeDetection(std::weak_ptr<Actor> t_source, int t_range) : IDetectionBehavior(t_source) {
		range = t_range;
		std::shared_ptr<Actor> temp = t_source.lock();
		if (temp) {
			if (temp->type == Actor::ActorType::player || temp->type == Actor::ActorType::worldStatic || temp->type == Actor::ActorType::collect) {	//Only these are allow to interact with the ice AND others
				wp_intruders = std::move(sensedIce());
				auto temp = std::move(sensedOthers());	
				wp_intruders.insert(end(wp_intruders), begin(temp), end(temp));	//Concatenate the vectors cause we have more intruders
			}
			else
				wp_intruders = std::move(sensedOthers());
		}
	};
	int getRange() {
		return range;
	}
	void behaveBitches() override;
};

class CollisionDetection : public RadarLikeDetection {
private:
	//This functions determine the result of collision between 2 actors. The source will have <Block> or <Destroy> behavior
	//depend on what it collides into
	void collide(std::weak_ptr<Actor> source, std::weak_ptr<Actor> receiver);
	bool collisionHappen();
public:
	CollisionDetection(std::weak_ptr<Actor> t_source, int t_range) : RadarLikeDetection(t_source, t_range){}
	void behaveBitches() override;
};

/////////////////////////


//People
class Characters : public Actor {
public:
	Characters(StudentWorld* world, ActorType type, int imgID, int startX, int startY, Direction dir, int t_hp, int t_str, double col_ran, double detect_range, int t_sound) : Actor(world, type, true, imgID, startX, startY, dir, 1.0, 0, t_hp, t_str, col_ran, detect_range, t_sound) {};
	virtual ~Characters() {};
};

class IceMan : public Characters {
private:
	std::vector<std::shared_ptr<SonarKit>>sonarVec;
	std::vector<std::shared_ptr<GoldNuggets>>goldVec;
	std::vector<std::shared_ptr<Squirt>>squirtVec;
	//This is bad and can cause problem later on because it has some kind of circular dependent. Too bad
	//This function find the player actor type in the whole list of actors
public:
	IceMan(StudentWorld* world, int startX = 30, int startY = 60) : Characters(world, player, IID_PLAYER, startX, startY, right, 10, 999, 3, 0, SOUND_PLAYER_ANNOYED) {};
	void doSomething() override;
	int getSonarNum() {
		return sonarVec.size();
	}
	int getSquirtNum() {
		return squirtVec.size();
	}
	int getGoldNum() {
		return goldVec.size();
	}
};

class Protesters : public Characters{
private:
	bool outOfField = false;
public:
	Protesters(StudentWorld* world, int imgID = IID_PROTESTER, int startX = 60, int startY = 60, int hp = 5, int t_str = 2, double col_range = 4, double detect_range = 0, int t_sound = SOUND_PROTESTER_ANNOYED) : Characters(world, npc, imgID, startX, startY, left, hp, t_str, col_range, detect_range, t_sound){
		movementBehavior = std::make_unique<FreeMovement>();
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
	}
	void doSomething() override;
};

class HardcoreProtesters: public Protesters{
private:

public:
	HardcoreProtesters(StudentWorld* world, int imgID = IID_HARD_CORE_PROTESTER, int startX = 60, int startY = 60, int hp = 20, int strength = 2, double col_range = 4, double detect_range = 0, int t_sound = SOUND_PROTESTER_ANNOYED) : Protesters(world, IID_HARD_CORE_PROTESTER, startX, startY, hp, strength, col_range, detect_range, t_sound) {
		movementBehavior = std::make_unique<FreeMovement>();
	}
};

//Stuffs
class Inanimated : public Actor {
private:

public:
	Inanimated(StudentWorld* world, ActorType t_type, bool visibility, int imgID, int startX, int startY, Direction dir = right, double size = 1.0, unsigned int depth = 2, int hp = 1, int strength = 0, double col_range = 4, double detect_range = 0, int t_sound = SOUND_NONE) : Actor(world, t_type, visibility, imgID, startX, startY, dir, size, depth, hp, strength, col_range, detect_range, t_sound) {};

	virtual ~Inanimated() {};
};

class Collectable : public Inanimated{
private:
	bool isHidden;
protected:
	std::unique_ptr<IExistenceBehavior> existBehavior;
public:
	Collectable(StudentWorld* world, bool visibility, int imgID, int startX, int startY, Direction dir, double size, unsigned int depth, int hp, int strength, double col_range, double detect_range) : Inanimated(world, collect, visibility, imgID, startX, startY, dir, size, depth, hp, strength, col_range, detect_range), isHidden(!visibility) {};
	virtual ~Collectable() {};
	void showSelf();
};

class OilBarrels : public Collectable{
private:

	//Functions
	void doSomething() override;
public:
	OilBarrels(StudentWorld* world, int startX, int startY, Direction dir = right, double size = 1.0, unsigned depth = 2.0, int hp = 1, int strength = 0, double col_range = 3, double detect_range = 4) : Collectable(world, true, IID_BARREL, startX, startY, dir, size, depth, hp, strength, col_range, detect_range) {
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
	GoldNuggets(StudentWorld* world, int startX, int startY, Direction dir = right, double size = 1.0, unsigned depth = 2.0, int hp = 1, int strength = 0, double col_range = 3, double detect_range = 4, bool t_pickable = true) : Collectable(world, true, IID_GOLD, startX, startY, dir, size, depth, hp, strength, col_range, detect_range), pickableByPlayer(t_pickable) {
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
	SonarKit(StudentWorld* world, int startX = 0, int startY = 60, Direction dir = right, double size = 1.0, unsigned depth = 2.0, int hp = 1, int strength = 0, double col_range = 3, double detect_range = 9999) : Collectable(world, true, IID_SONAR, startX, startY, dir, size, depth, hp, strength, col_range, detect_range) {
		existBehavior = std::make_unique<ExistTemporary>();
	};
	void doSomething() override;
};

class Water : public Collectable{
private:
public:
	Water(StudentWorld* world, int startX, int startY, Direction dir = right, double size = 1.0, unsigned depth = 2.0, int hp = 1, int strength = 0, double col_range = 3, double detect_range = 9999) : Collectable(world, true, IID_WATER_POOL, startX, startY, dir, size, depth, hp, strength, col_range, detect_range) {
		existBehavior = std::make_unique<ExistTemporary>();
	}
	void doSomething() override;
};

//Hazard is anything that moves into "your" position
//So make a function to determine if a world static is going to "overlap" any characters, then it will turn into "hazard" type
class Hazard : public Inanimated{
private:

public:
	Hazard(StudentWorld* world, ActorType t_type, bool visibility, int imgID, int startX, int startY, Direction dir, double size = 1.0, unsigned depth = 1.0, int hp = 1, int strength = 0, double col_range = 0, double detect_range = 9999) : Inanimated(world, t_type, visibility, imgID, startX, startY, dir, 1.0, 1, hp, strength, col_range, detect_range) {};
	virtual ~Hazard() {};
};

class Squirt : public Hazard{
private:
	//Functions
	void increaseAmmo(int amount);
	void shoot();
	void doSomething() override;
public:
	Squirt(StudentWorld* world, int startX, int startY, Direction dir, double size = 1.0, unsigned depth = 1.0, int hp = 1, int strength = 2, double col_range = 4, double detect_range = 9999) : Hazard(world, hazard, true, IID_WATER_SPURT, startX, startY, dir, size, depth, hp, strength, col_range, detect_range) {};

};

class Boulder : public Hazard {
public:
	enum BoudlderState { stable, waiting, falling };
private:
	BoudlderState state = stable;
	//Functions
	void fall();
	bool checkIceBelow();
	void doSomething() override;
public:
	Boulder(StudentWorld* world, int startX, int startY, Direction dir = down, double size = 1.0, unsigned depth = 1.0, int hp = 1, int strength = 9999, double col_range = 3, double detect_range = 9999) : Hazard(world, worldStatic, true, IID_BOULDER, startX, startY, dir, size, depth, hp, strength, col_range, detect_range){
		//Not hazard yet when first spawn
		changeActorType(ActorType::worldStatic);
		movementBehavior = std::make_unique<FallMovement>();
	}
};

class Ice : public Inanimated {
private:
	
public:
	Ice(StudentWorld* world, bool visibility, int startX, int startY, Direction dir = right, double size = 1.0, unsigned depth = 3.0, int hp = 1, int strength = 1, double col_range = 0, double detect_range = 9999, int t_sound = SOUND_DIG) : Inanimated(world, ActorType::ice, visibility, IID_ICE, startX, startY, dir, 0.25, 3, hp, strength, col_range, detect_range, t_sound) {};
	void doSomething() override;
};

#endif // ACTOR_H_