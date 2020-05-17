#include "Actor.h"
#include "StudentWorld.h"

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp

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

void Destroy::response() {
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

void Protesters::takeDmg(int amount) {
}


void Protesters::doSomething() {
}

IDetectionBehavior::IDetectionBehavior(bool radarEnable) {
}

std::unique_ptr<IActorResponse> Characters::collide(std::unique_ptr<Actor> receiver) {
	return std::unique_ptr<IActorResponse>();
}
