#include <iostream>
using namespace std;

class Duck {
};

//Inheritance
class CityDuck : public Duck {

};

//Composition
class Park {
	unique_ptr<Duck> rocky = make_unique<Duck>();
};

//Association
class PackOfDuck {
	shared_ptr<CityDuck> nelson = make_shared<CityDuck>();
};