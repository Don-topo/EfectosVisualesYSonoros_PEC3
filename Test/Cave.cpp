#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

//Screen dimension constants
const int SCREEN_WIDTH = 854;
const int SCREEN_HEIGHT = 480;

//GLOBAL
SDL_Renderer* gMyRenderer = NULL;

#define PI 3.14159265

// MAP
int environment[10][10] {
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{-1, 0, 0, 0, -1, 0, 0, 0, 0, -1},
	{-1, 0, 0, 0, -1, 0, 0, 0, 0, -1},
	{-1, 0, -1, 0, -1, 0, 0, 0, 0, -1},
	{-1, 0, 0, 0, 0, 0, 0, -1, 0, -1},
	{-1, 0, 0, 0, 0, 0, 0, -1, 0, -1},
	{-1, 0, 0, -1, 0, 0, 0, -1, 0, -1},
	{-1, 0, 0, -1, 0, 0, 0, -1, 0, -1},
	{-1, 0, 0, -1, 0, 0, 0, -1, 0, -1},
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
};

int playerPositionX = 8, playerPositionY = 2; 
int playerDirection = 0;
int monsterPositionX = 5, monsterPositionY = 4;
int finishPositionX = 8, finishPositionY = 8;
bool playerCanMove = true;
int distanceFactor = 20;
SDL_Window* gWindow = NULL;
Mix_Chunk* loadSound;

//Audios
vector<Mix_Chunk*> audios;
int waterFall, victory, stepMonster, stepHuman, wallHit, snoring, gameOver, death;


enum Direction
{
	Up,
	Left,
	Right,
	Down
};


void PerformMovement(int posX, int posY);
bool GameOver();
bool CompleteGame();
void MovePlayer();
void MoveMonster();
void PrintInfo();
void Initializations();
void LoopAudio();
int CalculateDistanceFromPlayer(int posX, int posY);
int* CalculatePanning(int posX, int posY);


int main(int argc, char* args[]) {

	bool exit = false;
	Initializations();


	while (!exit) {
		SDL_Event test_event;
		SDL_Scancode tecla;

		LoopAudio();

		while (SDL_PollEvent(&test_event)) {
			switch (test_event.type) {
			case SDL_KEYDOWN:
				tecla = test_event.key.keysym.scancode;
				if (tecla == SDL_SCANCODE_ESCAPE) {
					exit = true;
				}
				if (playerCanMove) {
					// Key Changes
					//BACK
					if (tecla == SDL_SCANCODE_W || tecla == SDL_SCANCODE_UP) {
						// check move
						MovePlayer();
					}

					if (tecla == SDL_SCANCODE_A || tecla == SDL_SCANCODE_LEFT) {
						if (playerDirection == Direction::Up)
						{
							playerDirection = Direction::Left;
						}
						else if (playerDirection == Direction::Left)
						{
							playerDirection = Direction::Down;
						}
						else if (playerDirection == Direction::Down)
						{
							playerDirection = Direction::Right;
						}
						else
						{
							playerDirection = Direction::Up;
						}
					}
					if (tecla == SDL_SCANCODE_D || tecla == SDL_SCANCODE_RIGHT) {
						if (playerDirection == Direction::Up) {
							playerDirection = Direction::Right;
						}
						else if (playerDirection == Direction::Left)
						{
							playerDirection = Direction::Up;
						}
						else if (playerDirection == Direction::Down)
						{
							playerDirection = Direction::Left;
						}
						else
						{
							playerDirection = Direction::Down;
						}
					}
				}				
				PrintInfo();
				break;
			case SDL_QUIT:
				exit = true;
				break;
			}
		}
	}


	//Destroy window
	SDL_DestroyRenderer(gMyRenderer);
	gMyRenderer = NULL;
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	// Free audio
	for (int i = 0; i < audios.size(); ++i)
	{
		Mix_FreeChunk(audios[i]);
	}
	//Quit SDL subsystems
	Mix_CloseAudio();
	SDL_Quit();

	return 0;
}

void PerformMovement(int posX, int posY) {
	int nextSpace = environment[playerPositionX + posX][playerPositionY + posY];
	switch (nextSpace)
	{
	case -1:
		// Hit wall sound
		Mix_HaltChannel(wallHit);
		Mix_PlayChannel(wallHit, audios[wallHit], 0);	
		// Monster stop snoring		
		Mix_HaltChannel(snoring);
		// Move monster
		MoveMonster();
		break;
	case 0:
		// Move player
		playerPositionX += posX;
		playerPositionY += posY;
		// Step sound
		Mix_HaltChannel(stepHuman);
		Mix_PlayChannel(stepHuman, audios[stepHuman], 0);
		// Update sound positions
		Mix_SetDistance(snoring, (Uint8)CalculateDistanceFromPlayer(monsterPositionX, monsterPositionY));
		Mix_SetDistance(waterFall, (Uint8)CalculateDistanceFromPlayer(finishPositionX, finishPositionY));
		break;
	}

	// Exit
	if (CompleteGame()) {
		// Play Victory sound
		Mix_HaltChannel(victory);
		Mix_PlayChannel(victory, audios[victory], 0);
		Mix_Pause(waterFall);
		Mix_Pause(snoring);
		cout << "Game completed" << endl;
	}
	// Monster
	if (GameOver()) {
		// Play Gameover sound
		Mix_PlayChannel(death, audios[death], 0);
		SDL_Delay(2500);
		Mix_HaltChannel(gameOver);
		Mix_PlayChannel(gameOver, audios[gameOver], 0);
		Mix_Pause(waterFall);
		Mix_Pause(snoring);
		cout << "Game Over" << endl;
	}
}

bool GameOver() {
	bool isGameOver = (playerPositionY == monsterPositionY && playerPositionX == monsterPositionX);
	if (isGameOver) playerCanMove = false;
	return (isGameOver);
}

bool CompleteGame() {
	bool isGameCompleted = (playerPositionY == finishPositionX && playerPositionX == finishPositionY);
	if (isGameCompleted) playerCanMove = false;
	return (isGameCompleted);
}


void MovePlayer() {

	// Obtain nextPosition
	int nextPositionX = 0, nextPositionY = 0;

	switch (playerDirection)
	{
	case Direction::Up:
		nextPositionX = -1;
		nextPositionY = 0;
		break;
	case Direction::Left:
		nextPositionX = 0;
		nextPositionY = -1;
		break;
	case Direction::Right:
		nextPositionX = 0;
		nextPositionY = 1;
		break;
	case Direction::Down:
		nextPositionX = 1;
		nextPositionY = 0;
		break;		
	}
	PerformMovement(nextPositionX, nextPositionY);
	// Set paddings
	int *pannings = CalculatePanning(monsterPositionX, monsterPositionY);
	Mix_SetPanning(snoring, pannings[0], pannings[1]);
	pannings = CalculatePanning(finishPositionX, finishPositionY);
	Mix_SetPanning(waterFall, pannings[0], pannings[1]);
}

void MoveMonster() {
	int possiblePositions[4][2], counter = 0;
	// Calculate posible positions
	// UP
	if (environment[monsterPositionX - 1][monsterPositionY] == 0) {
		possiblePositions[counter][0] = monsterPositionX - 1;
		possiblePositions[counter][1] = monsterPositionY;
		counter++;
	}
	// Left
	if (environment[monsterPositionX][monsterPositionY - 1] == 0) {
		possiblePositions[counter][0] = monsterPositionX;
		possiblePositions[counter][1] = monsterPositionY - 1;
		counter++;
	}
	// Right
	if (environment[monsterPositionX][monsterPositionY + 1] == 0) {
		possiblePositions[counter][0] = monsterPositionX;
		possiblePositions[counter][1] = monsterPositionY + 1;
		counter++;
	}
	// Down
	if (environment[monsterPositionX + 1][monsterPositionY] == 0) {
		possiblePositions[counter][0] = monsterPositionX + 1;
		possiblePositions[counter][1] = monsterPositionY;
		counter++;
	}

	// Select 1 randomly
	int rnd = rand() % counter;
	// Move
	monsterPositionX = possiblePositions[rnd][0];
	monsterPositionY = possiblePositions[rnd][1];
	int *pannings;
	// Play Move Sound
	Mix_HaltChannel(stepMonster);
	Mix_PlayChannel(stepMonster, audios[stepMonster], 0);
	// Set distance
	Mix_SetDistance(stepMonster, (Uint8)CalculateDistanceFromPlayer(monsterPositionX, monsterPositionY));
	// Set paddings
	pannings = CalculatePanning(monsterPositionX, monsterPositionY);
	Mix_SetPanning(stepMonster, pannings[0], pannings[1]);
}

void PrintInfo() {
	cout << "Environment" << endl;
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			if (playerPositionX == i && playerPositionY == j) {
				cout << "P ";
			}
			else if (monsterPositionX == i && monsterPositionY == j) {
				cout << "M ";
			}
			else
			{
				cout << environment[i][j] << " ";
			}			
		}
		cout << endl;
	}
	cout << "Player position: X:" << playerPositionX << " Y:" << playerPositionY << endl;
	cout << "Enemy position: X:" << monsterPositionX << " Y:" << monsterPositionY << endl;
	cout << "Player direction: X:" << playerDirection << endl;
	cout << "Finish position: X:" << finishPositionX << " Y:" << finishPositionY << endl;
	cout << "Distance from monster" << CalculateDistanceFromPlayer(monsterPositionX, monsterPositionY) << endl;
	cout << "Distance from waterfall" << CalculateDistanceFromPlayer(finishPositionX, finishPositionY) << endl;
}

void LoopAudio() {
	int* pannings;

	if (!Mix_Playing(waterFall) && playerCanMove) {
		Mix_PlayChannel(waterFall, audios[waterFall], 0);
		// Set distance
		Mix_SetDistance(waterFall, (Uint8)CalculateDistanceFromPlayer(finishPositionX, finishPositionY));
		// Set paddings
		pannings = CalculatePanning(finishPositionX, finishPositionY);
		Mix_SetPanning(waterFall, pannings[0], pannings[1]);
	}
	
	if (!Mix_Playing(snoring) && playerCanMove && !Mix_Playing(stepMonster)) {
		Mix_PlayChannel(snoring, audios[snoring], 0);		
		// Set distance
		Mix_SetDistance(snoring, (Uint8)CalculateDistanceFromPlayer(monsterPositionX, monsterPositionY));
		// Set paddings
		pannings = CalculatePanning(monsterPositionX, monsterPositionY);
		Mix_SetPanning(snoring, pannings[0], pannings[1]);
	}
}

int CalculateDistanceFromPlayer(int posX, int posY){
	int distance = (abs(playerPositionX - posX) * distanceFactor + abs(playerPositionY - posY) * distanceFactor);
	if (distance > 255) distance = 255;
	return distance;
}

int* CalculatePanning(int posX, int posY) {
	// 0 => Left channel, 1 => Right channel
	int directions[2] = { 255, 255 };
	switch (playerDirection)
	{
	case Direction::Up:
		if (playerPositionY > posY) {
			// Left side
			directions[1] = 128;

		}
		else if (playerPositionY < posY) {
			// Right side
			directions[0] = 128;
		}
		break;
	case Direction::Left:
		if (playerPositionX > posX) {
			// Right side
			directions[0] = 128;
		}
		else if(playerPositionX < posX)
		{
			// Left side
			directions[1] = 128;
		}
		break;
	case Direction::Right:
		if (playerPositionX > posX) {
			// Left side
			directions[1] = 128;
		}
		else if (playerPositionX < posX) {
			// Right side
			directions[0] = 128;
		}
		break;
	case Direction::Down:
		if (playerPositionY > posY) {
			// Right side
			directions[0] = 128;
		}
		else if(playerPositionY < posY)
		{
			// Left side
			directions[1] = 128;
		}
		break;
	}
	return directions;
}


void Initializations() {
	//Initialize SDL
	SDL_Init(SDL_INIT_EVERYTHING);
	//Create window
	gWindow = SDL_CreateWindow("PEC3 - La cueva de los condenados", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	gMyRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	//Sound audio active
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
	// Asign audio chanel numbers
	Mix_AllocateChannels(128);
	//Load Audios
	loadSound = Mix_LoadWAV("Assets/waterfall.wav");
	audios.push_back(loadSound);
	waterFall = audios.size() - 1;
	loadSound = Mix_LoadWAV("Assets/victory.wav");
	audios.push_back(loadSound);
	victory = audios.size() - 1;
	loadSound = Mix_LoadWAV("Assets/step-monster.wav");
	audios.push_back(loadSound);
	stepMonster = audios.size() - 1;
	loadSound = Mix_LoadWAV("Assets/step-human.wav");
	audios.push_back(loadSound);
	stepHuman = audios.size() - 1;
	loadSound = Mix_LoadWAV("Assets/shock-wall.wav");
	audios.push_back(loadSound);
	wallHit = audios.size() - 1;
	loadSound = Mix_LoadWAV("Assets/Monster-Snoring.mp3");
	audios.push_back(loadSound);
	snoring = audios.size() - 1;
	loadSound = Mix_LoadWAV("Assets/gameover.wav");
	audios.push_back(loadSound);
	gameOver = audios.size() - 1;
	loadSound = Mix_LoadWAV("Assets/death.wav");
	audios.push_back(loadSound);
	death = audios.size() - 1;	
}