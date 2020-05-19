#include "StudentWorld.h"
#include <string>
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
}

int StudentWorld::doThings() {
	shared_ptr<Actor>p;
	for (auto actors : actor_vec) { // Iterates through entire vector of actor objects 
									// and has them call their own doSomething() methods.
		if (actors->isAlive()) 
			actors->doSomething(); // If it is valid, have the actor do something.
		if (player->isAlive())
			player->doSomething(); // If the player is still alive, have them do something.
		else
			return GWSTATUS_PLAYER_DIED; // If the player has died, return the appropriate status.
		if (oilsLeft == 0)
			return GWSTATUS_FINISHED_LEVEL; // Or if the player has found all the barrels of oil, return this status.
	}

	return GWSTATUS_CONTINUE_GAME; // If the player hasn't died and hasn't found all the oils, continue to next tick. 
}

void StudentWorld::deleteFinishedObjects() {
	actor_vec.erase(remove_if(actor_vec.begin(), actor_vec.end(), [](Actor temp) {
		if (!temp.isAlive()) // If the object is not alive anymore, keep it at the back. 
							 // Else, we shift it to the front of the vector and delete it.
			return false;
		return true;
		}));
}