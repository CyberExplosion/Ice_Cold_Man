#ifndef GAMEWORLD_H_
#define GAMEWORLD_H_

#include "GameConstants.h"
#include <string>

const int START_PLAYER_LIVES = 3;

class GameController;

class GameWorld
{
public:

	GameWorld(std::string assetDir)
	 : m_lives(START_PLAYER_LIVES), m_score(0), m_level(0),
	   m_controller(nullptr), m_assetDir(assetDir)
	{
	}

	virtual ~GameWorld()
	{
	}
	
	virtual int init() = 0;
	virtual int move() = 0;
	virtual void cleanUp() = 0;

	//You’ll pass in a string to this function that specifies the proper stat values.
//	is used to specify what text is displayed at the top of the
//		game screen, e.g.:
//Lvl: 52 Lives : 3 Hlth : 80 % Wtr : 20 Gld : 3 Oil Left : 2 Sonar : 1 Scr : 321000
	void setGameStatText(std::string text);

	//getKey() can be used to determine if the user has hit a key on the keyboard to move the
	//	player or to fire.This method returns true if the user hit a key during the current tick, and
	//	false otherwise(if the user did not hit any key during this tick).The only argument to this
	//	method is a variable that will be filled in with the key that was pressed by the user(if any
	//		key was pressed)
	//	If the player does hit a key, the argument will be set to one of the
	//constants defined in GameConstants.h :
	bool getKey(int& value);

	//used to play a sound effect when an important event
	//	happens during the game(e.g., a Regular Protester gives up due to being squirted, or the
	//		Iceman picks up a Barrel of oil).You can find constants(e.g.,
	//			SOUND_PROTESTER_GIVE_UP) that describe what noise to make inside of the
	//	GameConstants.h file.
	//Example: GameController::getInstance().playSound(SOUND_PROTESTER_GIVE_UP)
	void playSound(int soundID);

	//can be used to determine the player’s current level number.
	unsigned int getLevel() const
	{
		return m_level;
	}

	//can be used to determine how many lives the player has left
	unsigned int getLives() const
	{
		return m_lives;
	}
	
	//reduces the number of player lives by one.
	void decLives()
	{
		m_lives--;
	}
	
	//increases the number of player lives by one
	void incLives()
    {
		m_lives++;
	}

	//can be used to determine the player’s current score
	unsigned int getScore() const
	{
		return m_score;
	}
	
	//specify how many points the player gets (e.g., 100 points for irritating
	//a Regular Protester to the point where it gives up).This means that the game score is
	//controlled by our GameWorld object – you must not maintain your own score member
	//	variable in your own class(es).
	void increaseScore(unsigned int howMuch)
	{
		m_score += howMuch;
	}
	
	  // The following should be used by only the framework, not the student

	bool isGameOver() const
	{
		return m_lives == 0;
	}

	void advanceToNextLevel() 
	{
		++m_level;
	}
   
	void setController(GameController* controller)
	{
		m_controller = controller;
	}

	std::string assetDirectory() const
	{
		return m_assetDir;
	}
	
private:
	unsigned int	m_lives;
	unsigned int	m_score;
	unsigned int	m_level;
	GameController* m_controller;
	std::string		m_assetDir;
};

#endif // GAMEWORLD_H_
