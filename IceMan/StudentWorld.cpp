#include "StudentWorld.h"
#include <algorithm>
#include <string>
#include <time.h>
#include <stdlib.h>
#include <type_traits>
#include <iomanip>
#include <sstream>

#include <iostream>
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
	default:
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
	ostringstream stream;

	lvl = to_string(getLevel()); 
	score = to_string(getScore()); 
	lives = to_string(getLives());
	health = to_string(player->getHealth() * 10); 
	wtr = to_string(player->getSquirtNum());
	gld = to_string(player->getGoldNum());
	//oil = to_string(oilsLeft);
	sonar = to_string(player->getSonarNum());

	//Fixed a little bit, following the pdf requirements
	/********************************************
	MAKE TEXT BE AT THE EDGE
	*******************************************/
	stream << "Lvl: " << setw(2) << lvl << ' ' ;
	stream << "Lives: " << lives << ' ';
	stream << "Hlth: " << setw(3) << health << "% ";
	stream << "Wtr: " << setw(2) << wtr << ' ';
	stream << "Gold: " << setw(2) << gld << ' ';
	stream << "Oil Left: " << setw(2) << oil << ' ';
	stream << "Sonar :" << setw(2) << sonar << ' ';
	stream << "Scr: " << setw(6) << setfill('0') << score;
	current_status = stream.str();
	//current_status += "Lvl:" + lvl + " Lives:  " + lives + " Hlth:  " + health + " Wtr:  " + wtr + " Gld:  " + gld + " Oil Left:  " + oil + " Sonar:  " + sonar + " Score:  " + score;
	setGameStatText(current_status);

	//Return for testing reason, THIS IS NOT CORRECT
	return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::doThings() {
	for (auto& actors : actor_vec) { // Iterates through entire vector of actor objects 
									// and has them call their own doSomething() methods.
		if (player && player->isAlive())
			player->doSomething(); // If the player is still alive, have them do something.
		else
			return GWSTATUS_PLAYER_DIED; // If the player has died, return the appropriate status.

		if (actors && actors->isAlive() && actors->type != Actor::ActorType::player) {
			//Logging
			//cout << "B4: " << actors->getHealth() << "->";
			actors->doSomething(); // If it is valid, have the actor do something.

			//Logging reason
			//if (actors->type == Actor::ActorType::ice)
				//cout << actors->getHealth() << "   ";
		}

		//Let's try just making the ice within proximity
		vector<weak_ptr<Actor>> iceTarget = iceInProxWithPlayer();
		for (auto& val : iceTarget) {
			shared_ptr<Actor>temp = val.lock();
			if (temp)
				temp->doSomething();
		}

		////Big performance hit right here

		//if (ice_array.size() > 0) {
		//	for (auto& iterRow : ice_array) {
		//		for (auto& iterCol : iterRow) {
		//			if (iterCol)
		//				iterCol->doSomething();
		//		}
		//	}
		//}

		if (oilsLeft == 0)
			return GWSTATUS_FINISHED_LEVEL; // Or if the player has found all the barrels of oil, return this status.
	}
	//Logging
	//cout << endl;
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

	if (!player->isAlive())
		player.reset();
}

//Return a pointer to the whole vector of actors
vector<shared_ptr<Actor>> StudentWorld::getAllActors() {
	return vector<shared_ptr<Actor>>(actor_vec);
}

void StudentWorld::populateIce() {
	/*********************************
	Spawn ice in the coordinate specified
	Put the Ice in the ice array and also put the pointer into another containers that holds every actors
	*********************************/
	for (int row = 0; row < ice_array.size(); row++) {
		for (int col = 0; col < ice_array[row].size(); col++) {
			if (col < 34 && col > 29 && row > 3 && row < 61) {
				ice_array[row][col] = nullptr;	//Don't add ice in cols and rows between those range
			}
			else {
				ice_array[row][col] = make_shared<Ice>(this, true, col, row);	//Cols is the x location and row is the y location in Cartesian coordinate
			}
		}
	}
}

void StudentWorld::createPlayer() {
	player = make_shared<IceMan>(this);
	actor_vec.push_back(player);
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
			localX = rand() % 57;	// 0 - 60
			localY = rand() % 33 + 20; // 20 - 56
			if ((localX >= 26 && localX <= 29) || (localY >= 0 || localY <= 55))
				continue;
		} while (!createObjects<Boulder>(localX, localY));	//If object cannot create at the location then try again
	}

	for (; numGold > 0; numGold--) {
		do {
			localX = rand() % 57;
			localY = rand() % 53;	// 0 - 56
			if ((localX >= 26 && localX <= 29) || (localY >= 0 || localY <= 55))
				continue;
		} while (!createObjects<GoldNuggets>(localX, localY));
	}

	for (; numOil > 0; numOil--) {
		do {
			localX = rand() % 57;
			localY = rand() % 53;
			if ((localX >= 26 && localX <= 29) || (localY >= 0 || localY <= 55))
				continue;
		} while (!createObjects<OilBarrels>(localX, localY));
	}
}

std::vector<std::weak_ptr<Actor>> StudentWorld::iceInProxWithPlayer() {
	/*****************************
		Check if the player is in certain radius of the actor
		Check in all direction, that means using a circle and Euclidean distance math, the detection range for the actor and the actor
		Use Euclidean to find out the distance from the player and the actor
		sum of detection range of the player and the actor is the "spot zone"
		If the distance is smaller or equal than the "spot zone"
			|
			Then return the intruder, the player(intruder) is "sensed"
		If it's not then return nullptr
	*****************************/
	/*SINCE THIS IS A SQUARE WE HAVE TO IMPLEMENT IT DIFFERENTLY*/
	vector<weak_ptr<Actor>> intruders;

	if (!ice_array.empty() && player && player->isAlive()) {
		//Ice array
		//array<array<shared_ptr<Ice>, COL_NUM>, ROW_NUM> ice_arr = std::move(source->getWorld()->getIceArr());

		int playerColRange = player->getCollisionRange();
		int localX = player->getX();
		int localY = player->getY();

		//Get only the ice in close proximity
		int spotPositiveX = playerColRange + localX + 1;
		int spotNegativeX = localX - playerColRange + 1;
		int spotPositiveY = playerColRange + localY + 1;
		int spotNegativeY = localY - playerColRange + 1;

		//Prune the distance so it doesn't go out of range
		for (; spotPositiveX >= COL_NUM; spotPositiveX--);
		for (; spotNegativeX < 0; spotNegativeX++);
		for (; spotPositiveY >= ROW_NUM; spotPositiveY--);
		for (; spotNegativeY < 0; spotNegativeY++);

		//Get the ice in them proximity
		for (auto i = spotNegativeY; i <= spotPositiveY; i++) {
			for (auto k = spotNegativeX; k <= spotPositiveX; k++) {
				intruders.push_back(ice_array[i][k]);
			}
		}


		//	for (auto& single : allActors) {
		//		if (single) {
		//			if (!single->isAlive() || single == source)	//The intruder and the player is the same actor
		//				continue;
		//			int distance = sqrt(pow(source->getCenterX() - single->getCenterX(), 2) + pow(source->getCenterY() - single->getCenterY(), 2));
		//			int spotZone = this->range + single->getCollisionRange();
		//			if (distance <= spotZone)
		//				intruders.push_back(single);
		//		}
		//	}
		//}
	}
	return intruders;
}

void StudentWorld::cleanUp() {
	//erase everything from vector
	actor_vec.erase(actor_vec.begin(), actor_vec.end());

	for (auto& rowIter : ice_array) {
		for (auto& colIter : rowIter) {
			colIter->resetAllBehaviors();
			colIter.reset();
		}
	}

	if (player)
		player.reset();
}

