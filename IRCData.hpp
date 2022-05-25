#pragma once
#include <iostream>
#include <string>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <netdb.h>
#include <list>
#include <algorithm>
#include "Channel.hpp"

class Channel;

class IRCData
{
/////	Server Info /////
	std::string								_selfIP;
	int										_port;
	std::string								_pass;

/////	PtrFctn /////
	listPair								_listFctn;

/////	Socket Info /////
	int										_opt;
	int										_master_socket, _addrlen, _new_socket,
											_activity, _sd, _index;
	fd_set									_readfds, _writefds, _crntfds;
	int										_max_sd;
	struct sockaddr_in						_address;

/////	Request Operation /////
	std::string								*_request;
	std::string								_cmd;
	std::string								_answer;
	char									_buff[1024];
	std::string 							_dest;
	int										_destSD;

/////	Client Info /////
	std::list<Client*>						_clients;
	std::list<std::string>					_servOps;
	clientIterator							_clientIt;
	std::string 							_rejectChar;

/////	Channel info /////
	std::list<Channel>						_channels;
	std::string								_channelTmp, _chanPassTmp;

/////	BanInfo /////
	std::list< _pairBan >					_servBan;

							IRCData( IRCData &src );
							IRCData	&operator=( IRCData &src );

		void				spaceTrimer( void )
		{
			strIt	trimIt;
			for ( trimIt = _request->begin(); trimIt != _request->end() && *trimIt == ' '; ++trimIt );
			_request->erase( 0, trimIt - _request->begin() );
		}

		void	clearPostArgs( void )
		{
			strIt	postFct;
			for ( postFct = _request->begin(); postFct != _request->end() && *postFct != '\n'; ++postFct );
			_request->erase( 0, postFct + 1 - _request->begin() );
			spaceTrimer();
		}

		void				receveRequest( void ) {
			std::cout << "message start" << std::endl;

			_request = const_cast<std::string *>( ( *_clientIt )->getRequest() );
			std::cout << _request << std::endl;
			if ( *( _request->end() - 1 ) == '\n' )
				_request->clear();
			int readvalue;
			for ( int index = 0; index != 1024; ++index )
				_buff[index] = 0;
			readvalue = recv( _sd, _buff, 1024, 0 );
			
			if ( readvalue != -1 )
			{
				_request->append( std::string( reinterpret_cast<char *>( _buff ), readvalue ) );
				std::cout << "_sd: " << _sd << " - readvalue: " << readvalue << " : " << _request->length() << " : " << std::endl << *_request << std::endl;
			}
			else
				std::cout << strerror(errno) << std::endl;
			std::cout << "message exit" << std::endl;
		}

		void				setCmd( void )
		{
			strIt	cmdIt;
			for ( cmdIt = _request->begin(); cmdIt != _request->end()
				&& *cmdIt != '\n' && *cmdIt != '\r' && *cmdIt != ' '; ++cmdIt );
			_cmd = std::string( *_request, 0, cmdIt - _request->begin() );
			_request->erase( 0, cmdIt - _request->begin() );
			for ( cmdIt = _cmd.begin(); cmdIt != _cmd.end(); cmdIt++ )
				if ( std::isalpha( *cmdIt ) )
					*cmdIt = std::toupper(*cmdIt);
			spaceTrimer();
		}

		void	sender( void )
		{
			if ( send( _destSD, _answer.c_str(), _answer.length(), 0 ) == -1 )
			{
				_answer.clear();
				throw( IRCErr( "send" ) );
			}
			_answer.clear();
		}

		bool	isCli( std::string const &userTmp ) {
			for ( clientIterator cliIt = _clients.begin(); cliIt != _clients.end(); ++cliIt ) {
				if ( userTmp == ( *cliIt )->getUser() )
					return (1);
			}
			return (0);
		}

		bool	isOps( Client *userTmp ) {
			for ( itStr opsIt = _servOps.begin(); opsIt != _servOps.end(); ++opsIt ) {
				if ( *opsIt == userTmp->getUser() )
					return (1);
			}
			return (0);
		}

		bool	isBan ( std::string const cliUser  ) {
			for ( itBan tmpIt = _servBan.begin(); tmpIt != _servBan.end(); ++tmpIt )
			{
				if ( tmpIt->first == cliUser )
				{
					if (tmpIt->second && tmpIt->second <= std::time(nullptr))
					{
						_servBan.erase(tmpIt);
						break;
					}
					else
						return 1;
				}
			}
			return (0);
		}

		void	WELCOME( void )
		{
			if ( (*_clientIt)->getPass() == _pass
				&& (*_clientIt)->getNick().size() && (*_clientIt)->getUser().size() )
			{
				_destSD = _sd;
				_answer = ":" + _selfIP + " 001 " + (*_clientIt)->getNick() + " :Welcome to the IRC_QJ_Server "
				+ (*_clientIt)->getNick() + "!" + (*_clientIt)->getUser() + "@" + inet_ntoa( _address.sin_addr ) + "\r\n";
				std::cout << "_destSD: " << _destSD << std::endl;
				sender();
				std::cout << "Welcome message sent successfully" << std::endl;
				(*_clientIt)->setAutentification();
			}
		}

		void				CAP( void )
		{
			std::cout << *_request << std::endl;
			clearPostArgs();
			std::cout << *_request << std::endl;
		}

		void				checkPass( void )
		{
			if ( (*_clientIt)->getPass() != _pass )
			{
				_answer = ":Bad password\r\n";
				sender();
				throw IRCErr( "Bad password" );
			}
		}

		void				PASS( void )
		{
			std::string	passTmp;
			strIt		passIt;

			std::cout << _cmd << std::endl;
			_destSD = _sd;
	
			for ( passIt = _request->begin(); passIt != _request->end()
				&& *passIt != '\n' && *passIt != '\r' && *passIt != ' '; ++passIt );
			passTmp = std::string( *_request, 0, passIt - _request->begin() );
			clearPostArgs();


			if ( (*_clientIt)->getAutentification() )
			{
				_answer = ":" + _selfIP + " 462 " + " " + (*_clientIt)->getNick() + " " + ":You may not reregister";
				sender();
				throw IRCErr( (*_clientIt)->getUser() + " try a double registration" );
			}
			(*_clientIt)->setPass( passTmp );
			checkPass();
			if ( !(*_clientIt)->getAutentification() )
				WELCOME();
		}

		void				NICK( void )
		{
			std::string	nickTmp;
			strIt		nickIt;
			nickTmp.clear();
			for ( nickIt = _request->begin(); nickIt != _request->end()
				&& *nickIt != '\n' && *nickIt != '\r' && *nickIt != ' '; ++nickIt );
			nickTmp = std::string( *_request, 0, nickIt - _request->begin() );
			clearPostArgs();

			if ( !nickTmp.size() )
			{
				_destSD = _sd;
				_answer = "NICK format : NICK <nickname>\r\n"; //a voir le format en cas de NICK
				sender();
				throw IRCErr( "Nick format" );
			}

			nickIt = nickTmp.begin();
			if ( !std::isalpha( *nickIt ) )
			{
				_destSD = _sd;
				_answer = "Nickerror - first char of nick must be an alphabetic character\r\n";
				sender();
				throw IRCErr( "Nick format - first char not an alphabetic character" );
			}

			for ( ; nickIt != nickTmp.end(); ++nickIt ){
				for ( strIt rejectIt = _rejectChar.begin(); rejectIt != _rejectChar.end(); ++rejectIt ){				
					if ( *nickIt == *rejectIt ){
						_destSD = _sd;
						_answer = "Nickerror\r\n";
						sender();
						throw IRCErr( "Nick format" );
			}	}	}

			clientIterator	cliIt = _clients.begin();
			for ( ; cliIt != _clients.end(); ++cliIt )
			{
				if ( (*cliIt)->getNick() == nickTmp )
				{
					_destSD = _sd;
					_answer = "Nick already in use\r\n"; // A verifier a deux, si j essaie de prendre le nick d un autre
					sender();
					throw IRCErr( "Nick already in use" );
				}
			}

			(*_clientIt)->setNick( nickTmp );
			checkPass();
			if ( !(*_clientIt)->getAutentification() )
				WELCOME();
		}

		void				USER( void )
		{
			std::string	userTmp, modeTmp, hostTmp, nameTmp;
			strIt		userIt;

			for ( userIt = _request->begin(); userIt != _request->end()
				&& *userIt != '\n' && *userIt != '\r' && *userIt != ' '; ++userIt );
			userTmp = std::string( *_request, 0, userIt - _request->begin() );
			_request->erase(0, userIt - _request->begin() + 1);
			spaceTrimer();

			for ( userIt = _request->begin(); userIt != _request->end()
				&& *userIt != '\n' && *userIt != '\r' && *userIt != ' '; ++userIt );
			modeTmp = std::string( *_request, 0, userIt - _request->begin() );
			_request->erase(0, userIt - _request->begin() + 1);
			spaceTrimer();

			for ( userIt = _request->begin(); userIt != _request->end()
				&& *userIt != '\n' && *userIt != '\r' && *userIt != ' '; ++userIt );
			hostTmp = std::string( *_request, 0, userIt - _request->begin() );
			_request->erase(0, userIt - _request->begin());
			spaceTrimer();

			if ( *_request->begin() == ':' )
				_request->erase( _request->begin() );
			while ( _request->begin() != _request->end() && *_request->begin() != '\r' && *_request->begin() != '\n' )
			{
				for ( userIt = _request->begin(); userIt != _request->end()
					&& *userIt != '\n' && *userIt != '\r' && *userIt != ' '; ++userIt );
				nameTmp += std::string( *_request, 0, userIt - _request->begin() );
				_request->erase(0, userIt - _request->begin());
				spaceTrimer();
				nameTmp += ' ';
			}
			nameTmp.pop_back();
			clearPostArgs();

			if ( !( userTmp.size() && modeTmp.size() && hostTmp.size() && nameTmp.size() ) )
			{
				_destSD = _sd;
				_answer = ":USER command format : USER <username> <mode> <host> <:fullname>\r\n"; //a voir le format en cas de USER
				sender();
				throw IRCErr( "USER format" );
			}

			for ( strIt rejectIt = _rejectChar.begin(); rejectIt != _rejectChar.end(); ++rejectIt ){
				for ( strIt userIt = userTmp.begin(); userIt != userTmp.end(); ++userIt ){
					if ( *userIt == *rejectIt ){
						_destSD = _sd;
						_answer = "User format error\r\n";
						sender();
						throw IRCErr( "User format error" );
			}	}	}

			if ( isBan( userTmp ) )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "User serverBan rpl_code\r\n"; //Chercher le code erreur;
				sender();
				throw IRCErr( "User " + ( *_clientIt )->getUser() + " tried to connect during his banish time." );
			}

			clientIterator	cliIt;
			for ( cliIt = _clients.begin(); cliIt != _clients.end() && userTmp != ( *cliIt )->getUser(); ++cliIt );
			if ( cliIt != _clients.end() )
			{
				_destSD = _sd;
				_answer = "User already in use\r\n"; // A verifier a deux, si j essaie de prendre le nick d un autre
				sender();
				throw IRCErr( "User already in use" );
			}		

			(*_clientIt)->setUser( userTmp );
			checkPass();
			if ( !( (*_clientIt)->getAutentification() ) )
				WELCOME();
		}

		void	PONG( void )
		{
			_destSD = _sd;
			_answer = ":" + _selfIP + " PONG " + _selfIP + " :" + _selfIP + "\r\n";
			clearPostArgs();
			sender();
		}

		void				JOIN( void )
		{
			strIt	channelIt;
			//ICI CREER UN CHECK AUTHENTIFICATION
			for ( channelIt = _request->begin(); channelIt != _request->end()
				&& *channelIt != '\n' && *channelIt != '\r' && *channelIt != ' '; ++channelIt );
			_channelTmp = std::string( *_request, 0, channelIt - _request->begin() );
			
			if ( *_channelTmp.begin() != '#' )
				_channelTmp = "#" + _channelTmp;

			_request->erase( 0, channelIt - _request->begin() );
			spaceTrimer();

			for ( channelIt = _request->begin(); channelIt != _request->end()
				&& *channelIt != '\n' && *channelIt != '\r' && *channelIt != ' '; ++channelIt );
			_chanPassTmp = std::string( *_request, 0, channelIt - _request->begin() );
			clearPostArgs();

			channelIterator	chanIt;

			for ( chanIt = _channels.begin(); chanIt != _channels.end() && _channelTmp != chanIt->getName(); ++chanIt );
			if ( chanIt == _channels.end() )
			{
				_channels.push_back( _channelTmp );
				chanIt = _channels.end();
				--chanIt;
			}
			
			if ( chanIt->isCli( *_clientIt ) )
			{
				_destSD = _sd;
				_answer = "a voir!!!";
				sender();
				throw( IRCErr( "Is already in the channel" ) );
			}
			chanIt->setCli( *_clientIt );

			for ( constClientIterator chanCliIt = chanIt->getCli()->begin();
					chanCliIt != chanIt->getCli()->end(); ++chanCliIt )
			{
				_destSD = ( *chanCliIt )->getSocket();
				_answer = ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " " + _cmd + " " + _channelTmp + "\r\n";
				try
				{ sender(); }
				catch ( IRCErr const &err )
				{ std::cerr << err.getError() << std::endl; }
			}
		}

		void			KILLING( clientIterator const &cliIt, std::string const reason )
		{
			_destSD = ( *cliIt )->getSocket();
			_answer = " Avoir le formatage réponse envoyé à la personne ban " + reason;
			sender();
			delete ( *cliIt );
			_clients.erase( cliIt );
		}

		void			KILL( void )
		{
			std::string		userKick;
			size_t			endStol;
			std::string		reason;
			strIt			argIt;
			clientIterator	cliIt;

			if ( !isOps( *_clientIt ) )
			{
				_request->clear();
				_destSD = ( *_clientIt )->getSocket();
				_answer = " A voir ici le code erreur KLINE\r\n";
				sender();
				throw IRCErr( ( *_clientIt )->getUser() + " try command KILL without serveur operator rights." );
			}

			for ( argIt = _request->begin(); argIt != _request->end()
				&& *argIt != '\n' && *argIt != '\r' && *argIt != ' '; ++argIt );
			userKick = std::string( *_request, 0, argIt - _request->begin() );
			_request->erase( 0, argIt - _request->begin() );
			spaceTrimer();

			if ( *argIt == ':' )
			{
				for ( argIt = _request->begin(); argIt != _request->end()
					&& *argIt != '\n' && *argIt != '\r'; ++argIt );
				reason = std::string( *_request, 0, argIt - _request->begin() );
			}
			clearPostArgs();

			for ( cliIt = _clients.begin(); cliIt != _clients.end() && ( *cliIt )->getUser() != userKick; ++cliIt );
			if ( cliIt != _clients.end() )
				KILLING( cliIt, reason );
		}

		void			KLINE( void )
		{
			std::string		userBan;
			time_t			timeBan = 0;
			size_t			endStol;
			std::string		reason;
			strIt			argIt;
			clientIterator	cliIt;

			if ( !isOps( *_clientIt ) )
			{
				_request->clear();
				_destSD = ( *_clientIt )->getSocket();
				_answer = " A voir ici le code erreur KLINE ";
				sender();
				throw IRCErr( ( *_clientIt )->getUser() + " try command KLINE without serveur operator rights." );
			}

			for ( argIt = _request->begin(); argIt != _request->end()
				&& *argIt != '\n' && *argIt != '\r' && *argIt != ' '; ++argIt );
			userBan = std::string( *_request, 0, argIt - _request->begin() );
			_request->erase( 0, argIt - _request->begin() );
			spaceTrimer();

			for ( argIt = _request->begin(); argIt != _request->end()
				&& *argIt != '\n' && *argIt != '\r' && *argIt != ' '; ++argIt );
			timeBan = IRC::stol( *_request, &endStol );
			if ( endStol == 'h' || endStol == 'd' )
			{
				timeBan *= 3600;
				if ( endStol == 'd' )
					timeBan *= 24;
				++argIt;
			}
			if ( timeBan )
				timeBan += std::time( 0 );
			_request->erase( 0, argIt - _request->begin() );
			spaceTrimer();

			if ( *argIt == ':' )
			{
				for ( argIt = _request->begin(); argIt != _request->end()
					&& *argIt != '\n' && *argIt != '\r'; ++argIt );
				reason = std::string( *_request, 0, argIt - _request->begin() );
			}
			clearPostArgs();

			if ( isBan( userBan ) )
			{
				itBan cliBanished;
				for ( cliBanished = _servBan.begin(); cliBanished != _servBan.end() && cliBanished->first != userBan; ++cliBanished );
				_servBan.erase( cliBanished );
			}
			_servBan.push_back( pairBan ( userBan, timeBan ) );

			for ( cliIt = _clients.begin(); cliIt != _clients.end() && ( *cliIt )->getUser() != userBan; ++cliIt );
			if ( cliIt != _clients.end() )
				KILLING( cliIt, reason );
		}

		void					OPENMSG( void )
		{
			channelIterator	chanIt;
			clientIterator	clientIt;
			strIt			msgIt;
			for ( msgIt = _request->begin(); msgIt != _request->end() && *msgIt != '\n' && *msgIt != '\r'; ++msgIt );
			std::string		privMsg( _request->substr( 0, msgIt - _request->begin() ) );
			clearPostArgs();
			for ( chanIt = _channels.begin(); chanIt != _channels.end() && _dest != chanIt->getName(); ++chanIt );
			if ( chanIt == _channels.end() )
			{
				_destSD = (*_clientIt)->getSocket();
				_answer = "A voir formattage message"; // A voir chanel innexistant
				sender();
				throw ( IRCErr( "PRIVMSG - Channel " + _dest + " doesn't exist" ) );
			}

			if ( chanIt->isBan( *_clientIt ) )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "A voir formattage message"; // A voir client banni du channel
				sender();
				throw ( IRCErr( "PRIVMSG - You have been banned of " + _dest ) );
			}

			if ( chanIt->getExt() && !chanIt->isCli( *_clientIt ) )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "A voir formattage message"; // A voir le channel est fermé aux message exterieur
				sender();
				throw ( IRCErr( "PRIVMSG - Channel " + _dest + " external restriction" ) );
			}

			if ( chanIt->getMod() && !chanIt->isOps( *_clientIt ) && !chanIt->isVo( *_clientIt ) )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "A voir formattage message"; // A voir le channel est restrin au moderateur
				sender();
				throw ( IRCErr( "PRIVMSG - Channel " + _dest + " moderation restriction" ) );
			}		

			for ( clientIt = ( const_cast< std::list<Client*>* >( chanIt->getCli() ) )->begin() ; clientIt != _clients.end(); ++clientIt )
			{
				if ( ( *clientIt )->getUser() != ( *_clientIt )->getUser() )
				{
					_destSD = ( *clientIt )->getSocket();
					_answer = ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " " + _cmd + " " + privMsg + "\r\n"; // A voir formatage pour envoyer un message
					try
					{ sender(); } //essaye d envoyer le message a l utilisateur
					catch( IRCErr const &err )
					{ std::cerr << err.getError() << std::endl; } //affiche ici si le message n a pas ete envoyé a un utilisateur
				}
			}
		}

		void				PRIVMSG( void )
		{
			clientIterator	clientIt;
			strIt			msgIt;
			for ( msgIt = _request->begin(); msgIt != _request->end() && *msgIt != '\n' && *msgIt != '\r'; ++msgIt );
			std::string		privMsg( _request->substr( 0, msgIt - _request->begin() ) );
			clearPostArgs();
			for ( clientIt = _clients.begin(); clientIt != _clients.end() && _dest != (*clientIt)->getNick(); ++clientIt );
			if ( clientIt != _clients.end() )
			{
				_destSD = (*clientIt)->getSocket();
				_answer = ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " " + _cmd + " " + privMsg + "\r\n"; // A voir formatage pour envoyer un message
				sender();
			}
		}

		void				MSG( void )
		{
			strIt strIt;
			//ICI CREER UN CHECK AUTHENTIFICATION
			for ( strIt = _request->begin(); strIt != _request->end() && *strIt != ' '; strIt++ );
			_dest = _request->substr( 0, strIt - _request->begin() );
			_request->erase( 0, strIt - _request->begin() );
			spaceTrimer();
			if ( ( *_request )[0] != ':' )
				*_request = ":" + *_request;
			if ( _dest[0] == '#' )
				OPENMSG();
			else
				PRIVMSG();
		}

		void				PART( void )
		{
			strIt argIt;
			std::string chanPart;
			clientIterator cliIt;
			channelIterator chanIt;

			for ( argIt = _request->begin(); argIt != _request->end()
				&& *argIt != '\n' && *argIt != '\r' && *argIt != ' '; ++argIt );
			chanPart = _request->substr( 0, argIt - _request->begin() );
			_request->erase( 0, argIt - _request->begin() );
			clearPostArgs();

			for (chanIt = _channels.begin(); chanIt != _channels.end() && chanIt->getName() != chanPart; chanIt++);
			if (chanIt == _channels.end())
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "Avoir code channel innextiant"; //A VOIR CODE ERREUR
				sender();
				throw(IRCErr("Channel doesnt exist."));
			}
			try
			{
				try
				{ chanIt->eraseCli(*_clientIt); }
				catch ( IRCErr const &err )
				{
					_destSD = ( *_clientIt )->getSocket();
					_answer = "client isnt in the channel"; //A VOIR CODE ERREUR
					throw( err );
				}
				_destSD = ( *_clientIt )->getSocket();
				_answer = ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " PART " + chanPart + " :left away\r\n";
				sender();
				for ( constClientIterator chanCliIt = chanIt->getCli()->begin();
					chanCliIt != chanIt->getCli()->end(); ++chanCliIt )
				{
					_destSD = ( *chanCliIt )->getSocket();
					_answer = ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " PART " + chanPart + " :left away\r\n";
					try
					{ sender(); }
					catch ( IRCErr const &err )
					{ std::cerr << err.getError() << std::endl; }
				}
			}
			catch ( IRCErr const &err)
			{
				sender();
				throw( err );
			}
		}

		void				QUIT( void )
		{
			std::string		reQuit;
			clearPostArgs();
			_request = &reQuit;
			for ( channelIterator chanIt = _channels.begin(); chanIt != _channels.end(); ++chanIt )
			{
				if ( chanIt->isCli( *_clientIt ) )
				{
					clearPostArgs();
					*_request = chanIt->getName() + "\r\n";
					try
					{ PART(); }
					catch ( IRCErr const &err )
					{ std::cerr << err.getError() << std::endl; }
				}
			}
			for ( clientIterator clientIt = _clients.begin(); clientIt != _clients.end(); ++clientIt )
			{
				_destSD = ( *clientIt )->getSocket();
				_answer = ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " QUIT :left the server\r\n";
				try
				{ sender(); }
				catch ( IRCErr const &err )
				{ std::cerr << err.getError() << std::endl; }
			}
			closeEraseDeleteClient();
		}

		void				setAddress( void )
		{
			_address.sin_family = AF_INET;
			_address.sin_addr.s_addr = INADDR_ANY;
			_address.sin_port = htons( _port );
		}

		void				closeEraseDeleteClient( void )
		{
			//Somebody disconnected , get his details and print
			getpeername( _sd , reinterpret_cast<struct sockaddr *>( &_address ), reinterpret_cast<socklen_t *>( &_addrlen ) ); 
			std::cout << "Host disconnected , ip " << inet_ntoa( _address.sin_addr ) << ", port " << ntohs( _address.sin_port ) << std::endl;
			FD_CLR( _sd, &_crntfds );
			//Close the socket and mark as 0 in list for reuse
			delete (*_clientIt);
			_clients.erase( _clientIt );
		}
	public:
							IRCData( void ):_port(), _pass(), _opt(), _master_socket(), _addrlen(), _new_socket(), _activity(),
							_sd(), _max_sd(), _clients(), _address(), _request(), _readfds()
							{ _clients.clear(); }
							~IRCData( void ) {
								for ( _clientIt = _clients.begin(); _clientIt != _clients.end(); ++_clientIt )
									delete ( *_clientIt );
								_clients.erase( _clients.begin(), _clients.end() );
							}
		int	const			getMasterSocket( void ) const { return _master_socket; }
		fd_set	const		getCrntFds( void ) const { return _crntfds; }
		fd_set	const		*getPtrCrntFds( void ) const { return &_crntfds; }
		fd_set	const		getReadFds( void ) const { return _readfds; }
		fd_set	const		*getPtrReadFds( void ) const { return &_readfds; }
		fd_set	const		getWriteFds( void ) const { return _writefds; }
		fd_set	const		*getPtrWriteFds( void ) const { return &_writefds; }
		struct sockaddr_in const &getAddress( void ) const { return _address; }
		void				nbArgs( const int ac )
		{
			if( ac != 3 )
				throw( IRCErr( "Server need 2 arguments : port and password." ) );
		}

		void initFct ()
		{
			_listFctn.push_back( pairKV( "CAP", &IRCData::CAP ) );
			_listFctn.push_back( pairKV( "PASS", &IRCData::PASS ) );
			_listFctn.push_back( pairKV( "NICK", &IRCData::NICK ) );
			_listFctn.push_back( pairKV( "USER", &IRCData::USER ) );
			_listFctn.push_back( pairKV( "PING", &IRCData::PONG ) );
			_listFctn.push_back( pairKV( "JOIN", &IRCData::JOIN ) );
			_listFctn.push_back( pairKV( "PRIVMSG", &IRCData::MSG ) );
			_listFctn.push_back( pairKV( "KILL", &IRCData::KILL ) );
			_listFctn.push_back( pairKV( "KLINE", &IRCData::KLINE ) );
			_listFctn.push_back( pairKV( "PART", &IRCData::PART ) );
			_listFctn.push_back( pairKV( "QUIT", &IRCData::QUIT ) );
//			_listFctn.push_back( pairKV( "MODE", &IRCData::MODE ) );	
		}

		void				init( std::string port, std::string password, char **ep )
		{
			size_t			lastchar;
			char			selfhost[256];
			struct hostent	*host_entry;

			_clients.clear();
			_opt = 1;
			_rejectChar = "~!@#$%&*()+=:;\"\',<.>?/";

			_port = IRC::stoi( port, &lastchar );
			std::cout << lastchar << std::endl;
			std::cout << port[lastchar] << std::endl;
			if ( port[lastchar] || _port < 0 || _port > 65535 )
				throw( IRCErr( "Bad port value : enter port to 0 at 65 535" ) );

			_pass = password;

			int	var;
			for ( var = 0; ep[var] && std::string( ep[var] ).compare( 0, 5, "USER=" ) ; ++var );
			if ( ep[var] )
				std::cout << ep[var] << std::endl;
			if ( !ep[var] )
				throw ( IRCErr( "no user found for serverOPs" ) );
			_servOps.push_back( ep[var] + 5 );
			std::cout << *_servOps.begin() << std::endl;

			if ( ( _master_socket = socket( AF_INET, SOCK_STREAM, 0 ) ) == 0 )
				throw( IRCErr( "socket failed" ) );
			if ( setsockopt( _master_socket, SOL_SOCKET, SO_REUSEPORT, reinterpret_cast<char *>( &_opt ), sizeof( _opt ) ) < 0 )
				throw( IRCErr( "setsock_opt" ) );

			setAddress();

			if ( bind( _master_socket, reinterpret_cast<struct sockaddr *>( &_address ), sizeof( _address ) ) < 0 )
				throw( IRCErr( "bind failed" ) );

			_addrlen = sizeof( _address );

			if ( listen( _master_socket, 0 ) < 0 )
			{
				std::cout << IRCErr( "listen" ).getError() << std::endl;
				exit( EXIT_FAILURE );
			}

			FD_ZERO( &_crntfds );
			FD_SET( _master_socket, &_crntfds );

			gethostname( selfhost, sizeof( selfhost ) );
			host_entry = gethostbyname( selfhost );
			_selfIP = inet_ntoa( *( reinterpret_cast<struct in_addr*>( host_entry->h_addr_list[0] ) ) );
			close(4);

			std::cout << _selfIP << std::endl;

			initFct();

			std::cout << "Waiting for connections ..." << std::endl;
		}

		void				addClearedMasterSocket( void )
		{
			_max_sd = _master_socket;
			for ( _clientIt = _clients.begin(); _clientIt != _clients.end(); ++_clientIt )
			{
				_sd = (*_clientIt)->getSocket();
				if( _sd > _max_sd )
					_max_sd = _sd;
			}
//			std::cout << "MAXSD: " << _max_sd <<std::endl;
		}

		void				activityListener( void )
		{
			_readfds = _writefds = _crntfds;
			_activity = select( _max_sd + _clients.size() + 1, &_readfds, &_writefds, NULL, NULL );
			if ( ( _activity < 0 ) )  
				throw IRCErr( "select error" );
			std::cout << "SELECT OK" << std::endl;
		}

		void				execFct( void )
		{
			std::cout << "EXEC start" << std::endl;
			listPair::iterator	_listPairIt;
			if ( !( *_clientIt )->getAutentification() && _cmd != "PING" && _cmd != "PONG" && _cmd != "CAP" && _cmd != "PASS" && _cmd != "NICK" && _cmd != "USER" )
			{
				_request->clear();
				_destSD = _sd;
				_answer = "code erreur Identify yourself with command PASS, NICK and USER\r\n";
				sender();
				throw ( IRCErr( "Client try to use other command than PASS, NICK or USER without authentification\r\n" ) );
			}

			for ( _listPairIt = _listFctn.begin(); _listPairIt != _listFctn.end() && _listPairIt->first != _cmd; ++_listPairIt );
			if ( _listPairIt != _listFctn.end() )
			{
				std::cout << _cmd << std::endl;
				ptrfct ptrFct = _listPairIt->second;
				(this->*ptrFct)();
			}
			else
				clearPostArgs();
			std::cout << "EXEC exit" << std::endl;
		}

		void				newClient( void )
		{
			if ( ( _new_socket = accept( _master_socket, reinterpret_cast<struct sockaddr *>( &_address ), reinterpret_cast<socklen_t *>( &_addrlen ) ) ) < 0 )
				throw IRCErr( "accept" );
			FD_SET( _new_socket, &_crntfds );
			_clients.push_back( new Client( _new_socket ) );
			_sd = _new_socket;
			( *( --_clients.end() ) )->setClIp( inet_ntoa( _address.sin_addr ) );
			std::cout << "New connection , socket fd is " << _new_socket << ", ip is : " << inet_ntoa( _address.sin_addr ) << ", port : " << ntohs( _address.sin_port ) << std::endl;
		}

		void				IOListener( void )
		{
			for ( _clientIt = _clients.begin(); _clientIt != _clients.end(); ++_clientIt )
			{
				try
				{
					if ( FD_ISSET( (*_clientIt)->getSocket() , &_readfds ) )
					{
						_sd = (*_clientIt)->getSocket();
						//Check if it was for closing , and also read the 
						//incoming message
						receveRequest();
						if ( _request->length() <= 0 )
							closeEraseDeleteClient();
						else if ( *( _request->end() - 1 ) == '\n' )
						{
							spaceTrimer();
							while ( _request->size() )
							{
								std::cout << *_request << std::endl;
								setCmd();

								execFct();
//								usleep( 5000000 );
							}
						}
					}
//					usleep( 5000000 );
//					std::cout << "EXIT IOEXEC" << std::endl;
				}
				catch ( IRCErr const &err )
				{ std::cerr << err.getError() << std::endl; }
			}
		}

};