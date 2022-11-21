#pragma once

enum class Protocol
{
	P_DISCONNECTION = 0,	//disconnection protocol, recivec if opps socket is closed
	P_INVALIDPROT,
	P_REGISTER,
	P_WAITPLAYERS,
	P_STARTGAME,
	P_STARTGAMERES,
	P_ONTURN,
	P_GETCOUNT,
	P_GETCOUNTRES,
	P_COUNTLEFT,
	P_COUNTLEFTRES,
	P_GAMEOVER,
	P_WIN,
	P_LOSE,

#define P_DISCON	Protocol::P_DISCONNECTION
#define P_INVALID	Protocol::P_INVALIDPROT
#define P_REGIST	Protocol::P_REGISTER
#define P_WAIT		Protocol::P_WAITPLAYERS
#define P_START		Protocol::P_STARTGAME
#define P_STARTRES	Protocol::P_STARTGAMERES
#define P_TURN		Protocol::P_ONTURN
#define P_COUNT		Protocol::P_GETCOUNT
#define P_COUNTRES	Protocol::P_GETCOUNTRES
#define P_CNTLEFT	Protocol::P_COUNTLEFT
#define P_CNTRES	Protocol::P_COUNTLEFTRES
#define P_GAMEOVER	Protocol::P_GAMEOVER
#define P_WIN		Protocol::P_WIN
#define P_LOSE		Protocol::P_LOSE
};