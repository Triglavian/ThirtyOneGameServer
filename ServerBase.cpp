#include "ServerBase.h"

ServerBase::ServerBase(Int port)
{
	result = WSAStartup(MAKEWORD(2, 2), &wsa);
	this->port = port;
	threadHandles = new std::vector<HANDLE*>();
	lSocket = new ListenSocket(this->port);
	newSocket = 0;
	ZeroMemory(&clientAddr, sizeof(clientAddr));
	cSockets = new std::vector<ClientSocket*>();
	InitializeCriticalSection(&handleSection);
	InitializeCriticalSection(&cSocketSection);
	gameLogic = new GameLogic();
}

ServerBase::~ServerBase()
{
	if (threadHandles != nullptr) 
	{
		for (auto obj : *threadHandles)
		{
			CloseHandle(obj);
		}
		delete threadHandles;
	}
	if (cSockets != nullptr) 
	{
		for (auto* obj : *cSockets) 
		{
			delete obj;
		}
		delete cSockets;
	}
	if (lSocket != nullptr) delete lSocket;
	DeleteCriticalSection(&handleSection);
	DeleteCriticalSection(&cSocketSection);
	if (gameLogic != nullptr)
	{
		delete gameLogic;
	}
}

void ServerBase::Run()
{
	if (!InitializeServer())
	{
		//error handler
		return;
	}
	AcceptClients();
}

bool ServerBase::InitializeServer()
{
	if (IsWSAStartFailed()) 
	{
		//error handler
		ServerLogger::PrintLog("Failed to start up wsa");
		return false;
	}
	if (lSocket->IsInvalidSock()) 
	{
		//error handler
		ServerLogger::PrintLog("Failed to init listen socket");
		return false;
	}
	if (lSocket->IsUnbindedSocket()) 
	{
		//error handler
		ServerLogger::PrintLog("Failed to bind listen socket");
		return false;
	}
	if (lSocket->IsListeningFailed())
	{
		//error handler
		ServerLogger::PrintLog("Failed to listen connection");
		return false;
	}
	return true;
}

bool ServerBase::IsWSAStartFailed()
{
	return result != 0;
}

void ServerBase::AcceptClients()
{
	int addrLen = sizeof(clientAddr);
	HANDLE threadHandle;
	ZeroMemory(&clientAddr, sizeof(clientAddr));
 	while (true) 
	{
		newSocket = accept(lSocket->GetSocket(), (sockaddr*)&clientAddr, &addrLen);
		EnterCriticalSection(&handleSection);
		threadHandles->emplace_back(&threadHandle);
		threadHandle = (HANDLE)_beginthreadex(0, 0, ServerBase::StateSwitch, this, 0, 0);
		threadHandles->erase(std::find(threadHandles->begin(), threadHandles->end(), &threadHandle));
		LeaveCriticalSection(&handleSection);
		CloseHandle(threadHandle);
	}
	//WaitForMultipleObjects(threadHandles->size(), , 1, INFINITE);
}

unsigned int __stdcall ServerBase::StateSwitch(void* obj)
{
	ServerBase* server = static_cast<ServerBase*>(obj);
	EnterCriticalSection(&server->cSocketSection);
	ClientSocket* cSocket = new ClientSocket(server->newSocket, server->clientAddr);
	server->cSockets->emplace_back(cSocket);
	LeaveCriticalSection(&server->cSocketSection);
	bool flag = true;
	while (flag)
	{
		switch (cSocket->GetMainState()) 
		{
			case MainState::E_MAIN:
				cSocket->ModifyStateWithProtocol();
				break;
			case MainState::E_WAITPLAYER:
				if (server->gameLogic->IsMaxPlayer())
				{
					break;
				}
				server->gameLogic->AddPlayer(cSocket);
				cSocket->SetMainState(MainState::E_PLAYGAME);
				break;
			case MainState::E_PLAYGAME:
				server->gameLogic->GameRunning(cSocket);
				return;
			case MainState::E_DISCONNECTED:
				flag = false;
				break;
		}
	}
	EnterCriticalSection(&server->cSocketSection);
	server->cSockets->erase(std::find(server->cSockets->begin(), server->cSockets->end(), cSocket));
	LeaveCriticalSection(&server->cSocketSection);
	delete cSocket;
	return 0;
}
