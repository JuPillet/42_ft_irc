#pragma once
#include <iostream>
#include <string>
#include <cstring>
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
/////   struct chained mode /////
	class Mode
	{
		public :
			ptrFct			fctn;
			channelIterator	chanIt;
			std::string		client;
			char			flop;
			std::string		arg;
			Mode( void )	{ return; }
			~Mode( void )	{ return; }
	};
	typedef std::list<Mode>::iterator		listModeIt;
	std::list<Mode>							_mods;
	listModeIt								_modsIt;
/////	Server Info /////
	std::string								_selfIP;
	int										_port;
	std::string								_pass;

/////	PtrFctn /////
	listPairM								_listFctU;
	listPairM								_listFctC;
	listPairI								_listFctI;

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
	std::string 							_target;
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

/////	ModeInfo /////
	char									_flop;
	std::string								_flag;

							IRCData( IRCData &src );
							IRCData	&operator=( IRCData &src );

		channelIterator		isChannel( std::string channelName )
		{
			channelIterator chanIt;
			for ( chanIt = _channels.begin(); chanIt != _channels.end() && chanIt->getName() != channelName; ++chanIt );
			return chanIt;
		}

		void				spaceTrimer( void )
		{
			strIt	trimIt;
			for ( trimIt = _request->begin(); trimIt != _request->end() && std::isspace( *trimIt ); ++trimIt );
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
				std::cout << std::strerror( errno ) << std::endl;
			std::cout << "message exit" << std::endl;
		}

		void	isUnsignedNumber( void )
		{
			std::string arg = _modsIt->arg;
			strIt argIt;
			for ( argIt = arg.begin(); argIt != arg.end() && std::isdigit( *argIt ); ++argIt );
			if ( !arg.size() || argIt != arg.end() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "voir erreur 461";
				sender();
				throw( IRCErr( ( *_clientIt )->getUser() + " argument isn't an unsigned number" ) );
			}
		}

		std::string const	getAllArg( void )
		{
			std::string argTmp;
			strIt		argIt;
			for ( argIt = _request->begin(); argIt != _request->end()
				&& *argIt != '\n' && *argIt != '\r'; ++argIt );
			argTmp = std::string( *_request, 0, argIt - _request->begin() );
			clearPostArgs();
			return argTmp;
		}

		std::string const	getMsgArg( void )
		{
			std::string argTmp;
			strIt		argIt;
			if ( ( *_request )[0] == ':' )
			{
				for ( argIt = _request->begin(); argIt != _request->end()
					&& *argIt != '\n' && *argIt != '\r'; ++argIt );
				argTmp = std::string( *_request, 0, argIt - _request->begin() );
			}
			clearPostArgs();
			return argTmp;
		}

		std::string const	getLastArg( void )
		{
			strIt		argIt;
			for ( argIt = _request->begin(); argIt != _request->end()
				&& *argIt != '\n' && *argIt != '\r' && !std::isspace( *argIt ); ++argIt );
			std::string argTmp = std::string( *_request, 0, argIt - _request->begin() );
			_request->erase( 0, argIt - _request->begin() );
			clearPostArgs();
			return argTmp;
		}

		std::string const	getArg( void )
		{
			strIt		argIt;
			for ( argIt = _request->begin(); argIt != _request->end()
				&& *argIt != '\n' && *argIt != '\r' && !std::isspace( *argIt ); ++argIt );
			std::string argTmp = std::string( *_request, 0, argIt - _request->begin() );
			_request->erase( 0, argIt - _request->begin() );
			spaceTrimer();
			return argTmp;
		}

		void				setCmd( void )
		{
			strIt	cmdIt;
			for ( cmdIt = _request->begin(); cmdIt != _request->end()
				&& *cmdIt != '\n' && *cmdIt != '\r' && !std::isspace( *cmdIt ); ++cmdIt );
			_cmd = getArg();
			for ( cmdIt = _cmd.begin(); cmdIt != _cmd.end(); cmdIt++ )
				if ( std::isalpha( *cmdIt ) )
					*cmdIt = std::toupper( *cmdIt );
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

		clientIterator	isCli( std::string const &userTmp )
		{
			clientIterator cliIt;
			for ( cliIt = _clients.begin(); cliIt != _clients.end() && userTmp != ( *cliIt )->getUser(); ++cliIt );
			return cliIt;
		}

		strListIt	isOps( std::string userTmp )
		{
			strListIt opsIt;
			for ( opsIt = _servOps.begin(); opsIt != _servOps.end() && *opsIt == userTmp; ++opsIt );
			return opsIt;
		}

		itBan	isBan ( std::string const &user )
		{
			itBan banIt;
			for ( banIt = _servBan.begin(); banIt != _servBan.end() && banIt->first != user; ++banIt );
			if ( banIt != _servBan.end() && banIt->second && banIt->second <= std::time( nullptr ) )
			{
				_servBan.erase( banIt );
				banIt = _servBan.end();
			}
			return banIt;
		}

		std::string	getUserByNick( std::string &nickTarget ) 
		{
			clientIterator clientIt;
			for ( clientIt = _clients.begin(); clientIt != _clients.end(); ++clientIt );
				if ( ( *clientIt )->getNick() == nickTarget )
					return ( *clientIt )->getUser();
			return "";
		}

		void	WELCOME( void )
		{
			if ( ( *_clientIt )->getPass() == _pass
				&& ( *_clientIt )->getNick().size() && ( *_clientIt )->getUser().size() )
			{
				_destSD = _sd;
				_answer = ":" + _selfIP + " 001 " + ( *_clientIt )->getNick() + " :Welcome to the IRC_QJ_Server "
				+ ( *_clientIt )->getNick() + "!" + ( *_clientIt )->getUser() + "@" + inet_ntoa( _address.sin_addr ) + "\r\n";
				std::cout << "_destSD: " << _destSD << std::endl;
				sender();
				std::cout << "Welcome message sent successfully" << std::endl;
				( *_clientIt )->setAutentification();
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
			if ( ( *_clientIt )->getPass() != _pass )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = ":*." + _selfIP + " " + " 475 " + (*_clientIt)->getNick() + " " + _target + " :Cannot join channel (incorrect channel key)\r\n";
				sender();
				throw( IRCErr( ( *_clientIt )->getUser() + " Bad password" ) );
			}
		}

		void				PASS( void )
		{
			std::string	passTmp;
			strIt		passIt;

			std::cout << _cmd << std::endl;
			_destSD = _sd;
	
			passTmp = getLastArg();
			if ( ( *_clientIt )->getAutentification() )
			{
				_answer = ":" + _selfIP + " 462 " + " " + ( *_clientIt )->getNick() + " " + ":You may not reregister";
				sender();
				throw IRCErr( ( *_clientIt )->getUser() + " try a double registration" );
			}
			( *_clientIt )->setPass( passTmp );
			checkPass();
			if ( !( *_clientIt )->getAutentification() )
				WELCOME();
		}

		void				NICK( void )
		{
			std::string	nickTmp;
			strIt		nickIt;
			nickTmp.clear();

			if ( !( nickTmp = getLastArg() ).size() )
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
				_answer = ":*." + _selfIP + " 432 " + nickTmp + " :Erroneous Nickname first character not an alphabetic character\r\n";
				sender();
				throw IRCErr( "Nick format - first char not an alphabetic character" );
			}

			for ( ; nickIt != nickTmp.end(); ++nickIt )
			{
				for ( strIt rejectIt = _rejectChar.begin(); rejectIt != _rejectChar.end(); ++rejectIt )
				{
					if ( *nickIt == *rejectIt )
					{
						_destSD = _sd;
						_answer = ":*." + _selfIP + " 432 " + nickTmp + " :Erroneous Nickname, nick contain caracter '" + *nickIt + "'\r\n";
						sender();
						throw IRCErr( "Nick format, nick contain caracter '" + *nickIt + '\'' );
					}
				}
			}

			clientIterator	cliIt = _clients.begin();
			for ( ; cliIt != _clients.end(); ++cliIt )
			{
				if ( ( *cliIt )->getNick() == nickTmp )
				{
					_destSD = _sd;
					_answer = ":*." + _selfIP + " 433 * " + nickTmp + " :Nickname already in use\r\n"; // A verifier a deux, si j essaie de prendre le nick d un autre
					sender();
					throw IRCErr( "Nick already in use" );
				}
			}

			( *_clientIt )->setNick( nickTmp );
			checkPass();
			if ( !( *_clientIt )->getAutentification() )
				WELCOME();
		}

		void				USER( void )
		{
			std::string	userTmp = getArg(), modeTmp = getArg(), hostTmp = getArg(), nameTmp;
			strIt		userIt;

			if ( *_request->begin() == ':' )
			{
				_request->erase( _request->begin() );
				while ( _request->begin() != _request->end() && *_request->begin() != '\r' && *_request->begin() != '\n' )
					nameTmp += ( getArg() + ' ' );
				nameTmp.pop_back();
			}
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

			if ( isBan( userTmp ) != _servBan.end() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "User serverBan rpl_code\r\n"; //Chercher le code erreur;
				sender();
				throw IRCErr( "User " + ( *_clientIt )->getUser() + " tried to connect during his banish time." );
			}

			if ( isCli( userTmp ) != _clients.end() )
			{
				_destSD = _sd;
				_answer = "User already in use\r\n"; // A verifier a deux, si j essaie de prendre le nick d un autre
				sender();
				throw IRCErr( "User already in use" );
			}		

			( *_clientIt )->setUser( userTmp );
			checkPass();
			if ( !( ( *_clientIt )->getAutentification() ) )
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
			_channelTmp = getArg();
			
			if ( _channelTmp == "-invite" )
				_channelTmp = ( *_clientIt )->getInvitation();
			if ( *_channelTmp.begin() != '#' )
				_channelTmp = "#" + _channelTmp;

			_chanPassTmp = getLastArg();
			channelIterator	chanIt;
			if ( ( chanIt = isChannel( _channelTmp ) ) == _channels.end() )
			{
				_channels.push_back( _channelTmp );
				chanIt = _channels.end();
				--chanIt;
			}

			constClientIterator chanCliIt = chanIt->isCli( ( *_clientIt )->getNick() );
			if ( chanCliIt != ( chanIt->getCli() )->end() )
			{
				_destSD = _sd;
				_answer = "a voir!!!";
				sender();
				throw( IRCErr( "Is already in the channel" ) );
			}
			if ( chanIt->getPass() != "" && chanIt->getPass() != _chanPassTmp )
			{
				_destSD = ( *chanCliIt )->getSocket();
				_answer = ":*." + _selfIP + " 475 " + ( *_clientIt )->getNick() + " " + _channelTmp + " :Cannot join channel ( incorrect channel key )\r\n";
				sender();
				throw( "bad password" );
			}
			if ( !( chanIt->getOps()->size() ) )
				chanIt->setOps( ( *_clientIt )->getUser() );
			if ( !chanIt->getLimit() || )
			chanIt->setCli( *_clientIt );
			_destSD = ( *chanCliIt )->getSocket();
			_answer = ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " " + _cmd + " " + _channelTmp + "\r\n";
			sender();
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

			if ( isOps( ( *_clientIt )->getUser() ) == _servOps.end() )
			{
				_request->clear();
				_destSD = ( *_clientIt )->getSocket();
				_answer = " A voir ici le code erreur KLINE\r\n";
				sender();
				throw IRCErr( ( *_clientIt )->getUser() + " try command KILL without serveur operator rights." );
			}

			userKick = getArg();
			reason = getMsgArg();

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

			if ( isOps( ( *_clientIt )->getUser() ) == _servOps.end() )
			{
				_request->clear();
				_destSD = ( *_clientIt )->getSocket();
				_answer = " A voir ici le code erreur KLINE ";
				sender();
				throw IRCErr( ( *_clientIt )->getUser() + " try command KLINE without serveur operator rights." );
			}

			userBan = getArg();
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
			getArg();

			reason = getMsgArg();
			itBan cliBanished = isBan( userBan );
			if ( cliBanished != _servBan.end() )
				_servBan.erase( cliBanished );
			_servBan.push_back( pairBan ( userBan, timeBan ) );
	
			if ( ( cliIt = isCli( userBan ) ) != _clients.end() )
				KILLING( cliIt, reason );
		}

		void					OPENMSG( void )
		{
			channelIterator	chanIt = isChannel( _target );
			std::string		privMsg = getMsgArg();

			if ( chanIt == _channels.end() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "A voir formattage message"; // A voir channel innexistant
				sender();
				throw ( IRCErr( "PRIVMSG - Channel " + _target + " doesn't exist" ) );
			}

			if ( ( chanIt->isBan( ( *_clientIt )->getUser() ) ) == chanIt->getBan()->end() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "A voir formattage message"; // A voir client banni du channel
				sender();
				throw ( IRCErr( "PRIVMSG - You have been banned of " + _target ) );
			}

			if ( chanIt->getExt() && chanIt->isCli( ( *_clientIt )->getUser() ) == chanIt->getCli()->end() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "A voir formattage message"; // A voir le channel est fermé aux message exterieur
				sender();
				throw ( IRCErr( "PRIVMSG - Channel " + _target + " external restriction" ) );
			}

			if ( chanIt->getMod()															
				&& chanIt->isOps( ( *_clientIt )->getUser() ) == chanIt->getOps()->end()
				&& chanIt->isVo( ( *_clientIt )->getUser() ) == chanIt->getVo()->end() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "A voir formattage message"; // A voir le channel est restrin au moderateur
				sender();
				throw ( IRCErr( "PRIVMSG - Channel " + _target + " moderation restriction" ) );
			}		

			constClientIterator	clientIt;

			_answer = ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " " + _cmd + " " + _target + " " + privMsg + "\r\n"; // A voir formatage pour envoyer un message
			for ( clientIt = ( chanIt->getCli() )->cbegin() ; clientIt != ( chanIt->getCli() )->cend(); ++clientIt )
			{
				if ( ( *clientIt )->getUser() != ( *_clientIt )->getUser() )
				{
					_destSD = ( *clientIt )->getSocket();
					try
					{ sender(); } //essaye d envoyer le message a l utilisateur
					catch( IRCErr const &err )
					{ std::cerr << err.getError() << std::endl; } //affiche ici si le message n a pas ete envoyé a un utilisateur
				}
			}
		}

		void				PRIVMSG( void )
		{
			std::string		privMsg = getMsgArg();
			clientIterator clientIt = isCli( _target );
			if ( clientIt != _clients.end() )
			{
				_destSD = ( *clientIt )->getSocket();
				_answer = ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " " + _cmd + " " + _target + " " + privMsg + "\r\n"; // A voir formatage pour envoyer un message
				sender();
			}
		}

		void				MSG( void )
		{
			_target = getArg();
			if ( ( *_request )[0] != ':' )
				*_request = ":" + *_request;
			if ( _target[0] == '#' )
				OPENMSG();
			else
				PRIVMSG();
		}

		void				PART( void )
		{
			std::string chanPart = getLastArg();
			channelIterator chanIt = isChannel( chanPart );

			if ( chanIt == _channels.end() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "Avoir code channel innextiant"; //A VOIR CODE ERREUR
				sender();
				throw( IRCErr( "Channel doesnt exist." ) );
			}
			try
			{
				try
				{ chanIt->eraseCli( ( *_clientIt )->getUser() ); }
				catch ( IRCErr const &err )
				{
					_destSD = ( *_clientIt )->getSocket();
					_answer = "client isnt in the channel"; //A VOIR CODE ERREUR
					sender();
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
			catch ( IRCErr const &err )
			{
				sender();
				throw( err );
			}
		}

		void				TOPIC( void )
		{
			_target = getArg();
			channelIterator channelIt = isChannel( _target );
			if ( channelIt == _channels.end() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "avoir code erreur channel innexistant"; //A VOIR CODE ERREUR
				sender();
				throw( "TOPIC: channel innexistant" );
			}
			std::string topic = getLastArg();
			if ( topic.size() != 0 )
				channelIt->setTopic( topic );
			else
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "format de retour du topic";
				sender();
			}
		}

		void				INVITE( void )
		{
			std::string user = ( *_clientIt )->getNick();
			_target = getArg();
			clientIterator cliTarget = isCli( _target );
			_channelTmp = getLastArg();
			channelIterator channel = isChannel( _channelTmp );
			if ( cliTarget == _clients.end() )
			{
				_request->clear();
				_destSD = ( *_clientIt )->getSocket();
				_answer = "format client cible n existe pas";
				sender();
				throw( _target + " not server client" );
			}
			if ( channel == _channels.end() || channel->isOps( user ) == channel->getOps()->end() )
			{
				_request->clear();
				_destSD = ( *_clientIt )->getSocket();
				_answer = "format n est pas operateur channel du channel visé";
				sender();
				throw( user + " not '" + _channelTmp + "' channel operator" );
			}
			if ( channel->isCli( _target ) == channel->getCli()->end() )
			{
				_request->clear();
				_destSD = ( *_clientIt )->getSocket();
				_answer =  ":*." + _selfIP + " 443 " + user + " " + _target + " :" + _channelTmp + " :is already on channel\r\n";
				sender();
				throw( _target + " allready on '" + _channelTmp + "' channel" );
			}
			_destSD = ( *cliTarget )->getSocket();
			_answer = ":" + _target + "!~" + ( *cliTarget )->getUser() + "@" + ( *cliTarget )->getClIp() + " " + _cmd + " " + ( *cliTarget )->getUser() + " :" + _channelTmp + "\r\n";
			sender();
			( *cliTarget )->setInvitation( _channelTmp );
			_destSD = ( *_clientIt )->getSocket();
			_answer = ":*." + _selfIP + " 341 " + user + " " + _target + " :" + _channelTmp + "\r\n";
			sender();
		}

		void				QUIT( void )
		{
			std::string		quitMsg;

			if ( !( quitMsg = getMsgArg() ).size() )
				quitMsg = ":left the server";
			for ( channelIterator chanIt = _channels.begin(); chanIt != _channels.end(); ++chanIt )
			{
				if ( chanIt->isCli( ( *_clientIt )->getUser() ) != chanIt->getCli()->end() )
				{
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
				_answer = ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " QUIT " + quitMsg + "\r\n";
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
			delete ( *_clientIt );
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

		void				MODE_GET_ARG( void )
		{
			std::string arg = getArg();
			_modsIt->arg = arg;
		}

		void				initFct( void )
		{
			////	listPtrFctnIRC
			_listFctI.push_back( pairKVI( "CAP", &IRCData::CAP ) );
			_listFctI.push_back( pairKVI( "PASS", &IRCData::PASS ) );
			_listFctI.push_back( pairKVI( "NICK", &IRCData::NICK ) );
			_listFctI.push_back( pairKVI( "USER", &IRCData::USER ) );
			_listFctI.push_back( pairKVI( "PING", &IRCData::PONG ) );
			_listFctI.push_back( pairKVI( "JOIN", &IRCData::JOIN ) );
			_listFctI.push_back( pairKVI( "PRIVMSG", &IRCData::MSG ) );
			_listFctI.push_back( pairKVI( "INVITE", &IRCData::INVITE ) );
			_listFctI.push_back( pairKVI( "KILL", &IRCData::KILL ) );
			_listFctI.push_back( pairKVI( "KLINE", &IRCData::KLINE ) );
			_listFctI.push_back( pairKVI( "PART", &IRCData::PART ) );
			_listFctI.push_back( pairKVI( "TOPIC", &IRCData::TOPIC ) );
			_listFctI.push_back( pairKVI( "QUIT", &IRCData::QUIT ) );
			_listFctI.push_back( pairKVI( "MODE", &IRCData::MODE ) );
			////	listPtrFctnModeChannel
			_listFctC.push_back( pairKVM( 'o', pairFctsM( &IRCData::C_MODE_O, &IRCData::MODE_GET_ARG ) ) );
			_listFctC.push_back( pairKVM( 'p', pairFctsM( &IRCData::C_MODE_P, 0 ) ) );
			_listFctC.push_back( pairKVM( 's', pairFctsM( &IRCData::C_MODE_S, 0 ) ) );
//			_listFctC.push_back( pairKVM( 'i', pairFctsM( &IRCData::C_MODE_I, 0 ) ) );
//			_listFctC.push_back( pairKVM( 't', pairFctsM( &IRCData::C_MODE_T, 0 ) ) );
			_listFctC.push_back( pairKVM( 'n', pairFctsM( &IRCData::C_MODE_N, 0 ) ) );
			_listFctC.push_back( pairKVM( 'm', pairFctsM( &IRCData::C_MODE_M, 0 ) ) );
//			_listFctC.push_back( pairKVM( 'l', pairFctsM( &IRCData::C_MODE_L, &IRCData::MODE_GET_ARG ) ) );
			_listFctC.push_back( pairKVM( 'b', pairFctsM( &IRCData::C_MODE_B, &IRCData::MODE_GET_ARG ) ) );
			_listFctC.push_back( pairKVM( 'v', pairFctsM( &IRCData::C_MODE_V, 0 ) ) );
			_listFctC.push_back( pairKVM( 'k', pairFctsM( &IRCData::C_MODE_K, &IRCData::MODE_GET_ARG ) ) );
			////	listPtrFctnModeUser
			_listFctU.push_back( pairKVM( 'o', pairFctsM( &IRCData::U_MODE_O, &IRCData::MODE_GET_ARG ) ) );
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
			close( 4 );

			std::cout << _selfIP << std::endl;

			initFct();

			std::cout << "Waiting for connections ..." << std::endl;
		}

		void				addClearedMasterSocket( void )
		{
			_max_sd = _master_socket;
			for ( _clientIt = _clients.begin(); _clientIt != _clients.end(); ++_clientIt )
			{
				_sd = ( *_clientIt )->getSocket();
				if( _sd > _max_sd )
					_max_sd = _sd;
			}
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
			listPairI::iterator	_listPairIt;
			if ( !( *_clientIt )->getAutentification() && _cmd != "PING" && _cmd != "PONG" && _cmd != "CAP" && _cmd != "PASS" && _cmd != "NICK" && _cmd != "USER" )
			{
				_request->clear();
				_destSD = _sd;
				_answer = "code erreur Identify yourself with command PASS, NICK and USER\r\n";
				sender();
				throw ( IRCErr( "Client try to use other command than PASS, NICK or USER without authentification\r\n" ) );
			}

			for ( _listPairIt = _listFctI.begin(); _listPairIt != _listFctI.end() && _listPairIt->first != _cmd; ++_listPairIt );
			if ( _listPairIt != _listFctI.end() )
			{
				std::cout << _cmd << std::endl;
				ptrFct ptrFct = _listPairIt->second;
				( this->*ptrFct )();
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
					if ( FD_ISSET( ( *_clientIt )->getSocket() , &_readfds ) )
					{
						_sd = ( *_clientIt )->getSocket();
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
							}
						}
					}
				}
				catch ( IRCErr const &err )
				{
					_request->clear();
					std::cerr << err.getError() << std::endl;
				}
			}
		}













		void				U_MODE_O( void )
		{
			if ( !_target.size() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer =  ":" + _selfIP + " 461 " + ( *_clientIt )->getNick()  + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " MODE :MODE user +o targetted user forgotten\r\n";
				sender();
				throw( IRCErr( "MODE user +o targeted user forgotten" ) );
			}
			if ( isCli( _target ) == _clients.end() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = ":*." + _selfIP + " 401 " + ( *_clientIt )->getNick() + " " + _target + " :No such nick\r\n";
				sender();
				throw( IRCErr( std::string( "No such nick " + _target ) ) );
			}
			if ( _modsIt->flop == '+' )
			{
				if ( isOps( _target ) == _servOps.end() )
					_servOps.push_back( _target );
			}
			else
			{
				strListIt opsTarget = isOps( _target );
				if ( opsTarget != _servOps.end() )
					_servOps.erase( opsTarget );
			}
			_destSD = ( *_clientIt )->getSocket();
			_answer = ":" + ( *_clientIt )->getNick() + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " MODE " + _target + " " + _modsIt->flop + "o :" + _target + "\r\n";
			sender();
		}

		void				C_MODE_B( void )
		{
			//char _flop egal a plus ou moins pour savoir si je dois ban ou unban
			std::string nickUser = getArg();
			if ( nickUser.size() )
			{
				if ( _modsIt->flop == '+' )
					_modsIt->chanIt->setBan( nickUser , 0 );
				else
					_modsIt->chanIt->unBan( nickUser );
			}
			else
			{
				_destSD = ( *_clientIt )->getSocket();
				std::list<pairBan> const	*channelBan = _modsIt->chanIt->getBan();
				for ( std::list<pairBan>::const_iterator chanBanIt = channelBan->begin(); chanBanIt != channelBan->end(); ++chanBanIt )
				{
					_answer = ":*." + _selfIP + " 367 " + ( *_clientIt )->getNick() + " " + _target + " " + nickUser + "!*@*\r\n";
					sender();
				}
				_answer = ":*." + _selfIP + " 368 " + ( *_clientIt )->getNick() + " " + _target + " :End of channel ban list\r\n";
				sender();
			}
		}





		void				C_MODE_K( void )
		{
			if ( _modsIt->flop == '-' )
				_modsIt->chanIt->unsetPass();
			else
			{
				_chanPassTmp = getArg();
				if ( !_chanPassTmp.size() )
				{
					_destSD = ( *_clientIt )->getSocket();
					_answer = ":*." + _selfIP + " 696 " + ( *_clientIt )->getNick() + _target + " k * :You must specify a parameter for the key mode. Syntax: <key>.\r\n";
					sender();
					throw( IRCErr( ( *_clientIt )->getUser() + " forget argument for channel mode " + _flop + "k" ) );
				}
				if ( _modsIt->chanIt->getPass().size() )
				{
					_destSD = ( *_clientIt )->getSocket();
					_answer = ":*." + _selfIP + " 467 " + ( *_clientIt )->getNick() + _target + " k * :Channel key already set.\r\n";
					sender();
					throw( IRCErr( ( *_clientIt )->getUser() + " password already exist for channel mode " + _flop + "k" ) );
				}
				_modsIt->chanIt->setPass( _chanPassTmp );
			}
		}

		void				C_MODE_M( void )
		{
			if ( _flop == '-' )
				_modsIt->chanIt->setMod( 0 );
			else if ( !_modsIt->chanIt->getMod() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = ":" + _selfIP + " " + _cmd + " " + _target + " :+m\r\n";
				sender();
				_modsIt->chanIt->setMod( 1 );
			}
		}

		void				C_MODE_N( void )
		{
			if ( _flop == '-' )
				channel->setExt( 0 );
			else
				channel->setExt( 1 );
		}

		void				C_MODE_O( void )
		{
			strIt	  strIt;
			//char _flop egal a plus ou moins pour savoir si je dois ban ou unban
			std::string user = getArg();
			if ( !user.size() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "Voir ici code erreur argument manquant\r\n";
				_request->clear();
				sender();
				throw( IRCErr( ( *_clientIt )->getUser() + " foget argument for channel mode " + _flop + "o" ) );
			}
			if ( _flop == '+' && channel->isOps( target ) == ( *channel->getOps() ).end() )
				channel->setOps( target );
			else if ( _flop == '-' && channel->isOps( target ) != ( *channel->getOps() ).end() )
				channel->unsetOps( target );
		}

		void				C_MODE_P( void )
		{
			channelIterator channel = isChannel( _target )
			if ( _flop == '-' )
				->setPriv( 0 );
			else
				channel->setPriv( 1 );
		}

		void				C_MODE_S( void )
		{
			if ( _flop == '-' )
				channel->setSecret( 0 );
			else
				channel->setSecret( 1 );
		}
	
		void				C_MODE_V( void )
		{
			std::string client = getArg();
			if ( !target.size() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "Voir ici code erreur argument manquant\r\n";
				_request->clear();
				sender();
				throw( IRCErr( ( *_clientIt )->getUser() + " foget argument for channel mode " + _flop + "v" ) )
			}
			if ( _flop == '+' && channel->isVo( target ) != ( *channel->getVo() ).end() )
				_target->setVo( client );
			else if ( _flop == '-' )
				_target->unsetVo( client );
		}

		void	wrongUserFlag( void )
		{
			std::string flagList( "io" );
			strIt flagIt;
			for ( flagIt = _flag.begin(); flagIt != _flag.end(); ++flagIt )
			{
				strIt flagListIt;
				for ( flagListIt = flagList.begin(); flagListIt != flagList.end() && *flagIt != *flagListIt; ++flagIt );
				if ( flagListIt == flagList.end() )
				{
					_destSD = ( *_clientIt )->getSocket();
					_answer = std::string( "Voir code erreur channel incconu. :flag " + *flagIt ) + " isn t flag of user Mode\r\n"; //A VOIR FORMATAGE CODE ERREUR FLAGMOD INNEXISTANT
					_request->clear();
					sender();
					throw( IRCErr( "unvalid flag" ) );
				}
			}
		}

		void	wrongFlag( void )
		{
			_destSD = ( *_clientIt )->getSocket();
			_answer = std::string( "Voir code 472 :flag: '" + _modsIt->arg ) + "' is unvalid flag Mode\r\n"; //A VOIR FORMATAGE CODE ERREUR FLAGMOD INNEXISTANT
			_answer = ":*." + _selfIP + " 501 :Unknown " + _modsIt->arg + "\r\n";
			sender();
		}

		void	execMode( void )
		{
			std::cout << "CHANNEL MODE start" << std::endl;
			for ( _modsIt = _mods.begin(); _modsIt != _mods.end(); ++_modsIt )
			{
				try
				{ ( this->*_modsIt->fctn )(); }
				catch( IRCErr const &err )
				{ std::cerr << err.getError() << std::endl; }
			}
			std::cout << "CHANNEL MODE exit" << std::endl;
		}

		void	setListFlagCmdU( void )
		{
			_mods.clear();
			for( strIt flagIt = _flag.begin(); flagIt != _flag.end(); ++flagIt )
			{
				_mods.push_back( Mode() );
				_modsIt = --( _mods.end() );
				listPairM::iterator	_listPairIt;
				for ( _listPairIt = _listFctU.begin(); _listPairIt != _listFctU.end() && _listPairIt->first != *flagIt; ++_listPairIt );
				if ( _listPairIt != _listFctU.end() )
				{
					_modsIt->client = _target;
					_modsIt->fctn = _listPairIt->second.first;
					if ( _listPairIt->second.second )
						( this->*_listPairIt->second.second )();
				}
				else
				{
					_modsIt->fctn = &IRCData::wrongFlag;
					_modsIt->arg = "user MODE " + *flagIt;
				}
			}
		}

		void	setListFlagCmdC( channelIterator &chanIt )
		{
			_flop = 0;
			_mods.clear();
			
			for( strIt flagIt = _flag.begin(); flagIt != _flag.end(); )
			{
				_flop = *( flagIt++ );
				for( ; flagIt != _flag.end() && *flagIt != '+' && *flagIt != '-'; ++flagIt )
				{
					_mods.push_back( Mode() );
					_modsIt = --( _mods.end() );
					_modsIt->flop = _flop;
					listPairM::iterator	_listPairIt;
					for ( _listPairIt = _listFctC.begin(); _listPairIt != _listFctC.end() && _listPairIt->first != *flagIt; ++_listPairIt );
					if ( _listPairIt != _listFctC.end() )
					{
						_modsIt->chanIt = chanIt;
						_modsIt->fctn = _listPairIt->second.first;
						if ( _listPairIt->second.second )
							( this->*_listPairIt->second.second )();
					}
					else
					{
						_modsIt->fctn = &IRCData::wrongFlag;
						_modsIt->arg = "channel MODE " + _flop + *flagIt;
					}
				}
			}
		}

		void	USERMODE( void )
		{
			setListFlagCmdU();
		}

		void	CHANMODE( void )
		{
			channelIterator chanIt = isChannel( _target );
			if ( chanIt == _channels.end() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "Voir code erreur channel incconu\r\n"; //A VOIR FORMATAGE CODE ERREUR CHANNEL INNEXISTANT CMD MODE
				_request->clear();
				sender();
				throw( IRCErr( "unvalid flag" ) );
			}
			if ( chanIt->isOps( ( *_clientIt )->getUser() ) == chanIt->getOps()->end() )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer = "Voir code erreur pas operateur du channel\r\n"; //A VOIR FORMATAGE CODE ERREUR CHANNEL INNEXISTANT CMD MODE
				_request->clear();
				sender();
				throw( IRCErr( "Not channel operator" ) );
			}
			setListFlagCmdC( chanIt ) ;
		}

		void	MODE( void )
		{
			strIt			flopIt;
			_flag.clear();
			_target = getArg();
			_flag = getArg();
			if ( !_target.size() || ( _target[0] != '+' && _target[0] != '-' ) )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer =  ":" + _selfIP + " 461 " + ( *_clientIt )->getNick()  + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " MODE : <channel|user> target forgotten\r\n";
				sender();
				throw( IRCErr( "MODE : <channel|user> target forgotten" ) );
			}
			if ( !_flag.size() || ( _flag[0] != '+' && _flag[0] != '-' ) )
			{
				_destSD = ( *_clientIt )->getSocket();
				_answer =  ":" + _selfIP + " 400 " + ( *_clientIt )->getNick()  + "!~" + ( *_clientIt )->getUser() + "@" + ( *_clientIt )->getClIp() + " MODE :operator [+|-] for flag Mode forgotten\r\n";
				sender();
				throw( IRCErr( "MODE :operator [+|-] for flag Mode forgotten" ) );
			}
			if ( _target[0] == '#' )
				CHANMODE();
			else
				USERMODE();
			execMode();
		}
};