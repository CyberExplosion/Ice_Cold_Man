#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include <string>
#include <vector>
#include <array>
#include <future>
#include "Actor.h"

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

//You may add as many other public or private methods and private member variables to
//your StudentWorld class as you like

class Actor;
class IceMan;
class Ice;

const int COL_NUM = 64,
ROW_NUM = 60;
const int DIST_ALLOW_BETW_SPAWN = 9;	//3 is the typical "collision size of most object" plus 6 square away each other
const int OBJECT_LENGTH = 4;
const int ICE_LENGTH = 1;
const int SONAR_CHANCE = 20,
WATER_CHANCE = 80;			//both percentages
const int shaftXoffsetL = 30,
shaftXoffsetR = 33,
shaftYoffsetD = 4,	//All inclusive
shaftYoffsetU = 60;

class StudentWorld : public GameWorld
{
public:

	StudentWorld(std::string assetDir)
		: GameWorld(assetDir) {
	};

	//Your init() method is responsible for creating the current levelfs oil field and populating
	//	it with Ice, Boulders, Barrels of Oil, and Gold Nuggets(wefll show you how below),
	//	and constructing a virtual representation of the current level in your StudentWorld class,
	//	using one or more data structures that you come up with.This function must return the
	//	value GWSTATUS_CONTINUE_GAME(defined in GameConstants.h).
	//	The init() method is automatically called by our provided code either(a) when the game
	//	first starts, (b)when the player completes the current level and advances to a new level
	//	(that needs to be initialized), or (c)when the player loses a life(but has more lives left)
	//	and the game is ready to restart at the current level.
	virtual int init()
	{
		populateIce();
		createPlayer();
		mainCreateObjects();
		initSpawnParameters();

		return GWSTATUS_CONTINUE_GAME;
	}


	// Each time your move() method is called, it must run a single tick
	//of the game.This means that it is responsible for asking each of the games actors
	//to try to do
	//	something: e.g., move themselves and/or perform their specified behavior. Finally, this
	//	method is responsible for disposing of(i.e., deleting) actors(e.g., a Squirt from the
	//		Icemanfs squirt gun that has run its course, a Regular Protester who has left the oil field,
	//		a Boulder that has fallen and crashed into Ice below, etc.) that need to disappear during a given tick
	virtual int move();
		//For example, if a Boulder has completed its fall and disintegrated in the Ice
		//	below, then its state should be set to dead, hand the after all of the actors in the game
		//	get a chance to do something during the tick, the move() method should remove that
		//	Boulder from the game world(by deleting its object and removing any reference to the
		//		object from the StudentWorldfs data structures)


		// This code is here merely to allow the game to build, run, and terminate after you hit enter a few times.
		// Notice that the return value GWSTATUS_PLAYER_DIED will cause our framework to end the current level.

		//decLives();
		//return GWSTATUS_PLAYER_DIED;


	//The cleanup() method is responsible for
	//	freeing all actors that are currently active in the game. This includes all
	//	actors created during either the init() method or introduced during subsequent game ticks
	//	that have not yet been removed from the game.
	virtual void cleanUp();

	std::vector<std::shared_ptr<Actor>> StudentWorld::getAllActors();

	std::shared_ptr<IceMan> getPlayer() {
		return player;
	}

	std::array<std::array<std::shared_ptr<Ice>, COL_NUM>, ROW_NUM> getIceArr() {
		return ice_array;
	}
	bool boulderFall(int x, int y);
	// Functions for interfaces
	std::vector<std::weak_ptr<Actor>> iceCollideWithActor(std::shared_ptr<Actor> actor, bool radarMode = false);
	std::vector<std::weak_ptr<Actor>> actorsCollideWithMe(std::shared_ptr<Actor> actor, bool radarMode = false);
	bool createSquirt();
	bool createGoodies(std::pair<int, int>);
	void decrementOil() {
		--oilsLeft;
	}
	void increaseEmptyIce();	//Call this function at the end to populate the location where water can spawn
	std::pair<int, int> findEmptyIce();	//Don't know why but my guess it's that async doesn't allow member function
	template<typename T>
	bool createObjects(int x, int y);
private:
	// Data Structures
	std::array<std::array<std::shared_ptr<Ice>, COL_NUM>, ROW_NUM> ice_array; // 2D array holding ice on screen. One holding columns, one holding rows.
	std::vector<std::pair<int, int>>empty_iceLocal;
	std::vector<std::shared_ptr<Actor>>actor_vec; // Holds all actor objects (ie. boulders, gold, protesters)
	std::shared_ptr<IceMan> player;

	// Functions for move()
	int updateStatus(); // Updates the status at the top of the screen. (Health, lives, gold, etc.)
	int doThings(); // Asks the player and actor objects to doSomething() each tick.
	void deleteFinishedObjects(); // Checks to see if an object has finished its task. (Eg. if a boulder has fallen, delete it from game.)
	
	// Functions for init()
	void populateIce();
	void createPlayer();
	void mainCreateObjects();
	void createProtesters();
	void initSpawnParameters();
	
	// Private Variables
	int ticksBeforeSpawn; // # of ticks before a protester can spawn on the field.
	int protesterSpawnLimit; // # of protestors allowed per level.
	int oilsLeft;
	int protesterCount;


	//Function for move
	void createNPC();

};

template<typename T>
bool StudentWorld::createObjects(int x, int y) {
	/************************************
	Check if the location is well spread out enough compare to other actors or not
	If the location is valid, clear the ice in that location
	Put the object into the actor vector
	************************************/
	//Test
	//cerr << "x :" << x << " || " << "y: " << y << "\n";
 	
	for (auto const& each : actor_vec) {
		int distance = sqrt(pow(each->getX() - x, 2) + pow(each->getY() - y, 2));
		if (distance < DIST_ALLOW_BETW_SPAWN)
			return false;
	}

	/*TODO: ICE IS NOT RECOGNIZED AS AN INTRUDER, FIX THIS SHIT*/
	/*THIS MAY HAPPEN BECAUSE THE ICE IS DEAD BUT SINCE WE NOT MOVE YET CLEAN UP HASN'T BEEN CALLED*/
	////////// This is fixed, I THINK????!!!

	//If reach this meaning that's there is an intruder and it's ice
	std::shared_ptr<T> object = std::make_shared<T>(this, x, y);	//Make the object
	if (object->type == Actor::ActorType::worldStatic) {	//Only destroy the ice if it's worldStatic
		object->collisionDetection = std::make_unique<CollisionDetection>(object);
		object->collisionDetection->behaveBitches();	//See if the object collide with any ice


		auto intruders = std::move(object->collisionDetection->wp_intruders);	//Get the ice

		for (auto& wp_entity : intruders) {
			//since i ran out of patient, imma going to do what i called a pro gamer moves
			shared_ptr<Actor>entity = wp_entity.lock();
			if (entity) {
				entity->dmgActor(9999);
				entity.reset();
			}
		}
	}
	if (object->collisionResult)	//If there's is a result for the said Collision
		object->collisionResult->response();	//Demand a response from the source
	actor_vec.push_back(object);	//Create a proper boulder at the location picked
	return true;
}


#endif // STUDENTWORLD_H_