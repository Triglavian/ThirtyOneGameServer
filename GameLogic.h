#pragma once
#include <array>
#include <Windows.h>
#include "ClientSocket.h"
#include "GameState.h"

class GameLogic
{
public:
	GameLogic();
	~GameLogic();
	bool GameRunning(ClientSocket* cSocket);
	bool AddPlayer(ClientSocket* clientSocket);
	bool IsMaxPlayer();
private:
	///static std::array<ClientSocket*, 4> players;
	const int maxPlayer = 4;
	static int playerCount;
	int playerIndex;
	static std::array<HANDLE, 4> turnEvent;
	static int currentTurn;
	static int counter;
	static bool gameOver;
	const int maxCount = 31;
	bool NeedMorePlayer();
	void EnableNextPlayer();
	void SetCounterEvent();
	static HANDLE registerEvent;
	static HANDLE startEvent;
	static HANDLE countEvent;
	GameLogic(const GameLogic&) = delete;
};

