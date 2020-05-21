#include "Actor.h"
#include "GameController.h"
#include "StudentWorld.h"
#include <cmath>
using namespace std;

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp


void Destroy::resetBehavior() {
	target.reset();
}

//Destroy an object or deal damage to characters
void Destroy::response() {
	if (target && target->isAlive())
		target->dmgActor(dmgTaken);
	if (target && !target->isAlive()) {
		target->getWorld()->playSound(SOUND_DIG);
		target->resetAllBehaviors();
	}
}

void FreeMovement::moveThatAss() {
}

void FreeMovement::resetBehavior() {
}

void FallMovement::moveThatAss() {
}

void FallMovement::resetBehavior() {
}

/////////*** NOT USED ***/////////////
shared_ptr<Actor> IceMan::findPlayer() {
	vector<shared_ptr<Actor>> acVec = *(getWorld()->getAllActors());
	auto re = std::find_if(rbegin(acVec), rend(acVec),	//Since this function is invoked right when created the player, there's a high chance the player is at the last place in the vector
		[](shared_ptr<Actor>& val) {
			if (val->type == player)
				return true;
			return false;
		});
	if (re == rend(acVec))	//The player is not in the actor list, which is unlikely
		*re = nullptr;
	return *re;
}
/***********************************/

void IceMan::doSomething() {
	/*******************************
	Initialize the existence behavior, the movement behavior, the collision detection and the collision behavior
	Check for collision detection and collision behavior with every call
	Then do all the behavior
	********************************/
	if (!isAlive()) {
		resetAllBehaviors();
		return;
	}
	//This certainly will cause problem in the future. TOO BAD
	shared_ptr<Actor> mySelf = shared_from_this();

	if (!movementBehavior) {
		movementBehavior = std::make_unique<ControlledMovement>(mySelf);
	}
	if (!displayBehavior)
		displayBehavior = make_unique<ExistPermanently>();
	if (!collisionDetection)
		collisionDetection = make_unique<CollisionDetection>(mySelf, mySelf->getCollisionRange());

	displayBehavior->showYourself();
	collisionDetection->behaveBitches();
	movementBehavior->moveThatAss();
	if (collisionResult)
		collisionResult->response();
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
		Then return the intruder, the player(intruder) is "sensed"
	If it's not then return nullptr
*****************************/
	if (source) {
		auto allActors = std::move(*(source->getWorld()->getAllActors()));
		for (const auto& intruder : allActors) {
			if (!intruder->isAlive() || intruder == source)	//The intruder and the player is the same actor
				continue;
			int distance = sqrt(pow(source->getX() - intruder->getX(), 2) + pow(source->getY() - intruder->getY(), 2));
			int spotZone = this->range + intruder->getCollisionRange();
			if (distance <= spotZone)
				return intruder;
		}
	}
	return nullptr;
}

void RadarLikeDetection::behaveBitches() {
	sensedActor();
}


void CollisionDetection::behaveBitches() {
	if (source) {
		if (collisionHappen()) {	//Make the Actor produce a result because of collision
			source->collisionResult->response();
		}
	}
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


void CollisionDetection::collide(std::shared_ptr<Actor> source, std::shared_ptr<Actor> receiver) {
	if ((source && receiver) && (source->isVisible() && receiver->isVisible()) && source != receiver) {	//Only enable collision for things that are shown
		if (source->type == Actor::player) {
			switch (receiver->type) {
			case Actor::worldStatic:
				source->collisionResult = make_unique<Block>(source);
				break;
			case Actor::npc:
			case Actor::hazard:
				source->collisionResult = make_unique<Destroy>(source, receiver->getStrength());
				break;
			default:
				break;
			}
		}
		if (source->type == Actor::npc) {
			switch (receiver->type) {
			case Actor::hazard:
				source->collisionResult = make_unique<Destroy>(source, receiver->getStrength());
				break;
			case Actor::worldStatic:
			case Actor::ice:
			case Actor::player:
				source->collisionResult = make_unique<Block>(source);
				break;
			default:
				break;
			}
		}

		if (source->type == Actor::worldStatic) {
			switch (receiver->type) {
			default:
				source->collisionResult = make_unique<Block>(source);
			}
		}

		if (source->type == Actor::hazard) {
			switch (receiver->type) {
			case Actor::Actor::worldStatic:
			case Actor::Actor::npc:
			case Actor::player:
			case Actor::ice:
				source->collisionResult = make_unique<Destroy>(source, receiver->getStrength());
				break;
			default:
				source->collisionResult = make_unique<Block>(source);
			}
		}
		if (source->type == Actor::ice) {
			switch (receiver->type) {
			case Actor::player:
			case Actor::worldStatic:
			case Actor::collect:
				source->collisionResult = make_unique<Destroy>(source, receiver->getStrength());
				break;
			default:
				source->collisionResult = make_unique<Block>(source);
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
}

//Collision is just a radar like detection but only cover a small radius
bool CollisionDetection::collisionHappen() {
	/*****************************
	See if you can "sensed" the actor. **This use an entirely different detection range than the "radar" detection range so it's ok**
	Then a collision happen and you should produce a collision result
	*****************************/
	if (intruder) {
		if (source->isAlive() && intruder->isAlive()) {
			//Produce a collision result right here
			collide(source, intruder);
			if (source->collisionResult)
				return true;
		}
	}
	return false;
}

void Block::resetBehavior() {
	target.reset();
}

//***This would cause problem if the moveTo() function create animation.****
void Block::response() {
	//Move to the same location == Standing still
	if (target)
		target->moveTo(target->getX(), target->getY());
}

void Collectable::showSelf() {
}

void OilBarrels::doSomething() {
	if (!isAlive()) {
		resetAllBehaviors();
		return;
	}

	shared_ptr<Actor>self = shared_from_this();
	//Create the behavior
	if (!existBehavior)
		existBehavior = make_unique<ExistPermanently>();
	if (!detectBehavior)
		detectBehavior = make_unique<RadarLikeDetection>(self, self->getDetectRange());
	if (!collisionDetection)
		collisionDetection = make_unique<CollisionDetection>(self, self->getCollisionRange());

	//Use the behavior
	collisionDetection->behaveBitches();
	detectBehavior->behaveBitches();
	existBehavior->showYourself();
	if (collisionResult)
		collisionResult->response();
}

bool GoldNuggets::tempTimeEnd() {
	return false;
}

void GoldNuggets::drop() {
}

void GoldNuggets::doSomething() {
	if (!isAlive()) {
		resetAllBehaviors();
		return;
	}

	shared_ptr<Actor> self = shared_from_this();

	//Create behavior
	if (!displayBehavior)
		if (pickableByPlayer)
			displayBehavior = make_unique<ExistPermanently>();
		else {
			displayBehavior.reset();
			displayBehavior = make_unique<ExistTemporary>();
		}
	if (!detectBehavior)
		detectBehavior = make_unique<RadarLikeDetection>(self, self->getDetectRange());
	if (!collisionDetection)
		collisionDetection = make_unique<CollisionDetection>(self, self->getCollisionRange());

	//Use the behavior
	displayBehavior->showYourself();
	detectBehavior->behaveBitches();
	collisionDetection->behaveBitches();
	if (collisionResult)
		collisionResult->response();
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
	if (!isAlive()) {
		resetAllBehaviors();
		return;
	}

	//Create behaviors
	shared_ptr<Actor>self = shared_from_this();
	if (!existBehavior)
		existBehavior = make_unique<ExistTemporary>();
	if (!detectBehavior)
		detectBehavior = make_unique<RadarLikeDetection>(self, self->getDetectRange());
	if (!collisionDetection)
		collisionDetection = make_unique<CollisionDetection>(self, self->getCollisionRange());

	//Use the behaviors
	existBehavior->showYourself();
	detectBehavior->behaveBitches();
	collisionDetection->behaveBitches();
	if (collisionResult)
		collisionResult->response();
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
	/********************************
	Initialize the existence Behavior, the collision detection behavior and the collision result behavior
	Check for collision detection and collision result with every call
	Then do the behaviors
	*********************************/
	if (isAlive()) {
		shared_ptr<Actor> self = shared_from_this();
		//Create behaviors
		if (!displayBehavior)
			displayBehavior = make_unique<ExistPermanently>();
		if (!collisionDetection)
			collisionDetection = make_unique<CollisionDetection>(self, self->getCollisionRange());

		//Use behaviors
		displayBehavior->showYourself();
		collisionDetection->behaveBitches();
		if (collisionResult)
			collisionResult->response();
	}
	else
		resetAllBehaviors();
}

void ExistTemporary::showYourself() {
}

void ExistTemporary::resetBehavior() {
}

void ExistPermanently::showYourself() {
}

void ExistPermanently::resetBehavior() {
}

void ControlledMovement::resetBehavior() {
	pawn.reset();
}

void ControlledMovement::moveThatAss() {
	/****************************
	This tells where the character that being controlled should move
	First you have to turn the character to face the direction you move
	If the characters already facing the direction you move, then move it toward that direction 1 square
	*****************************/
	if (pawn && pawn->isAlive()) {
		if (!pawn->getWorld()->getKey(key))
			key = INVALID_KEY;
		if (key != INVALID_KEY) {
			switch (key) {
			case KEY_PRESS_DOWN:
				if (pawn->getDirection() != GraphObject::Direction::down)
					pawn->setDirection(GraphObject::Direction::down);
				else {
					if (pawn->getY() - 1 < 0)
						break;
					else
						pawn->moveTo(pawn->getX(), pawn->getY() - 1);
				}
				break;
			case KEY_PRESS_UP:
				if (pawn->getDirection() != GraphObject::Direction::up)
					pawn->setDirection(GraphObject::Direction::up);
				else {
					if (pawn->getY() + 1 > 60)
						break;
					else
						pawn->moveTo(pawn->getX(), pawn->getY() + 1);
				}
				break;
			case KEY_PRESS_RIGHT:
				if (pawn->getDirection() != GraphObject::Direction::right)
					pawn->setDirection(GraphObject::Direction::right);
				else {
					if (pawn->getX() + 1 > 60)
						break;
					else
						pawn->moveTo(pawn->getX() + 1, pawn->getY());
				}
				break;
			case KEY_PRESS_LEFT:
				if (pawn->getDirection() != GraphObject::Direction::left)
					pawn->setDirection(GraphObject::Direction::left);
				else {
					if (pawn->getX() - 1 < 0)
						break;
					else
						pawn->moveTo(pawn->getX() - 1, pawn->getY());
				}
				break;
			default:
				break;
			}
		}
	}
}

void PursuingMovement::moveThatAss() {
}

void PursuingMovement::resetBehavior() {
}

void SquirtMovement::moveThatAss() {
}

void SquirtMovement::resetBehavior() {
}

void Actor::resetAllBehaviors() {
	if (movementBehavior)
		movementBehavior->resetBehavior();
	if (collisionResult)
		collisionResult->resetBehavior();
	if (detectBehavior)
		detectBehavior->resetBehavior();
	if (collisionDetection)
		collisionDetection->resetBehavior();
}

void shout(Actor::Direction dir) {

}

void doSomething() {
}