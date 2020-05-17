#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include <string>
#include <vector>
#include "Actor.h"
// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

//You may add as many other public or private methods and private member variables to
//your StudentWorld class as you like
class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir)
		: GameWorld(assetDir)
	{
	}

	//Your init() method is responsible for creating the current level�fs oil field and populating
	//	it with Ice, Boulders, Barrels of Oil, and Gold Nuggets(we�fll show you how below),
	//	and constructing a virtual representation of the current level in your StudentWorld class,
	//	using one or more data structures that you come up with.This function must return the
	//	value GWSTATUS_CONTINUE_GAME(defined in GameConstants.h).
	//	The init() method is automatically called by our provided code either(a) when the game
	//	first starts, (b)when the player completes the current level and advances to a new level
	//	(that needs to be initialized), or (c)when the player loses a life(but has more lives left)
	//	and the game is ready to restart at the current level.
	virtual int init()
	{
		return GWSTATUS_CONTINUE_GAME;
	}


	// Each time your move() method is called, it must run a single tick
	//of the game.This means that it is responsible for asking each of the game�s actors
	//to try to do
	//	something: e.g., move themselves and/or perform their specified behavior. Finally, this
	//	method is responsible for disposing of(i.e., deleting) actors(e.g., a Squirt from the
	//		Iceman�fs squirt gun that has run its course, a Regular Protester who has left the oil field,
	//		a Boulder that has fallen and crashed into Ice below, etc.) that need to disappear during a given tick
	virtual int move()
	{
		//For example, if a Boulder has completed its fall and disintegrated in the Ice
		//	below, then its state should be set to �gdead, �hand the after all of the actors in the game
		//	get a chance to do something during the tick, the move() method should remove that
		//	Boulder from the game world(by deleting its object and removing any reference to the
		//		object from the StudentWorld�fs data structures)


		// This code is here merely to allow the game to build, run, and terminate after you hit enter a few times.
		// Notice that the return value GWSTATUS_PLAYER_DIED will cause our framework to end the current level.

		//decLives();
		//return GWSTATUS_PLAYER_DIED;
	}

	//The cleanup() method is responsible for
	//	freeing all actors that are currently active in the game. This includes all
	//	actors created during either the init() method or introduced during subsequent game ticks
	//	that have not yet been removed from the game.
	virtual void cleanUp()
	{
	}

	int updateStatus() {} // Updates the status at the top of the screen. (Health, lives, gold, etc.)

	int doThings() {} // Asks the player and actor objects to doSomething() each tick.

	int deleteFinishedObjects() {} // Checks to see if an object has finished its task. (Eg. if a boulder has fallen, delete it from game.)

private:
	std::vector<Actor*>ice_vec; // Holds all ice on the screen
	std::vector<std::shared_ptr<Actor>>actors_vec; // Holds all actor objects (ie. boulders, gold, protesters)
	Actor* player;
};

#endif // STUDENTWORLD_H_
