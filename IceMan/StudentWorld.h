#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include <string>
#include <vector>
#include <array>
#include "Actor.h"

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

//You may add as many other public or private methods and private member variables to
//your StudentWorld class as you like

class Actor;
class IceMan;
class Ice;
const int COL_NUM = 60,
ROW_NUM = 60;
class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir)
		: GameWorld(assetDir) {
	};

	//Your init() method is responsible for creating the current levelÅfs oil field and populating
	//	it with Ice, Boulders, Barrels of Oil, and Gold Nuggets(weÅfll show you how below),
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
		//mainCreateObjects();
		return GWSTATUS_CONTINUE_GAME;
	}


	// Each time your move() method is called, it must run a single tick
	//of the game.This means that it is responsible for asking each of the gameÅs actors
	//to try to do
	//	something: e.g., move themselves and/or perform their specified behavior. Finally, this
	//	method is responsible for disposing of(i.e., deleting) actors(e.g., a Squirt from the
	//		IcemanÅfs squirt gun that has run its course, a Regular Protester who has left the oil field,
	//		a Boulder that has fallen and crashed into Ice below, etc.) that need to disappear during a given tick
	virtual int move();
		//For example, if a Boulder has completed its fall and disintegrated in the Ice
		//	below, then its state should be set to Ådead, Åhand the after all of the actors in the game
		//	get a chance to do something during the tick, the move() method should remove that
		//	Boulder from the game world(by deleting its object and removing any reference to the
		//		object from the StudentWorldÅfs data structures)


		// This code is here merely to allow the game to build, run, and terminate after you hit enter a few times.
		// Notice that the return value GWSTATUS_PLAYER_DIED will cause our framework to end the current level.

		//decLives();
		//return GWSTATUS_PLAYER_DIED;


	//The cleanup() method is responsible for
	//	freeing all actors that are currently active in the game. This includes all
	//	actors created during either the init() method or introduced during subsequent game ticks
	//	that have not yet been removed from the game.
	virtual void cleanUp()
	{
	}

	std::vector<std::shared_ptr<Actor>> StudentWorld::getAllActors();
	std::shared_ptr<IceMan> getPlayer() {
		return player;
	}
	std::array<std::array<std::shared_ptr<Ice>, COL_NUM>, ROW_NUM> getIceArr() {
		return ice_array;
	}
private:
	std::array<std::array<std::shared_ptr<Ice>, COL_NUM>, ROW_NUM> ice_array; // 2D array holding ice on screen. One holding columns, one holding rows.
	std::vector<std::shared_ptr<Actor>>actor_vec; // Holds all actor objects (ie. boulders, gold, protesters)
	std::shared_ptr<IceMan> player;
	int oilsLeft;
	int updateStatus(); // Updates the status at the top of the screen. (Health, lives, gold, etc.)
	int doThings(); // Asks the player and actor objects to doSomething() each tick.
	void deleteFinishedObjects(); // Checks to see if an object has finished its task. (Eg. if a boulder has fallen, delete it from game.)

	//Functions for init
	void populateIce();
	void createPlayer();
	void mainCreateObjects();
	template<typename T>
	bool createObjects(int x, int y);
};

template<typename T>
bool StudentWorld::createObjects(int x, int y) {
	/************************************
	Create a dummy object at the specified location with its collision range set up to 6. It will tell us if we can put the object in the location or not
	If there's a collision with any actors except ice, the location is compromised and thus should return false to get another location
	The location is not compromise, move to next phase:
		Create a real object with real collision and every attributes at the location
		Make a collision detection with the object to find intruder (ice)
		If there's an intruder
			Then make the intruder to check its' own collision detection
		If there's a collision happen in the source and the intruder
			Demand a collision response from the source
			Demand a collision response from the intruder
	Finally put the newly made object into actor containers
	************************************/
	std::shared_ptr<T> temp = std::make_shared<T>(this, x, y, GraphObject::Direction::right, 1.0, 2, 1, 9999, 6);	//Collision range of 6 because that's the requirement for a new object to be made

 	temp->collisionDetection = std::make_unique<CollisionDetection>(temp, temp->getCollisionRange());
	temp->collisionDetection->behaveBitches();	//Check for collision
	std::vector<std::weak_ptr<Actor>>intruders = std::move(temp->collisionDetection->getIntruders());

		for (auto const& sp_entity : intruders) {
			std::shared_ptr<Actor>entity = sp_entity.lock();
			if (entity && entity->type != Actor::ActorType::ice)	//There's an intruder and it's not ice
				return false;
		}

	/*TODO: ICE IS NOT RECOGNIZED AS AN INTRUDER, FIX THIS SHIT*/
	/*THIS MAY HAPPEN BECAUSE THE ICE IS DEAD BUT SINCE WE NOT MOVE YET CLEAN UP HASN'T BEEN CALLED*/
	////////// This is fixed, I THINK????!!!

	intruders.clear();
	//If reach this meaning that's there is an intruder and it's ice
	std::shared_ptr<T> object = std::make_shared <T>(this, x, y);	//Make the object
	if (object->isVisible()) {	//Only destroy the ice if it's visible
		object->collisionDetection = std::make_unique<CollisionDetection>(object, object->getCollisionRange());
		object->collisionDetection->behaveBitches();	//See if the object collide with any ice

		intruders = std::move(object->collisionDetection->getIntruders());	//Get the ice
		for (auto& wp_entity : intruders) {
			//since i ran out of patient, imma going to do what i called a pro gamer moves
			shared_ptr<Actor>entity = wp_entity.lock();

			if (entity) {
				//entity->collisionDetection = std::make_unique<CollisionDetection>(entity, entity->getCollisionRange());
				//entity->collisionDetection->behaveBitches();	//Get the intruder to check its' own collision detection
				//	
				//if (entity->collisionResult)
				//		entity->collisionResult->response();	//Demand a response from the intruder
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