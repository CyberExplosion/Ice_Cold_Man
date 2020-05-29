#include "StudentWorld.h"
#include <algorithm>
#include <string>
#include <time.h>
#include <stdlib.h>
#include <type_traits>
#include <iomanip>
#include <sstream>
#include <cmath>

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

		//Just the ice within the proximity of the player are allow to do any action
		vector<weak_ptr<Actor>> iceTarget = iceCollideWithActor(player);
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
	//actor_vec.push_back(player);
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
	////////Test
	//int numBoulder = 1;
	//int numGold = 1;
	int numGold = max(5 - currentLV / 2, 2);
	int numOil = min(2 + currentLV, 21);
	//int numOil = 0;

	//Seed the random
	srand(time(0));
	int localX, localY;
	int shaftXoffsetL = 30,
		shaftXoffsetR = 33,
		shaftYoffsetD = 4,	//All inclusive
		shaftYoffsetU = 60;

	for (; numBoulder > 0; numBoulder--) {
		do {
			localX = rand() % 61 - OBJECT_LENGTH;	// 0 - 60 (It's actually 0 - 56) because the location starts at down-left corner
			localY = rand() % 37 + 20 - OBJECT_LENGTH; // 20 - 56 (Actually 20 - 52)
			if ((localX >= shaftXoffsetL - OBJECT_LENGTH && localX <= shaftXoffsetR && localY >= shaftYoffsetD - OBJECT_LENGTH) || localX < 0 || localY < 0) {	//Location of the shaft
				++numBoulder;	//Make it loop again == Generate another random location
				break;
			}
			//Testing, remember to change the boulder location back to localX and Y
			//33 60 for testing collision with boulder
		} while (!createObjects<Boulder>(localX, localY));	//If object cannot create at the location then try again
	}

	for (; numGold > 0; numGold--) {
		do {
			localX = rand() % 61 - OBJECT_LENGTH;
			localY = rand() % 57 - OBJECT_LENGTH;	// 0 - 56
			if ((localX >= shaftXoffsetL - OBJECT_LENGTH && localX <= shaftXoffsetR && localY >= shaftYoffsetD - OBJECT_LENGTH) || localX < 0 || localY < 0) {	//Location of the shaft
				++numGold;	//Make it loop again == Generate another random location
				break;
			}
		} while (!createObjects<GoldNuggets>(localX, localY));
	}

	for (; numOil > 0; numOil--) {
		do {
			localX = rand() % 61 - OBJECT_LENGTH;
			localY = rand() % 57 - OBJECT_LENGTH;
			if ((localX >= shaftXoffsetL - OBJECT_LENGTH && localX <= shaftXoffsetR && localY >= shaftYoffsetD - OBJECT_LENGTH) || localX < 0 || localY < 0) {	//Location of the shaft
				++numOil;	//Make it loop again == Generate another random location
				break;
			}
		} while (!createObjects<OilBarrels>(localX, localY));
	}
}

std::vector<std::weak_ptr<Actor>> StudentWorld::iceCollideWithActor(std::shared_ptr<Actor> actor) {
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

	if (!ice_array.empty() && actor && actor->isAlive()) {
		//Ice array
		//array<array<shared_ptr<Ice>, COL_NUM>, ROW_NUM> ice_arr = std::move(source->getWorld()->getIceArr());
		
		//Player collision range is the size of the player
		int playerColRange = actor->getCollisionRange();	//Collision range from the lower left corner toward the positive y and x
		int localX = actor->getX();
		int localY = actor->getY();

		//Get only the ice in close proximity
		int spotPositiveX = playerColRange + localX;	//Don't know why plus 1 on the positive side, probably gota do with something involve pixel and array calculation
		int spotNegativeX = localX;	//Found out why plus 1. Example: the ice takes up 4 slots: 52 53 54 55 -> 53 is the center and thus we need to plus 1 on the positive side
		int spotPositiveY = playerColRange + localY;	//It needs to plus 1 to look good, don't know why
		int spotNegativeY = localY;

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

std::vector<std::weak_ptr<Actor>> StudentWorld::actorsCollideWithMe(std::shared_ptr<Actor> actor) {
	
	vector<weak_ptr<Actor>> intruders;
	
	if (!actor_vec.empty() && actor && actor->isAlive()) {
		//Player collision range is actually the size of the its' own
		int colRangePositive = actor->getCollisionRange();
		int localX = actor->getX();
		int localY = actor->getY();

		for (auto& each : actor_vec) {

			int actRangePositive = each->getCollisionRange();	//On positive side the document want us to make it collide with each actor when you're 3 squares away
			//int actRangeNegative = each->getCollisionRange();	//And since each actor collision range is 3 so it's fine
			int actX = each->getX();
			int actY = each->getY();
			////////////////////////////

			double distance = sqrt(pow(localX - actX, 2) + pow(localY - actY, 2));	//Euclidean distance
			
			if (localX >= actX && localY >= actY) {	//If inside the positive area of "each"
				double eachSpotZone = sqrt(pow(actRangePositive, 2) + pow(actRangePositive, 2));	//When on positive side, the spot zone is the distance from the lower left to their own collision range
				if (distance <= eachSpotZone)
					intruders.push_back(each);
			}
			else {	//Negative side of "each"
				double actorSpotZone = sqrt(pow(colRangePositive, 2) + pow(colRangePositive, 2));	//The spot zone of the actor
				if (distance <= actorSpotZone)
					intruders.push_back(each);
			}
		}
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

