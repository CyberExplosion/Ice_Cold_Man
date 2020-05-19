#include "Actor.h"
#include "GameController.h"
#include "StudentWorld.h"
#include <cmath>
using namespace std;

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp


//Destroy an object or deal damage to characters
void Destroy::response() {
	if(target->isAlive())
		target->dmgActor(dmgTaken);
}

void FreeMovement::moveThatAss() {
}

void FallMovement::moveThatAss() {
}


void IceMan::doSomething() {
	if (!isAlive())
		return;
}

void Protesters::shout(Direction dir) {
}



void Protesters::doSomething() {
}


std::shared_ptr<Actor> RadarLikeDetection::sensedActor() {
	/*****************************
	Check if the player is in certain radius of the actor
	Check in all direction, that means using a circle and Euclidean distance math, the detection range for the actor and the actor
	Use Euclidean to find out the distance from the player and the actor
	sum of detection range of the player and the actor is the "spot zone"
	If the distance is smaller or equal than the "spot zone"
		|
		Then return the intruder, the player is "sensed"
	If it's not then return nullptr
*****************************/
	int distance = sqrt(pow(source->getX() - intruder->getX(), 2) + pow(source->getY() - intruder->getY(), 2));
	int spotZone = this->range + intruder->getDetectRange();
	if (distance <= spotZone)
		return intruder;
	return nullptr;
}

void RadarLikeDetection::behaveBitches() {
	sensedActor();
}


void CollisionDetection::behaveBitches() {
	collisionHappen();
}

bool LineOfSightDetection::seePlayer() {
	/******************************
Check if the player is in the horizontal or vertical direction of the actor
Eye sight is blocked by worldStatic, ice actor type
Check the vertical direction first
	Check eyesight from the position of the actor, if an actor block or get out of screen before reaching the player then return false
	If found the player then turn the actor toward the characters direction and return false
		Get X distance between the actor and the player
		If it's the same
				|
				Calculate the Y distance between the actor and the player (absolute value)
				Check if there's any ice or worlsStatic actor in the X location that has Y location smaller than the distance
					If there's smaller -> There's something blocking the view -> false
					If none -> see the player -> true
Same for horizontal direction
*******************************/
	return false;
}

void LineOfSightDetection::behaveBitches() {
	seePlayer();
}


void CollisionDetection::collide(std::shared_ptr<Actor>& source, std::shared_ptr<Actor>& receiver) {
	if (source->type == Actor::player) {
		switch (receiver->type) {
		case Actor::worldStatic:
			source->collisionResult = make_unique<Block>(source);
			break;
		case Actor::hazard: {
			source->collisionResult = make_unique<Destroy>(source, receiver->getStrength());
			break;
		}
		default:
			break;
		}
	}
	if (source->type == Actor::npc) {
		switch (receiver->type) {
		case Actor::hazard:
			source->collisionResult = make_unique<Destroy>(source, receiver->getStrength());
			break;
		case Actor::Actor::worldStatic:
		case Actor::ice:
			source->collisionResult = make_unique<Block>(source);
			break;
		default:
			break;
		}
	}
	//World static doesn't count because it's never move or destroy

	if (source->type == Actor::hazard) {
		switch (receiver->type) {
		case Actor::Actor::worldStatic:
		case Actor::Actor::npc:
		case Actor::player:
		case Actor::ice:
			source->collisionResult = make_unique<Destroy>(source, receiver->getStrength());
			break;
		default:
			break;
		}
	}
	if (source->type == Actor::ice) {
		switch (receiver->type) {
		case Actor::player:
			source->collisionResult = make_unique<Destroy>(source, receiver->getStrength());
			break;
		default:
			break;
		}
	}
	if (source->type == Actor::dropByPlayer) {
		switch (receiver->type) {
		case Actor::npc:
			source->collisionResult = make_unique<Destroy>(source, receiver->getStrength());
			break;
		default:
			break;
		}
	}
	if (source->type == Actor::collect) {
		switch (receiver->type) {
		case Actor::player:
			source->collisionResult = make_unique<Destroy>(source, receiver->getStrength());
			break;
		default:
			break;
		}
	}
}

//Collision is just a radar like detection but only cover a small radius
bool CollisionDetection::collisionHappen() {
/*****************************
See if you can "sensed" the actor. **This use an entirely different detection range than the "radar" detection range so it's ok**
Then a collision happen and you should produce a collision result
*****************************/
	if (source->isAlive() && intruder->isAlive()) {
		if (sensedActor())
			collide(source, intruder);
		else 
			return false;
		if (source->collisionResult) {
			source->collisionResult->response();
			return true;
		}
	}
	return false;
}


void Block::response() {
}

void Collectable::showSelf() {
}

void OilBarrels::doSomething() {
}

bool GoldNuggets::tempTimeEnd() {
	return false;
}

void GoldNuggets::drop() {
}

void GoldNuggets::doSomething() {
}

int SonarKit::getAmmo() {
	return 0;
}

int SonarKit::increaseAmmo(int amount) {
	return 0;
}

void SonarKit::useSonar() {
}

void SonarKit::doSomething() {
}

void Water::doSomething() {
}


void Squirt::increaseAmmo(int amount) {
}

void Squirt::shoot() {
}

void Squirt::doSomething() {
}

void Boulder::fall() {
}

bool Boulder::checkIceBelow() {
	return false;
}

void Boulder::doSomething() {
}

void Ice::doSomething() {
}

void ExistTemporary::showYourself() {
}

void ExistPermanently::showYourself() {
}



void ControlledMovement::moveThatAss() {
	/****************************
	This tells where the character that being controlled should move
	First you have to turn the character to face the direction you move first
	If the characters already facing the direction you move, then move it toward that direction 1 square
	If the move destination ends up with a collide block, activate the Block behavior
	If the move destination ends up with an ice or collectible, activate the Destroy behavior
	*****************************/
	if (key != INVALID_KEY) {
		switch (key) {
		case KEY_PRESS_DOWN:
			if (pawn->getDirection() != GraphObject::Direction::down) {
				pawn->setDirection(GraphObject::Direction::down);
			}
			else {
				if (!pawn->collisionDetection) {
					//This is bad, i should check the coordinate instead of making another actors to check it for me. But fuck it
					
					pawn->collisionDetection = make_unique<CollisionDetection>(pawn, pawn->getCollisionRange())
				}
			}
			break;
		default:
			break;
		}
	}
}
