#include "Actor.h"
#include "GameController.h"
#include "StudentWorld.h"
#include "GameConstants.h"
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

bool IceMan::shootSquirt() {
	if (getWorld()->createSquirt())
		return true;
	else
		return false;
}


////Testing purposes
//int counter = 2;
////////////////////

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
	weak_ptr<Actor> mySelf = getWorld()->getPlayer();
	
	//shared_ptr<Actor>temp = mySelf.lock();

	int keyPressed;

	if (!getWorld()->getKey(keyPressed))
		keyPressed = INVALID_KEY;

	///////////Test
	//if (counter == 2) {
	//	keyPressed = KEY_PRESS_SPACE;
	//	counter--;
	//}
	//else
	//	keyPressed = KEY_PRESS_RIGHT;
	/////////////////

	if (!movementBehavior)
		movementBehavior = std::make_unique<ControlledMovement>(mySelf, keyPressed);
	else
		movementBehavior->setKey(keyPressed);


	if (!displayBehavior)
		displayBehavior = make_unique<ExistPermanently>();
	
	useGoodies(keyPressed);
	displayBehavior->showYourself();
	movementBehavior->moveThatAss();
	//The collision need to be executed AFTER the movement for this to works
	//Reset cycle of collision result and detection
	collisionResult.reset();
	collisionDetection = make_unique<CollisionDetection>(mySelf, this->getCollisionRange());
	collisionDetection->behaveBitches();	//If there's a detection then a response is already made automatically
	///////////////
}

//Functions return true if the user using goodies instead of moving
bool IceMan::useGoodies(int key) {
		switch (key) {
		case KEY_PRESS_SPACE:
			shootSquirt();
			return true;
		case KEY_PRESS_TAB:
			getWorld()->dropGold();
			return true;
		case KEY_PRESS_ESCAPE:
			return true;
		case 'Z':
		case 'z':
			break;
		default:
			break;
	}
	return false;
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

//Function works on recording the intruders and ice surround the source. Call this to update the intruders' list
void RadarLikeDetection::checkSurrounding(std::weak_ptr<Actor> t_source) {
	std::shared_ptr<Actor> temp = t_source.lock();
	if (temp) {
			wp_intruders = std::move(sensedIce());
			auto temp = std::move(sensedOthers());
			wp_intruders.insert(end(wp_intruders), begin(temp), end(temp));	//Concatenate the vectors cause we have more intruders
	}
}


void RadarLikeDetection::behaveBitches() {
	sensedIce();
}


void CollisionDetection::behaveBitches() {
	shared_ptr<Actor>source = wp_source.lock();
	if (source) {
		checkSurrounding(source);	//Update the current location of intruders
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
				//Destroy both the hazard and the player
				source->collisionResult = make_unique<Destroy>(source, receiver->getStrength());
				receiver->collisionResult = make_unique<Destroy>(receiver, 9999);
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
				break;
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
		}
		if (source->collisionResult)
			return true;
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
		if (targetFacing != target->getDirection())	//If they face different direction after being blocked, they can move again
				return;
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
	existBehavior->showYourself();
	collisionDetection->behaveBitches();
	detectBehavior->behaveBitches();

	self.reset();
}

bool GoldNuggets::tempTimeEnd() {
	if(tempTime == 0)
		return false;
	return true;
}

void GoldNuggets::doSomething() {
	if (!isAlive()) {
		cout << "Here" << endl;
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
	if (!tempTimeEnd()) {
		if (self->isAlive()) {
			self->dmgActor(9999);
		}
	}
	//Use the behavior
	tempTime--;
	displayBehavior->showYourself();
	detectBehavior->behaveBitches();
	collisionDetection->behaveBitches();

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

	self.reset();
}


void Squirt::increaseAmmo(int amount) {
}

void Squirt::shoot() {
}

void Squirt::doSomething() {
	if (!isAlive()) {
		resetAllBehaviors();
		return;
	}

	shared_ptr<Actor> self = shared_from_this();

	if (!displayBehavior)
		displayBehavior = make_unique<ExistTemporary>();
	if (!detectBehavior)
		detectBehavior = make_unique<RadarLikeDetection>(self, self->getDetectRange());
	if (!collisionDetection)
		collisionDetection = make_unique<CollisionDetection>(self, self->getCollisionRange());
	if (!movementBehavior)
		movementBehavior = make_unique<SquirtMovement>(self);

	//Always execute movement first before collision detection so that if collision is <Block> it can catch the movement
	displayBehavior->showYourself();
	detectBehavior->behaveBitches();
	movementBehavior->moveThatAss();
	collisionDetection->behaveBitches();

	self.reset();
}

void Boulder::fall() {
	int y = getY();
	fallTimer(); // According to documentation there needs to be time between when the player 
				 // destroys the ice below the boulder and when it falls. 
				 // This function serves as a counter for that purpose.

	if (y == 0) {
		changeActorType(ActorType::hazard);
		return;
	}

	else if (t == 0) 
		moveTo(getX(), getY() - 1);
}

void Boulder::fallTimer() {
	if (t != 0)
		t--;
	else
		return;
	
	return;
}

void Boulder::doSomething() {

	///*
	//	Boulder falls but we need to make it disappear ! ! !
	//	
	//*/
	//if (!isAlive()) // If the boulder already fell, there's nothing we need to do.
	//	return;

	//shared_ptr<Actor>mySelf = shared_from_this();

	//if (!displayBehavior)
	//	displayBehavior = make_unique<ExistPermanently>();

	//// Reset cycle of collision result and detection
	//collisionResult.reset();
	//collisionDetection = make_unique<CollisionDetection>(mySelf, this->getCollisionRange());
	//collisionDetection->behaveBitches();	//If there's a detection then a response is already made automatically
	//
	//displayBehavior->showYourself();
	//movementBehavior->moveThatAss();

	// Checks to see if there's ice below the boulder, if there isn't it will return true.

	if (!isAlive()) {
		resetAllBehaviors();
		return;
	}
	
	shared_ptr<Actor> self = shared_from_this();

	bool isFalling = getWorld()->boulderFall(getX(), getY());

	// Initiates the falling.
	if (isFalling) {
		fall();
		changeActorType(ActorType::hazard);
	}

	if (!collisionDetection)
		collisionDetection = make_unique<CollisionDetection>(self, self->getCollisionRange());


	collisionDetection->behaveBitches();

	self.reset();
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
	timeTillDeath = 20;
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

	//if (spPawn && spPawn->isAlive()) {
	//	if (!spPawn->getWorld()->getKey(key))
	//		key = INVALID_KEY;

		if (key != INVALID_KEY) {
			switch (key) {
			case KEY_PRESS_DOWN:
				if (spPawn->getDirection() != GraphObject::Direction::down)
					spPawn->setDirection(GraphObject::Direction::down);
				else {
					if (spPawn->getY() - 1 < 0)
						break;
					else {
						//Testing purposes
						

						spPawn->moveTo(spPawn->getX(), spPawn->getY() - 1);
					}
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

void PursuingMovement::moveThatAss() {
}

void PursuingMovement::resetBehavior() {
}

void SquirtMovement::moveThatAss() {
	shared_ptr<Actor> pawn = squirt.lock();
	
	if (pawn && pawn->isAlive()) {
		if (travelDist <= 0) {	//The distance it can travel expired
			pawn->dmgActor(9999);
		}

		GraphObject::Direction dir = pawn->getDirection();
		int localX = pawn->getX();
		int localY = pawn->getY();

		switch (dir) {
		case GraphObject::up:
			localY += 1;
			break;
		case GraphObject::down:
			localY -= 1;
			break;
		case GraphObject::left:
			localX -= 1;
			break;
		case GraphObject::right:
			localX += 1;
			break;
		default:
			break;
		}

		if (localY > ROW_NUM || localX > COL_NUM)	//Out of bounds
			return;
		else {
			pawn->moveTo(localX, localY);	//Update new location

			//pawn->collisionDetection.reset();
			//pawn->collisionDetection = make_unique<CollisionDetection>(pawn, pawn->getCollisionRange());
			//pawn->collisionDetection->behaveBitches();	//Force check collision after update the new location
			--travelDist;	//Decrement the distance it can still travel
		}
	}
	pawn.reset();
	return;
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
