#include "Pacman.h"

#include <sstream>
#include <time.h>
#include <iostream>

Pacman::Pacman(int argc, char* argv[]) : Game(argc, argv), _cPacmanFrameTime(150), _cPacmanSpeed(0.1f)
{
	// Seed "rand()" with time
	srand(time(NULL));

	_pacman = new Player();

	// Initialise multiple munchie pointers
	int i;
	for (i = 0; i < MUNCHIECOUNT; i++) {
		_munchie[i] = new Enemy;
		_munchie[i]->currentFrameTime = 0;
		_munchie[i]->frameCount = rand() % 1;
		_munchie[i]->frameTime = rand() % 500 + 50;
	}

	// Initialise ghost character
	for (i = 0; i < GHOSTCOUNT; i++) {
		_ghosts[i] = new movingEnemy;
		_ghosts[i]->direction = 0;
		_ghosts[i]->speed = 0.2f;
	}

	// Initialise member variables
	_cherry = new Enemy();

	_paused = false;
	_pKeyDown = false;
	_rKeyDown = false;

	_start = false;

	_pacman->direction = 0;
	_pacman->currentFrameTime = 0;
	_pacman->frame = 0;
	_pacman->speedMultiplier = 1.0f;
	_pacman->dead = false;
	_pacman->score = 0;

	_pop = new SoundEffect();

	//Initialise important Game aspects
	Audio::Initialise();
	Graphics::Initialise(argc, argv, this, 1024, 768, false, 25, 25, "Pacman", 60);
	Input::Initialise();

	// Start the Game Loop - This calls Update and Draw in game loop
	Graphics::StartGameLoop();
}

Pacman::~Pacman()
{
	// Clean up pointers within the Pacman structure
	delete _pacman->texture;
	delete _pacman->sourceRect;

	delete _cherry->texture;
	delete _cherry->sourceRect;

	delete _munchie[0]->texture;

	int nCount = 0;
	for (nCount = 0; nCount < MUNCHIECOUNT; nCount++) {
		delete _munchie[nCount]->position;
		delete _munchie[nCount]->sourceRect;
		delete _munchie[nCount];
	}

	for (nCount = 0; nCount < GHOSTCOUNT; nCount++) {
		delete _ghosts[nCount]->texture;
		delete _ghosts[nCount]->position;
		delete _ghosts[nCount]->sourceRect;
	}

	delete[] _munchie;
	delete[] _ghosts;

	// Clean up the Pacman structure pointer
	delete _pacman;
	delete _munchie;
	delete _cherry;
	delete _ghosts;
	delete _pop;
}

void Pacman::LoadContent()
{
	// Load sounds
	_pop->Load("Sounds/pop.wav");

	// Set start parameters
	_startBackground = new Texture2D();
	_startBackground->Load("Textures/Transparency.png", false);
	_startRectangle = new Rect(0.0f, 0.0f, Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
	_startStringPosition = new Vector2(Graphics::GetViewportWidth() / 2.0f, Graphics::GetViewportHeight() / 2.0f);

	// Load Pacman
	_pacman->texture = new Texture2D();
	_pacman->texture->Load("Textures/Pacman.png", false);
	_pacman->position = new Vector2(350.0f, 350.0f);
	_pacman->sourceRect = new Rect(0.0f, 0.0f, 32, 32);

	// Load Munchie
	Texture2D* munchieTex = new Texture2D();
	munchieTex->Load("Textures/Munchie.png", false);
	for (int i = 0; i < MUNCHIECOUNT; i++) {
		_munchie[i]->texture = munchieTex;
		_munchie[i]->sourceRect = new Rect(0.0f, 0.0f, 12, 12);
		_munchie[i]->position = new Vector2((rand() % (Graphics::GetViewportWidth() - _munchie[i]->sourceRect->Width)), (rand() % (Graphics::GetViewportHeight() - _munchie[i]->sourceRect->Height)));
	}
	// Load Cherry
	_cherry->texture = new Texture2D();
	_cherry->texture->Load("Textures/Cherry.png", false);
	_cherry->sourceRect = new Rect(0.0f, 0.0f, 12, 12);
	_cherry->position = new Vector2((rand() % (Graphics::GetViewportWidth() - _cherry->sourceRect->Width)), (rand() % (Graphics::GetViewportHeight() - _cherry->sourceRect->Height)));

	// Load Ghost
	Texture2D* ghostTex = new Texture2D();
	ghostTex->Load("Textures/GhostBlue.png", false);
	for (int i = 0; i < GHOSTCOUNT; i++) {
		_ghosts[i]->texture = ghostTex;
		_ghosts[i]->sourceRect = new Rect(0.0f, 0.0f, 20, 20);
		_ghosts[i]->position = new Vector2((rand() % (Graphics::GetViewportWidth() - _ghosts[i]->sourceRect->Width)), (rand() % (Graphics::GetViewportHeight() - _ghosts[i]->sourceRect->Height)));
	}
	// Set string position
	_stringPosition = new Vector2(10.0f, 25.0f);

	// Set Menu Parameters
	_menuBackground = new Texture2D();
	_menuBackground->Load("Textures/Transparency.png", false);
	_menuRectangle = new Rect(0.0f, 0.0f, Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
	_menuStringPosition = new Vector2(Graphics::GetViewportWidth() / 2.5f, Graphics::GetViewportHeight() / 2.0f);
}

void Pacman::Update(int elapsedTime)
{
	// Gets the current state of the keyboard
	Input::KeyboardState* keyboardState = Input::Keyboard::GetState();
	Input::MouseState* mouseState = Input::Mouse::GetState();

	checkStart(keyboardState, Input::Keys::SPACE);

	// Enables Pacman to move and pause the game
	if (_start) {
		if (!_pacman->dead) {
			checkPaused(keyboardState, Input::Keys::P);

			if (!_paused) {
				input(elapsedTime, keyboardState, mouseState);
				checkViewportCollision();
				for (int i = 0; i < MUNCHIECOUNT; i++)
					updateMunchie(_munchie[i], elapsedTime);
				updatePacman(elapsedTime);
				for (int i = 0; i < GHOSTCOUNT; i++)
					updateGhost(_ghosts[i], elapsedTime);
				checkGhostCollisions();
				checkMunchieCollisions();
			}
		}
	}
	if (!Audio::IsInitialised())
		std::cout << "Audio is not initialised" << std::endl;

	if (!_pop->IsLoaded())
		std::cout << "_pop member sound effect has not loaded" << std::endl;
}

void Pacman::Draw(int elapsedTime)
{
	// Allows us to easily create a string
	std::stringstream stream;
	stream << "Pacman X: " << _pacman->position->X << " Y: " << _pacman->position->Y;
	stream << "\nUser score: " << _pacman->score;

	SpriteBatch::BeginDraw(); // Starts Drawing
	// Draws Pacman if not dead
	if (!_pacman->dead) {
		SpriteBatch::Draw(_pacman->texture, _pacman->position, _pacman->sourceRect); 
	}
	// Draw ghosts
	for (int i = 0; i < GHOSTCOUNT; i++) {
		SpriteBatch::Draw(_ghosts[i]->texture, _ghosts[i]->position, _ghosts[i]->sourceRect);
	}

	// Draws all Munchies
	for (int i = 0; i < MUNCHIECOUNT; i++)
		SpriteBatch::Draw(_munchie[i]->texture, _munchie[i]->position, _munchie[i]->sourceRect); 
	// Draws Cherry
	SpriteBatch::Draw(_cherry->texture, _cherry->position, _cherry->sourceRect);

	// Creates start menu
	if (!_start) {
		std::stringstream menuStream;
		menuStream << "PRESS 'SPACE' TO PLAY!";

		SpriteBatch::Draw(_startBackground, _startRectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menuStringPosition, Color::Yellow);

	}
	
	// Draws String
	SpriteBatch::DrawString(stream.str().c_str(), _stringPosition, Color::Green);

	// Creates pause menu
	if (_paused) {
		std::stringstream menuStream;
		menuStream << "PAUSED!";

		SpriteBatch::Draw(_menuBackground, _menuRectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menuStringPosition, Color::Yellow);
	}
	// Draws death screen
	if (_pacman->dead) {
		std::stringstream menuStream;
		menuStream << "YOU ARE DEAD!" << endl << "Your score: " << _pacman->score;

		SpriteBatch::Draw(_menuBackground, _menuRectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menuStringPosition, Color::Red);
	}

	SpriteBatch::EndDraw(); // Ends Drawing
}

void Pacman::input(int elapsedTime, Input::KeyboardState* state, Input::MouseState* mouseState) {

	float pacmanSpeed = _cPacmanSpeed * elapsedTime * _pacman->speedMultiplier;
	// Pacmans movement
	// Moves up when W is pressed
	if (state->IsKeyDown(Input::Keys::W)) {
		_pacman->position->Y -= pacmanSpeed;
		_pacman->direction = 3;
	} // Moves down when S is pressed
	else if (state->IsKeyDown(Input::Keys::S)) {
		_pacman->position->Y += pacmanSpeed;
		_pacman->direction = 1;
	} // Moves left when A is pressed
	else if (state->IsKeyDown(Input::Keys::A)) {
		_pacman->position->X -= pacmanSpeed;
		_pacman->direction = 2;
	} // Moves right when D is pressed
	else if (state->IsKeyDown(Input::Keys::D)) {
		_pacman->position->X += pacmanSpeed;
		_pacman->direction = 0;
	}

	// Speed multiplier
	if (state->IsKeyDown(Input::Keys::LEFTSHIFT)) {
		// Apply multiplier
		_pacman->speedMultiplier = 1.5f;
	}
	else {
		// Reset multiplier
		_pacman->speedMultiplier = 0.9f;
	}

	// Randomly reposition Cherry
	if (state->IsKeyDown(Input::Keys::R) && !_rKeyDown && _cherry->position->X != -100) {
		_cherry->position = new Vector2((rand() % (Graphics::GetViewportWidth() - _cherry->sourceRect->Width)), (rand() % (Graphics::GetViewportHeight() - _cherry->sourceRect->Height)));
		_rKeyDown = true;
	}
	// Only moves cherry once 
	if (state->IsKeyUp(Input::Keys::R))
		_rKeyDown = false;

}

void Pacman::checkPaused(Input::KeyboardState* state, Input::Keys pauseKey) {

	if (state->IsKeyDown(Input::Keys::P) && !_pKeyDown) {
		_paused = !_paused;
		_pKeyDown = true;
	}
	// Makes sure that it isn't updated every frame when holding P key
	if (state->IsKeyUp(Input::Keys::P))
		_pKeyDown = false;
}

void Pacman::checkViewportCollision() {

	// Checks if Pacman is trying to disappear
	if (_pacman->position->X > Graphics::GetViewportWidth()) //1024 is game width
	{
		// Pacman hit right wall - wraps to left wall
		_pacman->position->X = -_pacman->sourceRect->Width;
	}

	if (_pacman->position->X + _pacman->sourceRect->Width < 0)
	{
		// Pacman hit left wall - wraps to right wall
		_pacman->position->X = 1024;
	}

	if (_pacman->position->Y + _pacman->sourceRect->Height < 0)
	{
		// Pacman hit top wall - wraps to bottom wall
		_pacman->position->Y = 768;
	}

	if (_pacman->position->Y > Graphics::GetViewportHeight())
	{
		// Pacman hit bottom wall - wraps to top wall
		_pacman->position->Y = -_pacman->sourceRect->Height;
	}
}

void Pacman::updatePacman(int elapsedTime) {

	_pacman->currentFrameTime += elapsedTime;
	if (_pacman->currentFrameTime > _cPacmanFrameTime) {
		_pacman->frame++;

		if (_pacman->frame >= 2)
			_pacman->frame = 0;

		_pacman->currentFrameTime = 0;
	}
	_pacman->sourceRect->Y = _pacman->sourceRect->Height * _pacman->direction;
	_pacman->sourceRect->X = _pacman->sourceRect->Width * _pacman->frame;
}

void Pacman::updateMunchie(Enemy*, int elapsedTime) {
	for (int i = 0; i < MUNCHIECOUNT; i++) {
		_munchie[i]->currentFrameTime += elapsedTime;
		if (_munchie[i]->currentFrameTime > _munchie[i]->frameTime) {
			_munchie[i]->frameCount++;

			if (_munchie[i]->frameCount >= 2)
				_munchie[i]->frameCount = 0;

			_munchie[i]->currentFrameTime = 0;
		}
		_munchie[i]->sourceRect->X = _munchie[i]->sourceRect->Width * _munchie[i]->frameCount;
		_cherry->sourceRect->X = _cherry->sourceRect->Width * _munchie[i]->frameCount;
	}
}

void Pacman::checkStart(Input::KeyboardState* state, Input::Keys startKey) {

	if (state->IsKeyDown(Input::Keys::SPACE)) {
		_start = true; // Starts the game
	}
}

void Pacman::updateGhost(movingEnemy* ghost, int elapsedTime) {
	if (ghost->direction == 0) { 
		ghost->position->X += ghost->speed * elapsedTime; // Moves right
	}
	else if (ghost->direction == 1) {
		ghost->position->X -= ghost->speed * elapsedTime; // Moves left
	}
	if (ghost->position->X + ghost->sourceRect->Width >= Graphics::GetViewportWidth())  { // Hits right edge
		ghost->direction = 1; // Change direction
	} 
	else if (ghost->position->X <= 0) { // Hits left edge
		ghost->direction = 0; // Change direction
	}
	
	for (int i = 0; i < GHOSTCOUNT; i++) {
		_ghosts[i]->sourceRect->X = _ghosts[i]->sourceRect->Width * _ghosts[i]->direction;
	}
}

void Pacman::checkGhostCollisions() {
	// Local variables
	int i = 0;
	int bottom1 = _pacman->position->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _pacman->position->X;
	int left2 = 0;
	int right1 = _pacman->position->X + _pacman->sourceRect->Width;
	int right2 = 0;
	int top1 = _pacman->position->Y;
	int top2 = 0;

	for (i = 0; i < GHOSTCOUNT; i++) {
		// Populate variables with Ghost data
		bottom2 = _ghosts[i]->position->Y + _ghosts[i]->sourceRect->Height;
		left2 = _ghosts[i]->position->X;
		right2 = _ghosts[i]->position->X + _ghosts[i]->sourceRect->Width;
		top2 = _ghosts[i]->position->Y;

		if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2)) {
			_pacman->dead = true;
			i = GHOSTCOUNT;
		}
	}
}

void Pacman::checkMunchieCollisions() {
	// Local variables
	int i = 0;
	int bottom1 = _pacman->position->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _pacman->position->X;
	int left2 = 0;
	int right1 = _pacman->position->X + _pacman->sourceRect->Width;
	int right2 = 0;
	int top1 = _pacman->position->Y;
	int top2 = 0;

	for (i = 0; i < MUNCHIECOUNT; i++) {
		// Populate variables with Munchie data
		bottom2 = _munchie[i]->position->Y + _munchie[i]->sourceRect->Height;
		left2 = _munchie[i]->position->X;
		right2 = _munchie[i]->position->X + _munchie[i]->sourceRect->Width;
		top2 = _munchie[i]->position->Y;

		if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2)) {
			Audio::Play(_pop);
			_pacman->score += 10;
			_munchie[i]->position->X = -100;
			_munchie[i]->position->Y = -100;
		}
	}

	// Populating with Cherry data
	bottom2 = _cherry->position->Y + _cherry->sourceRect->Height;
	left2 = _cherry->position->X;
	right2 = _cherry->position->X + _cherry->sourceRect->Width;
	top2 = _cherry->position->Y;

	if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2)) {
		Audio::Play(_pop);
		_pacman->score += 100;
		_cherry->position->X = -100;
		_cherry->position->Y = -100;
	}
}
