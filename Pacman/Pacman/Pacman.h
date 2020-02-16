#pragma once
#define MUNCHIECOUNT 50
#define GHOSTCOUNT 5

// If Windows and not in Debug, this will run without a console window
// You can use this to output information when debugging using cout or cerr
#ifdef WIN32 
	#ifndef _DEBUG
		#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
	#endif
#endif

// Just need to include main header file
#include "S2D/S2D.h"

// Reduces the amount of typing by including all classes in S2D namespace
using namespace S2D;

// Structure Definition 
struct Player {
	bool dead;
	float speedMultiplier;
	int currentFrameTime;
	int direction;
	int frame;
	int score;
	Rect* sourceRect;
	Texture2D* texture;
	Vector2* position;
};

struct Enemy {
	int frameCount;
	Rect* sourceRect;
	int currentFrameTime;
	Vector2* position;
	Texture2D* texture;
	int frameTime;
};

struct movingEnemy {
	Vector2* position;
	Texture2D* texture;
	Rect* sourceRect;
	int direction;
	float speed;
};

// Declares the Pacman class which inherits from the Game class.
// This allows us to overload the Game class methods to help us
// load content, draw and update our game.
class Pacman : public Game
{
private:

	void input(int elapsedTime, Input::KeyboardState* state, Input::MouseState* mouseState);
	void checkPaused(Input::KeyboardState* state, Input::Keys pauseKey);
	void checkStart(Input::KeyboardState* state, Input::Keys startKey);
	void checkViewportCollision();
	void updatePacman(int elapsedTime);
	void updateMunchie(Enemy*, int elapsedTime);
	void checkGhostCollisions();
	void updateGhost(movingEnemy*, int elapsedTime);
	void checkMunchieCollisions();

	Player* _pacman;
	Enemy* _munchie[MUNCHIECOUNT];
	Enemy* _cherry;
	movingEnemy* _ghosts[GHOSTCOUNT];

	// Data for Menu
	Texture2D* _menuBackground;
	Rect* _menuRectangle;
	Vector2* _menuStringPosition;
	bool _paused;
	bool _pKeyDown;

	bool _rKeyDown;

	// Data for start
	bool _start;
	Rect* _startRectangle;
	Vector2* _startStringPosition;
	Texture2D* _startBackground;

	// Position for String
	Vector2* _stringPosition;
	
	// Constant data for Game Variables
	const float _cPacmanSpeed;
	const int _cPacmanFrameTime;
	
	// Data for sound effects
	SoundEffect* _pop;

public:
	/// <summary> Constructs the Pacman class. </summary>
	Pacman(int argc, char* argv[]);

	/// <summary> Destroys any data associated with Pacman class. </summary>
	virtual ~Pacman();

	/// <summary> All content should be loaded in this method. </summary>
	void virtual LoadContent();

	/// <summary> Called every frame - update game logic here. </summary>
	void virtual Update(int elapsedTime);

	/// <summary> Called every frame - draw game here. </summary>
	void virtual Draw(int elapsedTime);
};