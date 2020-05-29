#include "Actor.h"
#include "GameController.h"
#include "StudentWorld.h"
#include <cmath>
#include <iostream>

using namespace std;

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp


void Destroy::resetBehavior() {
	wp_target.reset();
}

//Destroy an object or deal damage to characters
void Destroy::response() {
	shared_ptr<Actor> target = wp_target.lock();

	if(target && target->isAlive())
		target->dmgActor(dmgTaken);
	if (target && !target->isAlive()) {
		target->resetAllBehaviors();
	}

	target.reset();
}

void FreeMovement::moveThatAss() {
}

void FreeMovement::resetBehavior() {
}

void FallMovement::moveThatAss() {
}

void FallMovement::resetBehavior() {
}

///////////*** NOT USED ***/////////////
//shared_ptr<Actor> IceMan::findPlayer() {
//	vector<shared_ptr<Actor>> acVec = *(getWorld()->getAllActors());
//	auto re = std::find_if(rbegin(acVec), rend(acVec),	//Since this function is invoked right when created the player, there's a high chance the player is at the last place in the vector
//		[](shared_ptr<Actor>& val) {
//			if (val->type == player)
//				return true;
//			return false;
//		});
//	if (re == rend(acVec))	//The player is not in the actor list, which is unlikely
//		*re = nullptr;
//	return *re;
//}
///***********************************/

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
	weak_ptr<Actor> mySelf = this->getWorld()->getPlayer();

	if (!movementBehavior) {
		movementBehavior = std::make_unique<ControlledMovement>(mySelf);
	}
	if (!displayBehavior)
		displayBehavior = make_unique<ExistPermanently>();
	
	//Reset cycle of collision result and detection
	collisionResult.reset();
	collisionDetection = make_unique<CollisionDetection>(mySelf, this->getCollisionRange());
	collisionDetection->behaveBitches();	//If there's a detection then a response is already made automatically
	///////////////
	displayBehavior->showYourself();
	movementBehavior->moveThatAss();

	
	//if (collisionResult)
	//	collisionResult->response();
}

void IceMan::dmgActor(int amt) {
	Actor::dmgActor(amt);
	getWorld()->playSound(dmgSound);
}



void Protesters::doSomething() {
}


std::vector<std::weak_ptr<Actor>> RadarLikeDetection::sensedIce() {

	/*SINCE THIS IS A SQUARE WE HAVE TO IMPLEMENT IT DIFFERENTLY*/
	vector<weak_ptr<Actor>> intruders;
	shared_ptr<Actor>source = wp_source.lock();

	if (source && source->isAlive()) {
		intruders = std::move(source->getWorld()->iceCollideWithActor(source));
	}

	return intruders;
}

std::vector<std::weak_ptr<Actor>> RadarLikeDetection::sensedOthers() {
	vector<weak_ptr<Actor>> intruders;
	shared_ptr<Actor>source = wp_source.lock();

	if (source && source->isAlive()) {
		intruders = std::move(source->getWorld()->actorsCollideWithMe(source));
	}
	return intruders;
}

void RadarLikeDetection::behaveBitches() {
	sensedIce();
}


void CollisionDetection::behaveBitches() {
	shared_ptr<Actor>source = wp_source.lock();
	//shared_ptr<Actor> dummy;
	//if (!wp_intruders.empty())
	//	dummy = wp_intruders[0].lock();	//See if this is the actor containers or the ice containers

	if (source) {
		//switch (dummy->type) {
		////case Actor::ice:
		////	if (collideIceHappen()) 	//Make the Actor produce a result because of collision
		////		source->collisionResult->response();
		////	break;
		//default:
			if (collisionHappen())
				source->collisionResult->response();

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

//ActorType { player, npc, worldStatic, hazard, ice, dropByPlayer, collect };
//Affect the both side of the collision
void CollisionDetection::collide(std::weak_ptr<Actor> wp_source, std::weak_ptr<Actor> wp_receiver) {
	shared_ptr<Actor> source = wp_source.lock();
	shared_ptr<Actor> receiver = wp_receiver.lock();

	if ((source && receiver) && (source->isVisible() && receiver->isVisible()) && source != receiver) {	//Only enable collision for things that are shown
		//if (source->collisionResult)
		//	source->collisionResult.reset();
		//if (receiver->collisionResult)
		//	source->collisionResult.reset();

		switch (source->type) {
		case Actor::player:
			switch (receiver->type) {
			case Actor::npc:
				source->collisionResult = make_unique<Destroy>(source, receiver->getStrength());
				break;
			case Actor::worldStatic:
				source->collisionResult = make_unique<Block>(source);
				break;
			case Actor::hazard:
				source->collisionResult = make_unique<Destroy>(source, receiver->getStrength());
				break;
			case Actor::ice:
				receiver->collisionResult = make_unique<Destroy>(receiver, source->getStrength());
				break;
			case Actor::collect:
				receiver->collisionResult = make_unique<Destroy>(receiver, source->getStrength());
				break;
			default:
				break;
			}
			break;
		case Actor::npc:
			switch (receiver->type) {
			case Actor::player:
			case Actor::ice:
			case Actor::worldStatic:
				source->collisionResult = make_unique<Block>(source);
				break;
			case Actor::hazard:
				source->collisionResult = make_unique<Destroy>(source, receiver->getStrength());
			case Actor::dropByPlayer:
				receiver->collisionResult = make_unique<Destroy>(receiver, source->getStrength());
			default:
				break;
			}
			break;
		case Actor::hazard:
			switch (receiver->type) {
			case Actor::worldStatic:
			case Actor::ice:
				//Destroy only myself
				source->collisionResult = make_unique<Destroy>(source, 9999);
				break;
			case Actor::player:
			case Actor::npc:
				//Destroy only the receiver
				receiver->collisionResult = make_unique<Destroy>(receiver, source->getStrength());
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
}

//NOT WORKING IN ITS CURRENT STATE DUE TO FOCUS ON MAKING ICE COLLISION

//Collision is just a radar like detection but only cover a small radius
bool CollisionDetection::collisionHappen() {
/*****************************
**This use an entirely different detection range than the "radar" detection range so it's ok**
Then a collision happen and you should produce a collision result
*****************************/
	shared_ptr<Actor> source = wp_source.lock();
	shared_ptr<Actor> perp;
	if (source) {
		if (!wp_intruders.empty()) {
			for (const auto& wp_perp : wp_intruders) {
				perp = wp_perp.lock();
				if (perp) {
					if (source->isAlive() && perp->isAlive()) {
						//Produce a collision result right here
						collide(source, perp);
					}
				}
			}
			if(source->collisionResult)
				return true;
		}
	}
	return false;
}

void Block::resetBehavior() {
	wp_target.reset();
}

//***This would cause problem if the moveTo() function create animation.****
void Block::response() {
	//Problem: Once they in the block state, they cannot get out
	shared_ptr<Actor>target = wp_target.lock();

	//Move to the same location == Standing still
	if (target) {
		double currentX = 0,
			currentY = 0;
		double targetX,
			targetY;
		targetX = target->getX();
		targetY = target->getY();
		target->getAnimationLocation(currentX, currentY);
		if (targetFacing != target->getDirection()) {	//If they face different direction after being blocked, they can move again
				return;
		}
		else
			target->moveTo(currentX, currentY);	//Move to the current location == staying in place
	}
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

	self.reset();
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

	self.reset();
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

	self.reset();
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
		//shared_ptr<Actor> self = shared_from_this();
		////Create behaviors
		//if (!displayBehavior)
		//	displayBehavior = make_unique<ExistPermanently>();

		////Use behaviors
		//displayBehavior->showYourself();
		
		if(collisionResult)
			collisionResult->response();

		//self.reset();
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
	shared_ptr<Actor>spPawn = pawn.lock();

	if (spPawn && spPawn->isAlive()) {
		if (!spPawn->getWorld()->getKey(key))
			key = INVALID_KEY;

		////Test for different key, remember to comment later
		//key = KEY_PRESS_RIGHT;
		//////////////////////////////

		if (key != INVALID_KEY) {
			switch (key) {
			case KEY_PRESS_DOWN:
				if (spPawn->getDirection() != GraphObject::Direction::down)
					spPawn->setDirection(GraphObject::Direction::down);
				else {
					if (spPawn->getY() - 1 < 0)
						break;
					else
						spPawn->moveTo(spPawn->getX(), spPawn->getY() - 1);
				}
				break;
			case KEY_PRESS_UP:
				if (spPawn->getDirection() != GraphObject::Direction::up)
					spPawn->setDirection(GraphObject::Direction::up);
				else {
					if (spPawn->getY() + 1 > 60)
						break;
					else
						spPawn->moveTo(spPawn->getX(), spPawn->getY() + 1);
				}
				break;
			case KEY_PRESS_RIGHT:
				if (spPawn->getDirection() != GraphObject::Direction::right)
					spPawn->setDirection(GraphObject::Direction::right);
				else {
					if (spPawn->getX() + 1 > 60)
						break;
					else
						spPawn->moveTo(spPawn->getX() + 1, spPawn->getY());
				}
				break;
			case KEY_PRESS_LEFT:
				if (spPawn->getDirection() != GraphObject::Direction::left)
					spPawn->setDirection(GraphObject::Direction::left);
				else {
					if (spPawn->getX() - 1 < 0)
						break;
					else
						spPawn->moveTo(spPawn->getX() - 1, spPawn->getY());
        }
				break;
			default:
				break;
			}
		}
		spPawn.reset();
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

Actor::~Actor() {
	m_sw->playSound(death_sound);
}

void Actor::resetAllBehaviors() {
	if(movementBehavior)
		movementBehavior->resetBehavior();
	if(collisionResult)
		collisionResult->resetBehavior();
	if(detectBehavior)
		detectBehavior->resetBehavior();
	if(collisionDetection)
		collisionDetection->resetBehavior();
}

void shout(Actor::Direction dir) {
	
}

void doSomething() {
}
