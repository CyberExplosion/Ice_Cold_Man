#include "Actor.h"
#include "StudentWorld.h"
using namespace std;

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp

unique_ptr<IActorResponse> Actor::collide (shared_ptr<Actor>& receiver) {

	if (this->type == player) {
		switch (receiver->type) {
		case Actor::worldStatic: {
			unique_ptr<Block> re = make_unique<Block>(this);
			return re;
			}
		case Actor::hazard: {
			unique_ptr<Destroy> re = make_unique<Destroy>(this, receiver->getStrength());
			return re;
		}
		default:
			break;
		}
	}
	if (this->type == npc) {
		switch (receiver->type) {
		case Actor::hazard:{
			unique_ptr<Destroy> re = make_unique<Destroy>(this, receiver->getStrength());
			return re;
		}
		case Actor::worldStatic:
		case Actor::ice: {
			unique_ptr<Block> re = make_unique<Block>(this);
			return re;
		}
		default:
			break;
		}
	}
	//World static doesn't count because it's never move or destroy

	if (this->type == hazard) {
		switch (receiver->type) {
		case Actor::worldStatic:
		case Actor::npc:
		case Actor::player:
		case Actor::ice:{
			unique_ptr<Destroy> re = make_unique<Destroy>(this, receiver->getStrength());
			return re;
		}
		default:
			break;
		}
	}
	if (this->type == ice) {
		switch (receiver->type) {
		case Actor::player:{
			unique_ptr<Destroy> re = make_unique<Destroy>(this, receiver->getStrength());
			return re;
		}
		default:
			break;
		}
	}
	if (this->type == dropByPlayer) {
		switch (receiver->type) {
		case Actor::npc:{
			unique_ptr<Destroy> re = make_unique<Destroy>(this, receiver->getStrength());
			return re;
		}
		default:
			break;
		}
	}
	if (this->type == collect) {
		switch (receiver->type) {
		case Actor::player:{
			unique_ptr<Destroy> re = make_unique<Destroy>(this, receiver->getStrength());
			return re;
		}
		default:
			break;
		}
	}
	return nullptr;
}


void Actor::doSomething() {
}

void Boulder::falling() {
}

bool Boulder::checkIceBelow() {
	return false;
}

void Boulder::doSomething() {
}


int Squirt::getAmmo() {
	return 0;
}

void Squirt::increaseAmmo(int amount) {
}

void Squirt::shoot() {
}

void Squirt::doSomething() {
}

void Ice::doSomething() {
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

void ExistTemporary::killMeNow() {
}

void Collectable::showSelf() {
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

void Block::response() {
}

//Destroy an object or deal damage to characters
void Destroy::response() {
	if(target->isAlive())
		target->dmgActor(dmgTaken);
}

void FreeMovement::moveThatAss() {
}

void FallMovement::moveThatAss() {
}

void ControlledMovement::moveThatAss() {
}

void PursuingMovement::moveThatAss() {
}

DetectPlayerBehavior::DetectPlayerBehavior(bool radarEnable) {
}

void DetectPlayerBehavior::behaveBitches() {
}

void DetectInanimateBehavior::behaveBitches() {
}

void IceMan::doSomething() {
}

void Protesters::shout(Direction dir) {
}



void Protesters::doSomething() {
}

IDetectionBehavior::IDetectionBehavior(bool radarEnable) {
}

void SquirtMovement::moveThatAss() {
}
