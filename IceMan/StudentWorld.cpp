#include "StudentWorld.h"
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

//Cleanup: 
// + This phase occurs either when the player has lost a life(but has more lives left),the player has completed
//	 the current level, or the player has lost all of their lives and the game is over.	
// + This phase frees all of the objects in the World(e.g., Regular
//		Protesters, Ice, Boulders, Goodies, the Iceman, etc.) since the level has ended.
// + If gameplay is not over(i.e., the player has more lives), then the game proceeds back to the Initialization step,
//		 where the oil field is repopulated with new occupantsand game play restarts at the current level.
void StudentWorld::cleanUp() {

	//if player gets all the oil the level, the player has is completed 
	if (oilsLeft == 0){
	//free all of the objects in the World(e.g., Regular Protesters, Ice, Boulders, Goodies, the Iceman, etc.)
	//since the level has ended.

		//create iterator to go through whole vector and delete everything
		/*for (auto i = begin(actor_vec); i != end(actor_vec); i++) {
			actor_vec.erase(i);
			i = begin(actor_vec);
		}*/
		
		//clear(): Removes all elements from the vector(which are destroyed), leaving the container with a size of 0.
		actor_vec.clear();									//clear the vector with actors from game
		fill(ice_array.begin(), ice_array.end(), nullptr); //clear the 2D array of ice
		//DO NOT NEED TO CREATE INIT (Gamecontroller calls init for you)
	 }

	// if (player losses a life && still has lives left) && if (player has lost all lives)
	if (getLives() >= 0) {
	//	free all of the objects in the World(e.g., Regular Protesters, Ice, Boulders, Goodies, the Iceman, etc.)
	//	since the level has ended.
		
		actor_vec.clear();									//clear the vector with actors from game
		fill(ice_array.begin(), ice_array.end(), nullptr); //clear the 2D array of ice

		//DO NOT NEED TO CREATE INIT (Gamecontroller calls init for you)		
	}
}

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp
//put in my branch
unique_ptr<vector<std::shared_ptr<Actor>>> StudentWorld::getAllActors(){
	
	unique_ptr<vector<std::shared_ptr<Actor>>> temp = make_unique<vector<std::shared_ptr<Actor>>>(actor_vec);
	return temp;
}
