#include "GameLogic.h"

GameLogic::GameLogic()
{
	//ZeroMemory(&players, sizeof(players));
	playerCount = 0;
	for (int i = 0; i < maxPlayer; i++) 
	{
		turnEvent[i] = CreateEvent(0, 0, 0, 0);
	}
	currentTurn = 0;
	registerEvent = CreateEvent(0, 0, 0, 0);
	startEvent = CreateEvent(0, 0, 0, 0);
	countEvent = CreateEvent(0, 0, 0, 0);
	playerIndex = 0;
	SetEvent(registerEvent);
	gameOver = false;
}

GameLogic::~GameLogic()
{
	CloseHandle(registerEvent);
	CloseHandle(startEvent);
	CloseHandle(countEvent);
	for (int i = 0; i < turnEvent.size(); i++)
	{
		CloseHandle(turnEvent[i]);
	}
}

bool GameLogic::GameRunning(ClientSocket* cSocket)
{	
	GameState state = GameState::E_WAIT;
	Int recvNum = 0;
	while (true)
	{
		switch (state)
		{
			case GameState::E_WAIT:
				if (WaitForSingleObject(startEvent, INFINITE) != WAIT_OBJECT_0)
				{
					ServerLogger::PrintLog(playerIndex + " start event failed");
					return false;
				}
				state = GameState::E_START;
				break;
			case GameState::E_START:
				cSocket->SendProtocolPacket(P_START);
				if (playerIndex == 0)	//first player start turn event
				{
					SetEvent(turnEvent[0]);
					state = GameState::E_ONTURN;
				}
				else 
				{
					state = GameState::E_OFFTURN;
				}
				break;
			case GameState::E_ONTURN:
				if (WaitForSingleObject(turnEvent[playerIndex], INFINITE) != WAIT_OBJECT_0)	//wait player turn event
				{
					ServerLogger::PrintLog(playerIndex + " start event failed");
					return false;
				}

				cSocket->SendProtocolPacket(P_TURN);	//send player turn protocol
				recvNum = 0;
				cSocket->RecvIntPacket(recvNum);		//recv selected count
				if (!cSocket->ValidateProtocol(P_COUNT))//packet validation
				{
					ServerLogger::PrintLog("Invalid Protocol");
					return false;
				}

				counter += recvNum.ToInt();	//add counter
				if (counter >= maxCount)	//if player reach 31 count
				{
					gameOver = true;
					state = GameState::E_END;
					cSocket->SetProtocol(P_WIN);
					SetCounterEvent();	
					break;
				}
				SetCounterEvent();
				EnableNextPlayer();
				state = GameState::E_OFFTURN;
				break;
			case GameState::E_OFFTURN:
				if (WaitForSingleObject(countEvent, INFINITE) != WAIT_OBJECT_0)
				{
					ServerLogger::PrintLog(playerIndex + " count event failed");
					return false;
				}
				if (gameOver) 
				{
					state = GameState::E_END;
					cSocket->SetProtocol(P_LOSE);
					break;
				}
				if (currentTurn == playerIndex)
				{
					state = GameState::E_ONTURN;
				}
				cSocket->SendIntPacket(P_COUNT, counter);
				break;
			case GameState::E_END:
				cSocket->SendProtocolPacket();
				return true;
		}
	}
	return false;
}

bool GameLogic::NeedMorePlayer()
{
	return playerCount < maxPlayer;
}

void GameLogic::EnableNextPlayer()
{	
	currentTurn = playerIndex + 1 < maxPlayer ? playerIndex + 1 : 0;
	SetEvent(turnEvent[currentTurn]);
}

void GameLogic::SetCounterEvent()
{
	for (int i = 0; i < maxPlayer - 1; i++)
	{
		SetEvent(countEvent);
	}
}

bool GameLogic::AddPlayer(ClientSocket* clientSocket)
{
	WaitForSingleObject(registerEvent, INFINITE);
	if (!NeedMorePlayer()) 
	{
		return false;
	}
	playerIndex = playerCount;
	//players[playerCount++] = clientSocket;
	if (NeedMorePlayer()) 
	{
		SetEvent(registerEvent);
	}
	else 
	{
		for (int i = 0; i < maxPlayer; i++)
		{
			SetEvent(startEvent);
		}
	}
	return true;
}

bool GameLogic::IsMaxPlayer()
{
	return playerCount == maxPlayer;
}
