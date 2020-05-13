#ifndef ACTOR_H_
#define ACTOR_H_

#include <iostream>
#include "GraphObject.h"

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

//Movement Strategy
class IMovementBehavior{};

class FreeMovement : public IMovementBehavior{};

class NoMovement : public IMovementBehavior{};

class FallMovement : public IMovementBehavior{};

class ControlledMovement : public IMovementBehavior{};

class PursuingMovement : public IMovementBehavior{};

//////////////////////////

//Collision Strategy
class ICollisionBehavior{};

class NoBehavior : public ICollisionBehavior{};

class BlockBehavior : public ICollisionBehavior{};

class DestructibleBehavior : public ICollisionBehavior{};

/////////////////////////

class Actor : public GraphObject{
private:
	std::unique_ptr<IMovementBehavior> movementBehavior;
	std::unique_ptr<ICollisionBehavior> collisionBehavior;
};

//People
class Characters : public Actor {};

class IceMan : public Characters{};

class Protesters : public Characters{};

class HardcoreProtesters: public Characters{};

//Stuffs
class Inanimated : public Actor{};

class Collectable : public Actor{};

class OilBarrels : public Collectable{};

class GoldNuggets : public Collectable{};

class SonarKit : public Collectable{};

class Water : public Collectable{};

class Hazard : public Actor{};

class WaterSquirt : public Hazard{};

class Boulder : public Hazard{};

class Ice : public Inanimated{};

#endif // ACTOR_H_
