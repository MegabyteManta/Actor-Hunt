#include <sstream>
#include <iostream>
#include <Windows.h>

#include "ActorHunt.h"

using namespace std;

enum GameStates { won, lost, ongoing };
const unsigned short WORLD_HEIGHT = 20;
const unsigned short WORLD_WIDTH = 40;

/**
* Clears the screen
* @param char fill = ' ' : the char to fill the screen with
*/
void ClearScreen(char fill = ' ') {
	const COORD tl = { 0,0 };
	CONSOLE_SCREEN_BUFFER_INFO s;
	const HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(console, &s);
	DWORD written, cells = s.dwSize.X * s.dwSize.Y;
	FillConsoleOutputCharacter(console, fill, cells, tl, &written);
	FillConsoleOutputAttribute(console, s.wAttributes, cells, tl, &written);
	SetConsoleCursorPosition(console, tl);
}

/**
* Spawns pawns for the world.
* @param World* world : the world that spawns the pawns
* @param int damagersToSpawn : the number of Damager pawns to spawn
* @param int healersToSpawn : the number of Healer pawns to spawn
* @return Player* : the newly spawned player
*/
Player* SpawnPawns(World* world, int damagersToSpawn, int healersToSpawn)
{
	// spawn main charcter
	Player* mainCharacter = world->CreateActor<Player>();
	mainCharacter->AddComponent(new MoveComponent());

	// set the starting position of character
	// uses SetFrame in order to not set direction 
	const Vector2 move(WORLD_WIDTH / 2.0f, WORLD_HEIGHT / 2.0f);
	Frame initFrame;
	initFrame.pos = move;
	mainCharacter->SetFrame(initFrame);

	int damagersSpawned = 0;
	int healersSpawned = 0;

	// randomly spawn damagers and healers in the world 
	// pawns can't be spawned on top of other pawns
	while (damagersSpawned < damagersToSpawn || healersSpawned < healersToSpawn)
	{
		if (healersSpawned < healersToSpawn)
		{
			unsigned short x = rand() % WORLD_WIDTH;
			unsigned short y = rand() % WORLD_HEIGHT;
			if (world->FindPawnsAtLocation<Pawn>(x, y).empty())
			{
				healersSpawned++;
				Healer* healer = world->CreateActor<Healer>();
				Vector2 initPos(x, y);
				initFrame.pos = initPos;
				healer->SetFrame(initFrame);
			}
		}

		if (damagersSpawned < damagersToSpawn)
		{
			unsigned short x = (rand() % WORLD_WIDTH) + 1;
			unsigned short y = (rand() % WORLD_HEIGHT) + 1;
			if (world->FindPawnsAtLocation<Pawn>(x, y).empty())
			{
				damagersSpawned++;
				Damager* damager = world->CreateActor<Damager>();
				Vector2 initPos(x, y);
				initFrame.pos = initPos;
				damager->SetFrame(initFrame);
			}
		}
	}
	return mainCharacter;
}

/**
* Sets up world grid, adds the pawn's sigils to the grid, and prints out the grid
* @param World* world : the world that is being displayed
* @param char (&worldMap)[WORLD_HEIGHT][WORLD_WIDTH+1] : the world grid that will be printed out
* @param Player* mainCharacter : the player
*/
void BuildWorld(World* world, char (&worldMap)[WORLD_HEIGHT][WORLD_WIDTH+1], Player* mainCharacter)
{
	// fill world with default grid spaces
	for (int r = 0; r < WORLD_HEIGHT; ++r)
	{
		for (int c = 0; c < WORLD_WIDTH; ++c)
		{
			worldMap[r][c] = '#';
		}
	}

	// update the world with the pawns' sigils
	for (int i = 0; i <= world->GetAllActors().size(); i++)
	{
		// workaround to make sure main character's sigil always prints on top
		Actor* actor = (i != world->GetAllActors().size()) ? world->GetAllActors()[i] : mainCharacter;

		ActorHuntPawn* pawn = dynamic_cast<ActorHuntPawn*>(actor);

		// finds where the pawn is on the grid and sets that grid cell to the pawn's sigil
		if (pawn)
		{
			Frame frame = pawn->GetFrame();

			// prevents pawns from going out of bounds
			unsigned short posX = max(1, min((unsigned short)frame.pos.x, WORLD_WIDTH));
			unsigned short posY = max(1, min((unsigned short)frame.pos.y, WORLD_HEIGHT));
			frame.pos = Vector2(posX, posY);
			pawn->SetFrame(frame);

			// set the pawn's sigil on the world grid
			unsigned short r = WORLD_HEIGHT - (unsigned short)frame.pos.y;
			unsigned short c = WORLD_WIDTH - (unsigned short)frame.pos.x;
			r = max(0, min(r, WORLD_HEIGHT - 1));
			c = max(0, min(c, WORLD_WIDTH - 1));
			worldMap[r][c] = pawn->GetSigil();
		}
	}

	// print out world to screen
	for (int r = 0; r < WORLD_HEIGHT; ++r)
	{
		cout << worldMap[r] << endl;
	}
}

/**
* Finds pawns that overlap with the player and calls the player's overlap function with those pawns
* @param World* world : the world the player is in
* @param Player* mainCharacter : the player
*/
void HandleOverlappingPawns(World* world, Player* mainCharacter)
{
	vector<Pawn*> overlappingPawns = world->FindPawnsAtLocation<Pawn>(
		mainCharacter->GetFrame().pos.x, mainCharacter->GetFrame().pos.y);
	for (Pawn* pawn : overlappingPawns)
	{
		if (pawn != mainCharacter)
		{
			mainCharacter->OnOverlappedPawn(pawn);
		}
	}
}

int main()
{
	srand((unsigned int)time(NULL));

	char worldMap[WORLD_HEIGHT][WORLD_WIDTH+1] = {0};
	World * world = new World();

	// spawns the player, the damagers, and the healers.
	const int DAMAGERS_TO_SPAWN = 5;
	const int HEALERS_TO_SPAWN = 5;
	Player* mainCharacter = SpawnPawns(world, DAMAGERS_TO_SPAWN, HEALERS_TO_SPAWN);

	// if prev pos is the same as the player's initial pos the player won't take
	// damage or heal if a pawn were initially spawned onto them. Not relevant
	// for current game because a pawn can't spawn on the player.
	Vector2 prevPlayerPos(0,0);

	GameStates gameState = GameStates::ongoing;
	list<string> gameMessages;
	bool firstFrame = true;

	while( true )
	{
		// Capture the input
		InputManager & input = InputManager::GetInstance();
		input.CaptureInput();

		// Clear the screen if a key has been pressed for this first time this frame
		if (input.IsAnyKeyFirstPressed())
		{
			ClearScreen();
		}

		if (input.IsKeyPressed('Q'))
		{
			cout << "Exiting simulation" << endl;
			break;
		}

		// world only updates if it's the first frame or a key is pressed
		if (input.IsAnyKeyFirstPressed() || firstFrame)
		{
			world->Tick();

			// constructs the grid characters and the pawn sigils and displays the grid 
			BuildWorld(world, worldMap, mainCharacter);

			// Call main character's overlap function with any pawns its overlapping.
			// Won't get called if the main character didn't move.
			Vector2 curPlayerPos = mainCharacter->GetFrame().pos;
			if (prevPlayerPos.x != curPlayerPos.x || prevPlayerPos.y != curPlayerPos.y)
			{
				HandleOverlappingPawns(world, mainCharacter);
			}

			// update player's previous pos
			prevPlayerPos = mainCharacter->GetFrame().pos;


			// displays player's hp
			int hp = 0;
			mainCharacter->TryGetAttribute("hp", hp);
			ostringstream hpMessage;
			hpMessage << " Current hp: " << hp;
			gameMessages.emplace_front(hpMessage.str());

			// displays player's score
			int score = 0;
			mainCharacter->TryGetAttribute("score", score);
			ostringstream scoreMessage;
			scoreMessage << " Current score: " << score;
			gameMessages.emplace_front(scoreMessage.str());

			// state machine for winning/losing the game
			ostringstream stateMessage;
			switch (gameState) 
			{
			case GameStates::ongoing:
				if (hp < 1)
				{
					gameState = GameStates::lost;
					stateMessage << " You lost!";
				}
				else if (score >= 100)
				{
					gameState = GameStates::won;
					stateMessage << " You win!";
				}
				break;
			case GameStates::won:
				stateMessage << " You win!";
				break;
			case GameStates::lost:
				stateMessage << " You lost!";
				break;
			}
			if (!stateMessage.str().empty())
			{
				gameMessages.emplace_front(stateMessage.str());
			}

			// attach player's messages and print out all of the game's messages
			gameMessages.insert(gameMessages.end(), mainCharacter->GetMessages().begin(), mainCharacter->GetMessages().end());
			for (const string& message : gameMessages)
			{
				cout << message << endl;
			}
			gameMessages.clear();
		}

		firstFrame = false;
	}

	delete world;

	return 0;
}
