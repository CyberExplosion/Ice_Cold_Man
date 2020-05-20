#include "StudentWorld.h"
#include <algorithm>
#include <string>
#include <time.h>
#include <stdlib.h>
#include <type_traits>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp

int StudentWorld::move() {
	updateStatus();
	int status_of_game = doThings();
	deleteFinishedObjects();

	switch (status_of_game) 
	{
	case 0:
		return GWSTATUS_PLAYER_DIED;
	case 1:
		return GWSTATUS_CONTINUE_GAME;
	case 2:
		return GWSTATUS_PLAYER_WON;
	case 3:
		//playFinishedLevelSound();
		return GWSTATUS_FINISHED_LEVEL;
	case 4:
		return GWSTATUS_LEVEL_ERROR;
	}
	
	/* ADD STUFF FOR PART 2*/
	/*
		Add new actors during each tick.
		Format el game stats @ the TOP TOP TOP!
	
	*/
}

int StudentWorld::updateStatus() {
	string current_status;
	string lvl, lives, health, wtr, gld, oil, sonar, score;

	lvl = to_string(getLevel()); 
	score = to_string(getScore()); 
	lives = to_string(getLives());
	//health = to_string(player->getHealth()); 
	//wtr = to_string(player->getSquirtNum());
	//gld = to_string(player->getGoldNum());
	//oil = to_string(oilsLeft);
	//sonar = to_string(player->getSonarNum());
	
	current_status = 
		  "Lvl: " + lvl 
		+ "Lives: " + lives 
		+ "Hlth: " + health 
		+ "Wtr: " + wtr 
		+ "Gld: " + gld 
		+ "Oil Left: " + oil
		+ "Sonar: " + sonar 
		+ "Scr: " + score;

	setGameStatText(current_status);

	//Return for testing reason, THIS IS NOT CORRECT
	return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::doThings() {
	shared_ptr<Actor>p;
	for (auto& actors : actor_vec) { // Iterates through entire vector of actor objects 
									// and has them call their own doSomething() methods.
		if (actors && actors->isAlive() && actors->type != Actor::ActorType::player) 
			actors->doSomething(); // If it is valid, have the actor do something.
		if (player && player->isAlive())
			player->doSomething(); // If the player is still alive, have them do something.
		else
			return GWSTATUS_PLAYER_DIED; // If the player has died, return the appropriate status.
		if (oilsLeft == 0)
			return GWSTATUS_FINISHED_LEVEL; // Or if the player has found all the barrels of oil, return this status.
	}

	return GWSTATUS_CONTINUE_GAME; // If the player hasn't died and hasn't found all the oils, continue to next tick. 
}

void StudentWorld::deleteFinishedObjects() {
	actor_vec.erase(remove_if(actor_vec.begin(), actor_vec.end(), [](shared_ptr<Actor>& temp) {
		if (temp && !temp->isAlive()) { // If the object is not alive anymore we delete it
			temp.reset();
			return true;
		}
		return false;
		}), end(actor_vec));
	
	for (auto& rowIter : ice_array) {	//Remove the ice actor if not alive
		for (auto& colIter : rowIter) {
			if (colIter && !colIter->isAlive()) {
				colIter->resetAllBehaviors();
				colIter.reset();
			}
		}
	}
}

//Return a pointer to the whole vector of actors
std::unique_ptr<vector<std::shared_ptr<Actor>>> StudentWorld::getAllActors() {
	return make_unique<vector<shared_ptr<Actor>>>(actor_vec);
}

void StudentWorld::populateIce() {
	/*********************************
	Spawn ice in the coordinate specified
	Put the Ice in the ice array and also put the pointer into another containers that holds every actors
	*********************************/
	for (int row = 0; row < ice_array.size(); row++) {
		for (int col = 0; col < ice_array[row].size(); col++) {
			if (col < 33 && col > 30 && row > 4 && row < 59) {
				ice_array[row][col] = nullptr;	//Don't add ice in cols and rows between those range
			}
			else {
				ice_array[row][col] = make_shared<Ice>(this, true, col, row);	//Cols is the x location and row is the y location in Cartesian coordinate
				actor_vec.emplace_back(ice_array[row][col]);
			}
		}
	}
}

void StudentWorld::createPlayer() {
	player = make_shared<IceMan>(this);
	actor_vec.emplace_back(player);
}

void StudentWorld::mainCreateObjects() {
	/*****************************
	Actor of the same group
		Spawn an actor at the random location inside the restriction
		Spawn another actor follow the restriction
		If the newly spawn actor get in the detection range of the first actor then Destroy it
			Keep re-spawning the new actor until the detection doesn't hit
		No detection hit -> decrement the count need to spawn on the corresponding actor
		Move on to spawn the next actor
	*****************************/
	int currentLV = getLevel();
	int numBoulder = min(currentLV / 2 + 2, 9);
	int numGold = max(5 - currentLV / 2, 2);
	int numOil = min(2 + currentLV, 21);

	//Seed the random
	srand(time(0));
	int localX, localY;

	for (; numBoulder > 0; numBoulder--) {
		do {
			localX = rand() % 61;	// 0 - 60
			localY = rand() % 37 + 20; // 20 - 56
			if ((localX >= 30 && localX <= 33) || (localY >= 4 || localY <= 59))
				continue;
		} while (!createObjects<Boulder>(localX, localY));	//If object cannot create at the location then try again
	}

	for (; numGold > 0; numGold--) {
		do {
			localX = rand() % 61;
			localY = rand() % 57;	// 0 - 56
			if ((localX >= 30 && localX <= 33) || (localY >= 4 || localY <= 59))
				continue;
		} while (!createObjects<GoldNuggets>(localX, localY));
	}

	for (; numOil > 0; numOil--) {
		do {
			localX = rand() % 61;
			localY = rand() % 57;
			if ((localX >= 30 && localX <= 33) || (localY >= 4 || localY <= 59))
				continue;
		} while (!createObjects<OilBarrels>(localX, localY));
	}
}
