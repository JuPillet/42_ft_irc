#pragma once
#include <ctime>
#include <list>
#include <vector>
#include <string>

class	Client;
class	Channel;
class	IRCData;

typedef std::string::iterator					strIt;
typedef	std::vector<Client*>::iterator			clientIterator;
typedef	std::vector<Client*>::const_iterator	constClientIterator;
typedef std::list<Channel*>						channelsList;
typedef channelsList::iterator					channelsListIt;
typedef std::list<Channel>::iterator			channelIterator;
typedef std::list<std::string>::iterator		strListIt;
typedef std::list<std::string>::const_iterator	conStrListIt;
typedef std::pair<std::string, time_t>			pairBan;
typedef std::list<pairBan>::iterator			itBan;
typedef void( IRCData::*ptrFct )( void );
typedef std::pair<ptrFct, ptrFct>				pairFctsM;
typedef std::pair<char, pairFctsM>				pairKVM;
typedef std::pair<std::string, ptrFct>			pairKVI;
typedef std::list<pairKVM>						listPairM;
typedef std::list<pairKVI>						listPairI;
typedef std::pair< std::string, time_t >	_pairBan;
