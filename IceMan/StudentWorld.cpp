#include "StudentWorld.h"
#include <algorithm>
#include <string>
#include <time.h>
#include <stdlib.h>
//#include <type_traits>
#include <iomanip>
#include <cmath>
#include <future>
#include <list>
#include <iostream>
#include <execution>
using namespace std;

//testing variable
//int collectableLeft;
const int INVALID_LOCAL = -1;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp


int StudentWorld::move() {
	ticksBeforeSpawn--;
	tickSonar--;	//added by nelson
	if (tickSonar == 0) {
		TurnOffPowerDetectionRange();
	}


	auto fut = async(launch::async, &StudentWorld::findEmptyIce, this);	//Could take a long time so we create another thread

	updateStatus();
	createProtesters();
	createGoodies(fut.get());	//Give in the location here

	int status_of_game = doThings();

	deleteFinishedObjects();
	increaseEmptyIce();	//Increase the possible location that water can spawn

	switch (status_of_game)
	{
	case 0:
		return GWSTATUS_PLAYER_DIED;
	case 1: {
		//Update the graph in case we need to calculate path
		double localX, localY;
		pair<int, int> location;

		player->getAnimationLocation(localX, localY);

		location.first = static_cast<int>(localX);
		location.second = static_cast<int>(localY);
		graph->addNewVertice(location);

		return GWSTATUS_CONTINUE_GAME;
	}
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

	if (player) {
		health = to_string(player->getHealth() * 10);
		wtr = to_string(player->getWater());
		gld = to_string(player->getGold());
		sonar = to_string(player->getSonar());
	}
	else {
		health = '0';
		wtr = '0';
		gld = '0';
	}
	oil = to_string(oilsLeft);

	//Fixed a little bit, following the pdf requirements
	/********************************************
	MAKE TEXT BE AT THE EDGE
	*******************************************/
	stream << "Lvl: " << setw(2) << lvl << ' ';
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
	if (player && player->isAlive())
		player->doSomething(); // If the player is still alive, have them do something.
	else
		return GWSTATUS_PLAYER_DIED; // If the player has died, return the appropriate status.


	for (auto& actors : actor_vec) { // Iterates through entire vector of actor objects 
								// and has them call their own doSomething() methods.
		if (actors && actors->isAlive()) {
			actors->doSomething(); // If it is valid, have the actor do something.
		}
	}

	//Just the ice within the proximity of the player are allow to do any action
	vector<weak_ptr<Actor>> iceTarget = iceCollideWithActor(player);
	for (auto& val : iceTarget) {
		shared_ptr<Actor>temp = val.lock();
		if (temp)
			temp->doSomething();	//Don't know why it needs but it'll crash
	}

	if (oilsLeft == 0)
		return GWSTATUS_FINISHED_LEVEL; // Or if the player has found all the barrels of oil, return this status.

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

	if (player && !player->isAlive())
		player.reset();
}

bool StudentWorld::boulderFall(int x, int y)
{
	/*
		This function checks to see if there is ice underneath a boulder.
		If there isn't commence falling!
		x and y give us the location of the boulder.
	*/

	int targetY = y - 1; // The height one below the boulder.
	int tx1 = x; int tx2 = x + 1; int tx3 = x + 2; int tx4 = x + 3; // We need to check the width of the 
																	// boulder at a height just below it for ice.

	bool flag1 = true, flag2 = true, flag3 = true, flag4 = true;	// Each flag indicates if there is snow under the boulder. 
																	// True = there is ice underneath, false = no ice.
																	// If all four flags are false then there is no 
																	// ice underneath the boulder and we should let it fall!

	for (int row = 0; row < ice_array.size(); row++) {
		for (int col = 0; col < ice_array[row].size(); col++) {
			if (row == targetY && col == tx1) {
				if (ice_array[row][col] == nullptr) {
					flag1 = false;
					continue;
				}
			}
			if (row == targetY && col == tx2) {
				if (ice_array[row][col] == nullptr) {
					flag2 = false;
					continue;
				}
			}
			if (row == targetY && col == tx3) {
				if (ice_array[row][col] == nullptr) {
					flag3 = false;
					continue;
				}
			}
			if (row == targetY && col == tx4) {
				if (ice_array[row][col] == nullptr) {
					flag4 = false;
					continue;
				}
			}
		}
	}

	if (flag1 == false && flag2 == false && flag3 == false && flag4 == false) {
		return true;
	}
	return false;
}

//Return a pointer to the whole vector of actors
vector<shared_ptr<Actor>> StudentWorld::getAllActors() {
	return vector<shared_ptr<Actor>>(actor_vec);
}

void StudentWorld::populateIce() {
	/*********************************
	Spawn ice in the coordinate specified
	Put the Ice in the ice array and also put the pointer into another containers that holds every actors
	Put the empty Ice location in the vector
	*********************************/
	for (int row = 0; row < ice_array.size(); row++) {
		for (int col = 0; col < ice_array[row].size(); col++) {
			if (col <= shaftXoffsetR && col >= shaftXoffsetL && row >= shaftYoffsetD && row < shaftYoffsetU) {
				ice_array[row][col] = nullptr;	//Don't add ice in cols and rows between those range
			}
			else {
				ice_array[row][col] = make_shared<Ice>(this, true, col, row);	//Cols is the x location and row is the y location in Cartesian coordinate
			}
		}
	}
	for (int i = shaftYoffsetU; i >= shaftYoffsetD; i--) {
		empty_iceLocal.emplace_back(shaftXoffsetL, i);	//Put the whole shaft as location for empty ice that water can fit in
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
	////Test
	//currentLV = 13;

	int numBoulder = min(currentLV / 2 + 2, 9);
	int numGold = max(5 - currentLV / 2, 2);
	int numOil = min(2 + currentLV, 21);

	////Test
	//int numBoulder = 1;
	//int numGold = 1;
	//int numOil = 1;

	oilsLeft = numOil;

	//Seed the random
	srand(time(0));
	int localX, localY;

	createNPC();

	//numbers in the range [M, N] could be generated with something like
	//M + rand() / (RAND_MAX / (N - M + 1) + 1)

	//Test
	//cerr << "Boulder: \n";
	for (; numBoulder > 0; numBoulder--) {
		do {
			localX = 0 + rand() / (RAND_MAX / ((COL_NUM - OBJECT_LENGTH) - 0 + 1) + 1);	// 0 - 60 (It's actually 0 - 56) because the location starts at down-left corner
			localY = 20 + rand() / (RAND_MAX / ((ROW_NUM - OBJECT_LENGTH) - 20 + 1) + 1); // 20 - 56
			if ((localX >= shaftXoffsetL - OBJECT_LENGTH && localX <= shaftXoffsetR && localY >= shaftYoffsetD - OBJECT_LENGTH) || localX < 0 || localY < 0) {	//Location of the shaft
				++numBoulder;	//Make it loop again == Generate another random location
				break;
			}
			//Testing, remember to change the boulder location back to localX and Y
			//33 60 for testing collision with boulder
		} while (!createObjects<Boulder>(localX, localY));	//If object cannot create at the location then try again
	}

	//cerr << "Gold: \n";
	for (; numGold > 0; numGold--) {
		do {
			localX = 0 + rand() / (RAND_MAX / ((COL_NUM - OBJECT_LENGTH) - 0 + 1) + 1);	//0 - 60
			localY = 0 + rand() / (RAND_MAX / ((ROW_NUM - OBJECT_LENGTH) - 0 + 1) + 1);	// 0 - 52
			if ((localX >= shaftXoffsetL - OBJECT_LENGTH && localX <= shaftXoffsetR && localY >= shaftYoffsetD - OBJECT_LENGTH) || localX < 0 || localY < 0) {	//Location of the shaft
				++numGold;	//Make it loop again == Generate another random location
				break;
			}
			//Testing for hidden gold nugget showing up, it's 60 35
		} while (!createObjects<GoldNuggets>(localX, localY));
	}

	//cerr << "Oil: \n";
	for (; numOil > 0; numOil--) {
		do {
			localX = 0 + rand() / (RAND_MAX / ((COL_NUM - OBJECT_LENGTH) - 0 + 1) + 1);	//0 - 60
			localY = 0 + rand() / (RAND_MAX / ((ROW_NUM - OBJECT_LENGTH) - 0 + 1) + 1);	// 0 - 52
			if ((localX >= shaftXoffsetL - OBJECT_LENGTH && localX <= shaftXoffsetR && localY >= shaftYoffsetD - OBJECT_LENGTH) || localX < 0 || localY < 0) {	//Location of the shaft
				++numOil;	//Make it loop again == Generate another random location
				break;
			}
		} while (!createObjects<OilBarrels>(localX, localY));
	}

}

void StudentWorld::createProtesters() {
	int currentLV = getLevel();
	if (ticksBeforeSpawn == 0) {
		if (protesterCount != protesterSpawnLimit) {
			createNPC();
		}
		ticksBeforeSpawn = max(25, 200 - currentLV);
	}
}

void StudentWorld::initSpawnParameters() {
	/*
		This function determines the parameters in which a protester can spawn.
		These formulas are provided by the PDF.
	*/
	int currentLV = getLevel();
	ticksBeforeSpawn = max(25, 200 - currentLV);
	protesterSpawnLimit = min(15, 2 + int(currentLV * 1.5));

}

void StudentWorld::initNPCPath() {
	graph = make_unique<Graph>(empty_iceLocal);
}

void StudentWorld::createNPC() {
	/****************************
	Use a random number to determine the odds
	Spawn the npc at the appropriate location
	****************************/
	srand(time(0));
	int currentLvl = getLevel();
	int probOfHardcore = min(90, currentLvl * 10 + 30);
	bool spawnHardcore = (rand() % 100) < probOfHardcore;	//If smaller than the prob then it fall into that probability



	if (spawnHardcore) {
		actor_vec.emplace_back(make_shared<Protesters>(this));
	}
	else
		actor_vec.emplace_back(make_shared<HardcoreProtesters>(this));
}

std::vector<std::weak_ptr<Actor>> StudentWorld::iceCollideWithActor(std::shared_ptr<Actor> actor, bool radarMode) {
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
	}
	return intruders;
}

std::vector<std::weak_ptr<Actor>> StudentWorld::actorsCollideWithMe(std::shared_ptr<Actor> actor, bool radarMode) {

	vector<weak_ptr<Actor>> intruders;

	if (!actor_vec.empty() && actor && actor->isAlive()) {
		for (auto& each : actor_vec) {
			int playerRange;
			int eachRange;

			//Depend on which mode, different range will be used
			if (radarMode) {
				playerRange = actor->getDetectRange();
				eachRange = each->getDetectRange();

			}
			else {
				playerRange = actor->getCollisionRange();
				eachRange = each->getCollisionRange();
			}

			int actX = actor->getX();
			int actY = actor->getY();

			int spotNegativeX = actX - eachRange;
			int spotPositiveX = actX + playerRange;	//Range from left to right
			int spotNegativeY = actY - eachRange;
			int spotPositiveY = actY + playerRange;	//Range from bottom to top

			//Prune the distance so it doesn't go out of range
			//for (; spotPositiveX >= COL_NUM; spotPositiveX--);
			//for (; spotNegativeX < 0; spotNegativeX++);
			//for (; spotPositiveY >= ROW_NUM; spotPositiveY--);
			//for (; spotNegativeY < 0; spotNegativeY++);

			if (each != actor) {
				//Get the intruders in the proximity
				for (int i = spotNegativeY; i <= spotPositiveY; i++) {
					for (int k = spotNegativeX; k <= spotPositiveX; k++) {
						if (each->isAlive() && each->getX() == k && each->getY() == i)
							intruders.push_back(each);
					}
				}
			}
		}
	}
	return intruders;
}

bool StudentWorld::createSquirt() {
	if (player && player->isAlive()) {
		GraphObject::Direction sqrtDir = player->getDirection();
		int localX = player->getX();
		int localY = player->getY();
		int playerColRange = player->getCollisionRange();

		Squirt temp(this, localX, localY, sqrtDir);
		int squirtColRange = temp.getCollisionRange();


		switch (sqrtDir) {
		case GraphObject::up:
			localY += playerColRange + 1;			//If the player look in different location, we need to account for
			break;									//the collision between the squirt and player since it's a hazard
		case GraphObject::down:
			localY -= squirtColRange + 1;
			break;
		case GraphObject::left:
			localX -= squirtColRange + 1;
			break;
		case GraphObject::right:
			localX += playerColRange + 1;
			break;
		default:
			break;
		}
		//Plus Object Length cause the location is lower left corner
		if ((localY < 0 || localY > ROW_NUM + OBJECT_LENGTH - squirtColRange) || (localX < 0 || localX > COL_NUM + OBJECT_LENGTH - squirtColRange) || (localX == player->getX() && localY == player->getY()))	//Value stay the same or out of bounds
			return false;
		else {
			for (int row = localY; row <= localY + squirtColRange && row < ROW_NUM; row++)
				for (int col = localX; col <= localX + squirtColRange && col < COL_NUM; col++)
					if (ice_array[row][col])	//If there are ice in the way
						return false;

			actor_vec.emplace_back(make_shared<Squirt>(this, localX, localY, sqrtDir));
			return true;
		}
	}
	return false;
}


/////test this for bugss plssss
bool StudentWorld::createGoodies(pair<int, int> locale) {
	//See if there's any actors in the proximity

	//Water dummy(this, x, y, dir, size, depth, hp, strength, col, detect, sound, score);
	auto dummy = make_shared<Water>(this, locale.first, locale.second, GraphObject::Direction::none, 1.0, 3, 1, 0, OBJECT_LENGTH - 1, OBJECT_LENGTH - 1, SOUND_NONE, 0);
	dummy->detectBehavior = make_unique<RadarLikeDetection>(dummy);
	dummy->detectBehavior->behaveBitches();		//Check if any actor already in your position
	if (!dummy->detectBehavior->wp_intruders.empty())
		return false;

	int currentLvl = getLevel();
	double spawnChance = currentLvl * 25 + 300;
	double spawnPercentage = (1 / spawnChance) * 100;	//1 out of G chance

	bool allowSpawn = (rand() % 100) < spawnPercentage;

	if (allowSpawn) {
		bool spawnWater = (rand() % 100) < WATER_CHANCE;	//water is 80 percent
		////Tests
		//bool spawnWater = true;
		///////////////
		if (locale.first != INVALID_LOCAL && locale.second != INVALID_LOCAL) {
			if (spawnWater) {
				actor_vec.emplace_back(make_shared<Water>(this, locale.first, locale.second));
			}
			else {
				actor_vec.emplace_back(make_shared<SonarKit>(this));
			}
			return true;
		}
	}
	return false;
}

//The function put in location that the player traveled as possible empty ice place
void StudentWorld::increaseEmptyIce() {
	if (player && player->isAlive()) {
		double localX, localY;
		player->getAnimationLocation(localX, localY);
		if (localX > 0 && localX < COL_NUM - OBJECT_LENGTH && localY > 0 && localY < ROW_NUM - OBJECT_LENGTH) {	//Make sure it's in the ice field
			for (int i = localY; i < ROW_NUM && i < localY + OBJECT_LENGTH; i++) {	//Check the surrounding ice to make sure there's none exist in 4x4 radius
				for (int k = localX; k < COL_NUM && k < localX + OBJECT_LENGTH; k++) {
					if (ice_array[i][k])	//If there's an ice exist in an area, then it's not qualified
						return;
				}
			}
			empty_iceLocal.emplace_back(localX, localY);
		}
	}
}

//Function will return a random pair of location for possible water spawning place
std::pair<int, int> StudentWorld::findEmptyIce() {
	if (player && player->isAlive()) {
		while (!empty_iceLocal.empty()) {
			//numbers in the range [M, N] could be generated with something like
			//M + rand() / (RAND_MAX / (N - M + 1) + 1)

			int theOne = rand() / (RAND_MAX / (empty_iceLocal.size() - 0) + 1);
			pair<int, int> locale = empty_iceLocal[theOne];

			int distance = sqrt(pow(locale.first - player->getX(), 2) + pow(locale.second - player->getY(), 2));
			if (distance >= DIST_ALLOW_BETW_SPAWN)	//Return the location if it's different than where the player is
				return locale;
		}
	}
	return make_pair(INVALID_LOCAL, INVALID_LOCAL);	//Should throw some error here instead, but meh
}

void StudentWorld::useSonar()
{
	//call player, player calls power function
	playSound(SOUND_SONAR);
	player->setDetectRange(8);	//increase
	tickSonar = 1;
}

void StudentWorld::TurnOffPowerDetectionRange()
{
	player->setDetectRange(4);
}


void StudentWorld::cleanUp() {
	//erase everything from vector
	actor_vec.clear();
	empty_iceLocal.clear();

	for (auto& rowIter : ice_array) {
		for (auto& colIter : rowIter) {
			if (colIter)
				colIter->resetAllBehaviors();
			colIter.reset();
		}
	}

	if (player)
		player.reset();
}

void Graph::createEdge() {
	vector<Vertice> neighbor_vec;
	for (auto& verticle : m_graph) {
		for_each(begin(m_graph), end(m_graph),
			[&](list<Vertice> neighborList) {	//If there are vertices that are adjacent then create an edge for them
				if ((neighborList.front().location.first == verticle.front().location.first + OBJECT_LENGTH) || (neighborList.front().location.first == verticle.front().location.first - OBJECT_LENGTH) || (neighborList.front().location.second == verticle.front().location.second + OBJECT_LENGTH) || (neighborList.front().location.second == verticle.front().location.second - OBJECT_LENGTH))
					neighbor_vec.push_back(neighborList.front());	//Push the neighbor into the vector
			});

		for (auto& neighbor : neighbor_vec) {
			verticle.push_back(neighbor);	//Add the neighbor into your list of many neighbors
		}
		neighbor_vec.clear();	//Prepare for another neighbor_vec
	}
}

void Graph::populateGraph(vector<pair<int, int>> emptyIce_vec) {
	for (auto& val : emptyIce_vec) {
		Vertice vert(IFN, val);	//Set the distance of the graph at infinity
		list<Vertice> vertice;
		vertice.push_front(std::move(vert));

		m_graph.push_back(std::move(vertice));	//Push the vertice into the graph
	}
	createEdge();	//Create edges after making all vertices
}

//Create another vertice and add edges to if automatically
void Graph::addNewVertice(std::pair<int, int> location) {
	Vertice vert(IFN, std::move(location));
	list<Vertice> vertList;
	vertList.push_front(std::move(vert));

	//Add edges to the new vertices
	vector<Vertice> neighbor_vec;
	for_each(begin(m_graph), end(m_graph),
		[&neighbor_vec, vertList](list<Vertice> neighborList) {
			if ((neighborList.front().location.first == vertList.front().location.first + 4) || (neighborList.front().location.first == vertList.front().location.first - 4) || (neighborList.front().location.second == vertList.front().location.second + 4) || (neighborList.front().location.second == vertList.front().location.second - 4))
				neighbor_vec.push_back(vertList.front());	//Find all possible edges
		});
	for (auto& neighbor : neighbor_vec) {	//Add edges here
		vertList.push_back(neighbor);
	}

	m_graph.push_back(std::move(vertList));	//Put the new vertice with edges into the graph
}

//The function return an map of all location with there respective "cost" to travel to
unordered_map<std::pair<int, int>, int, pairHash> Graph::distValueGenerate(std::pair<int, int> source) {
	auto start = find_if(execution::par, begin(m_graph), end(m_graph),	//Start will return the iterator to the starting Vertices List
		[&source](list<Vertice> vert) {
			return vert.front().location == source;	//Find the correct verticle to start from
		});

	//Create a map to store the distance needs to arrive at a particular vertice(location)
	unordered_map<pair<int, int>, int, pairHash> distTravel;

	scoped_lock<mutex> betterLock(locker);	//All of next step is critical

	if (start != end(m_graph)) {
		//Create a queue for breadth first search. It will hold the neighbors
		queue<list<Vertice>> que;

		//Set the start node distance and preliminary stuff
		start->front().distance = 0;
		que.push(*start);	//Push the starting Vertice as the first victim
		distTravel[start->front().location] = 0;	//Set the distance traveling to starting location be 0

		//Going through all the neighbor vertices
		while (!que.empty()) {
			//Pop the vertice out to work with it
			auto neighbors = que.front();	//If just start the loop, then ** neighbors == start **
			que.pop();

			//Get all adjacent vertices, if it's not visited then mark it's visited then push it to the queue
			for (auto& next_neighbor : neighbors) {
				if (distTravel.count(next_neighbor.location) == 0) {	//If we haven't visited this neighbor yet --> The distance to the neighbor not exist

					next_neighbor.distance = distTravel[neighbors.front().location] + 1;	//Distance from the first neighbor to the next one is an increase of 1
					distTravel[next_neighbor.location] = next_neighbor.distance;	//Update the travel distance to "this" neighbor

					//Find the neighbor's list of neighbors to continue expanding
					auto next = find_if(execution::par, begin(m_graph), end(m_graph), [next_neighbor](list<Vertice> val) {return val.front().location == next_neighbor.location; });
					if (next != end(m_graph))
						que.push(*next);
				}
			}
		}
	}
	return distTravel;
}
