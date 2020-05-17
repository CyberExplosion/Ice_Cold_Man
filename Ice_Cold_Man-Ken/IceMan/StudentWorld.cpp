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
	doThings();
	deleteFinishedObjects();
}

int StudentWorld::updateStatus() {
	string current_status;
	string lvl, health, wtr, gld, oil, sonar, score;
	lvl = to_string(getLevel()); health = to_string()
}

int StudentWorld::doThings() {

}

int StudentWorld::deleteFinishedObjects() {

}