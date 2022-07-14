#pragma once
#include <stdlib.h>
#include <cstring>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <netdb.h>
#include <list>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "IRCutils.hpp"
#include "IRCmsg.hpp"
#include "Client.hpp"
#include "Channel.hpp"

class IRCData
{
/////   struct chained mode /////
	class Mode
	{
		public :
			ptrFct				fctn;
			channelIterator		chanIt;
			std::string			client;
			char				flop;
			char				flag;
			std::string			arg;
								Mode( void )	{ return; }
								~Mode( void )	{ return; }
	};
	std::string								_printedFlags;
	std::string								_printedArgs;
	typedef std::list<Mode>::iterator		listModeIt;
	std::list<Mode>							_mods;
	listModeIt								_modsIt;
/////	Server Info /////
	std::string								_servIP;
	int										_port;
	std::string								_pass;

/////	PtrFctn /////
	listPairM								_listFctU;
	listPairM								_listFctC;
	listPairI								_listFctI;

/////	Socket Info /////
	int										_opt;
	int										_master_socket, _addrlen, _new_socket,
											_activity, _sd;
	fd_set									_readfds, _writefds, _crntfds;
	int										_max_sd;
	struct sockaddr_in						_address;

/////	Request Operation /////
	std::string								*_request;
	std::string								_cmd;
	std::string								_answer;
	char									_buff[1024];
	std::string 							_target;

/////	Client Info /////
	std::vector<Client*>					_clients;
	std::list<std::string>					_servOps;
	clientIterator							_clientIt;
	std::string 							_rejectChar;

/////	Channel info /////
	std::list<Channel>						_channels;
	std::string								_channelTmp, _chanPassTmp;

/////	BanInfo /////
	std::list< _pairBan >					_servBan;

/////	ModeInfo /////
	char									_flop;
	std::string								_flag;

	channelIterator							isChannel( std::string channelName );

	void									spaceTrimer( void );

	void									clearPostArgs( void );

	void									receveRequest( void );

	
	std::string const						getArg( void );
	std::string const						getLastArg( void );

	void									setCmd( void );

	clientIterator							isCli( std::string const &userTmp );

	strListIt								isOps( std::string userTmp );

	itBan									isBan ( std::string const &user );
	std::list<std::string>					KBList( const std::string& s );
	void									printNotChanOps( void );
	void									printNotServOps( void );
	void									C_MODE_L( void );
	void									C_MODE_B( void );
	void									C_MODE_K( void );
	void									C_MODE_M( void );
	void									C_MODE_T( void );
	void									C_MODE_N( void );
	void									C_MODE_O( void );
	void									C_MODE_P( void );
	void									C_MODE_S( void );
	void									C_MODE_I( void );
	void									C_MODE_V( void );
	void									U_MODE_O( void );
	void									wrongUserFlag( void );
	void									wrongFlag( void );
	void									execMode( void );
	void									setListFlagCmdU( void );
	void									setListFlagCmdC( channelIterator &chanIt );
	void									USERMODE( void );
	void									CHANMODE( void );
	void									MODE( void );

	void									LIST( void );
	void									WELCOME( void );
	void									CAP( void );
	void									checkPass( void );
	void									PASS( void );
	void									NICK( void );
	void									USER( void );
	void									WHO( void );
	void									PONG( void );
	void									JOIN( void );
	void									KICK( channelIterator chanIt, std::list<std::string> KBlist, std::string reason );
	void									BAN( channelIterator chanIt, std::list<std::string> KBlist, std::string reason );
	void									KICKBAN( void );
	void									KILLING( clientIterator const &kickIt/*, std::string cmd, std::string const reason */);
	void									KILL( void );
	void									OPENMSG( void );
	void									PRIVMSG( void );
	void									NOTICE( void );
	void									MSG( void );
	void									PART( void );
	void									QUIT( void );
	void									TOPIC( void );
	void									INVITE( void );
	void									MODE_GET_ARG( void );



	void									setAddress( void );
	void									closeEraseDeleteClient( clientIterator clientIt );

	public:
											IRCData( void );
											~IRCData( void );
		int									getMasterSocket( void ) const;
		fd_set	const						getCrntFds( void ) const;
		fd_set	const						*getPtrCrntFds( void ) const;
		fd_set	const						getReadFds( void ) const;
		fd_set	const						*getPtrReadFds( void ) const;
		fd_set	const						getWriteFds( void ) const;
		fd_set	const						*getPtrWriteFds( void ) const;
		struct sockaddr_in const 			&getAddress( void ) const;
		void								nbArgs( const int ac );

		void								initFct( void );
		void								init( std::string port, std::string password, char **ep );
		void								newClient( void );

		void								addClearedMasterSocket( void );
				
		void								activityListener( void );
		void								IOListener( void );
		void								execFct( void );

};