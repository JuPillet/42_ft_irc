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
#include "Client.hpp"

class IRCData
{
/////	Server Info /////
	std::string								_selfIP;
	int										_port;
	std::string								_pass;
/////	PtrFctn /////
	typedef									void ( IRCData::*ptrfct )( void );
	typedef std::pair<std::string, IRCData::ptrfct>	pairKV;
	typedef std::list<pairKV>				listPair;
	listPair								_listFctn;
/////	Socket Info /////
	int										_opt;
	int										_master_socket, _addrlen, _new_socket,
											_activity, _sd, _index;
	int										_max_sd;
	struct sockaddr_in						_address;
/////	Request Operation /////
	std::string								_request;
	std::string								_cmd;
	std::string								_answer;
	char									_buff[1024];
	fd_set									_readfds;
	std::string 							_dest;
/////	Client Info /////
//	Client									_clientTmp;
	std::list<Client>						_clients;
	typedef	std::list<Client>::iterator	clientIterator;
	clientIterator							_clientIt;
	std::string								_passTmp, _nickTmp, _userTmp, _channelTmp;
	std::string 							_rejectChar;

	
							IRCData( IRCData &src );
							IRCData	&operator=( IRCData &src );

		void				CAPLS( void )
		{ _request.erase( 0 , _request.find( '\n' ) + 1 ); }

		void				checkPass( void )
		{
			if ( _clientIt->getPass() != _pass )
			{
				send( _new_socket, "Bad password\r\n", 15, 0 );
				throw IRCErr( "bad password" );
			}
		}

		void				PASS( void )
		{
			_clientIt->setPass( _passTmp );
			checkPass();
		}
		void				NICK( void )
		{
			clientIterator	tmpIt = _clients.begin();
			for ( std::string::iterator rejectIt = _rejectChar.begin(); rejectIt != _rejectChar.end(); ++rejectIt ){
				for ( std::string::iterator nickIt = _nickTmp.begin(); nickIt != _nickTmp.end(); ++nickIt ){
					if ( *nickIt == *rejectIt ){
						send( _sd, "Nickerror\r\n", 12, 0 );
						throw IRCErr( "Nick format" );
			}	}	}
			for ( ; tmpIt != _clients.end(); ++tmpIt)
			{
				if ( tmpIt->getNick() == _nickTmp )
				{
					send( _sd, "Nick already in use\r\n", 22, 0 ); // A verifier a deux, si j essaie de prendre le nick d un autre
					throw IRCErr( "Nick already in use" );
				}
			}
			_clientIt->setNick(_nickTmp);
		}

		void				USER( void )
		{
			clientIterator							tmpIt = _clients.begin();
			for ( std::string::iterator rejectIt = _rejectChar.begin(); rejectIt != _rejectChar.end(); ++rejectIt ){
				for ( std::string::iterator userIt = _userTmp.begin(); userIt != _userTmp.end(); ++userIt ){
					if ( *userIt == *rejectIt ){
						send( _sd, "Usererror\r\n", 12, 0 );
						throw IRCErr( "User format" );
			}	}	}
			_clientIt->setUser(_userTmp);
		}

		void				JOIN( void )
		{
			std::list<std::string>::const_iterator chanIt;
			for ( chanIt = _clientIt->getChannels().begin(); *chanIt != _channelTmp && chanIt != _clientIt->getChannels().end(); ++chanIt );
			if ( chanIt != _clientIt->getChannels().end() )
			{
				send( _sd, "a voir!!!", 10, 0 ); // A VOIR!!!
				throw( IRCErr( "Is already in the channel" ) );
			}
			_clientIt->setChannel( _channelTmp );
		}

		void				OPENMSG( void )
		{
			clientIterator							tmpIt = _clients.begin();
			while (tmpIt != _clients.end())
			{
		//		if ( tmpIt->getChannel() == _clientIt->getChannel() )
		//			send( tmpIt->getSocket(), reinterpret_cast<const char *>( _buff ), std::strlen(_buff), 0 );
				tmpIt++;
			}
		}

		void				PRIVMSG( void )
		{
			clientIterator							tmpIt = _clients.begin();

			if (_clientIt->getPass() != _pass)
				return ;
			while (tmpIt != _clients.end())
			{
				if ( tmpIt->getNick() == _nickTmp )
				{
					send( tmpIt->getSocket(), reinterpret_cast<const char *>( _buff ), std::strlen(_buff), 0 );
					break;
				}
				tmpIt++;
			}
		}

		void				MSG( void )
		{
			std::string::iterator strIt;
			for (strIt = _request.begin(); strIt != _request.end() && *strIt != ' '; strIt++);
			_dest = _request.substr( 0, _request.begin() - strIt );

			_request.erase(_request.begin(), strIt + 1);
			if (_dest[0] == '#')
				OPENMSG();
			else
				PRIVMSG();
		}

		void				setAddress( void )
		{
			_address.sin_family = AF_INET;
			_address.sin_addr.s_addr = INADDR_ANY;
			_address.sin_port = htons( _port );
		}

		void sender ( void )
		{
			if ( send(_sd, _answer.c_str(), _answer.length(), 0) )
			{
				_answer.clear();
				throw( IRCErr( "send" ) );
			}
			_answer.clear();
		}

		std::string WELCOME( void )
		{
			_answer = ":" + _selfIP + " 001 " + _clientIt->getNick() + " :Welcome to the IRC_QJ_Server "
			+ _clientIt->getNick() + "!" + _clientIt->getUser() + "@" + inet_ntoa( _address.sin_addr ) + "\r\n";
			sender();
		}

		std::string PONG( void )
		{
			_answer = ":" + _selfIP + " PONG " + _selfIP + " :" + inet_ntoa( _address.sin_addr ) + "\r\n"; 
			sender();
		}

		void				receveMessage( void ) {
			std::cout << "message start" << std::endl;
			int readvalue;
			_request.clear();
			for ( int index = 0; index != 1024; ++index )
				_buff[index] = 0;
//			do
//			{
//				std::cout << "prev recv" << std::endl;
				readvalue = recv( _sd , _buff, 1024, 0 );
//				std::cout << "next recv" << std::endl;
				_request.append( reinterpret_cast<char *>( _buff ), readvalue );
//			} while ( readvalue == 1024 );
				std::cout << readvalue << " : " << _request.length() << " : " << std::endl << _request << std::endl;
//			_request.pop_back();
			std::cout << "message exit" << std::endl;
		}

		void				formatRequest( void )
		{
			if ( *_request.c_str() == '/' )
			{
				std::string::iterator _requestIt;
				int index = _request.find( ' ' );
				_request.erase( _request.begin() );
				if ( index >= _request.length() )
					index = _request.length();
				for ( _requestIt = _request.begin(); _requestIt != _request.begin() + index; ++_requestIt );
					( *_requestIt ) = std::toupper( *_requestIt );
				_cmd = _request.substr( 0, index );
				_request.erase( 0, index );
			}
		}

		void				newClient( void )
		{
			receveMessage();

			std::cout << _request << std::endl;
			_request.erase( 0 , _request.find( '\n' ) + 1 );
			if ( !_request.size() )
				receveMessage();
			( _request, 5, _request.find( "\r\n" ) - 5 );
			_request.erase( 0 , _request.find( '\n' ) + 1 );
//			receveMessage();
			std::string nick( _request, 5, _request.find( "\r\n" ) - 5 );
			_request.erase( 0 , _request.find( '\n' ) + 1 );
//			receveMessage();
			std::string user( _request, 5, _request.find( "\r\n" ) - 5 );
			_request.erase( 0 , _request.find( '\n' ) + 1 );
			std::cout << "Adding to list of sockets as " << _index << std::endl;
		}

		void				connectionAcceptator( void )
		{
			if ( ( _new_socket = accept( _master_socket, reinterpret_cast<struct sockaddr *>( &_address ), reinterpret_cast<socklen_t *>( &_addrlen ) ) ) < 0 )
				throw IRCErr( "accept" );
			_sd = _new_socket;
			_clients.push_back( Client( _new_socket ) );
			_clientIt = ( _clients.end()-- );
			std::cout << "New connection , socket fd is " << _new_socket << ", ip is : " << inet_ntoa( _address.sin_addr ) << ", port : " << ntohs( _address.sin_port ) << std::endl;
			newClient();
			WELCOME();
			std::cout << "Welcome message sent successfully" << std::endl;
		}

		void				closeEraseDeleteClient( void )
		{
			//Somebody disconnected , get his details and print
			getpeername( _sd , reinterpret_cast<struct sockaddr *>( &_address ), reinterpret_cast<socklen_t *>( &_addrlen ) ); 
			std::cout << "Host disconnected , ip " << inet_ntoa( _address.sin_addr ) << ", port " << ntohs( _address.sin_port ) << std::endl;
	
			//Close the socket and mark as 0 in list for reuse
			close( _sd );
			_clients.erase( _clientIt );
		}
	public:
							IRCData( void ):_port(), _pass(), _opt(), _master_socket(), _addrlen(), _new_socket(), _activity(),
							_sd(), _max_sd(), _clients(), _address(), _request(), _readfds()
							{ _clients.clear(); }
							~IRCData( void ) {
//								for ( clientIterator userIt = _clients.begin(); userIt != _clients.end(); ++userIt )
//									delete ( *userIt );
								_clients.erase( _clients.begin(), _clients.end() );
							}
		int	const			getMasterSocket( void ) const { return _master_socket; }
		fd_set	const		getReadFds( void ) const { return _readfds; }
		fd_set	const		*getPTReadFds( void ) const { return &_readfds; }
		struct sockaddr_in const &getAddress( void ) const { return _address; }
		void				nbArgs( const int ac )
		{
			if( ac != 3 )
				throw( IRCErr( "Server need 2 arguments : port and password." ) );
		}

		void initFct ()
		{
			_listFctn.push_back( pairKV( "NICK", NICK ) );
			_listFctn.push_back( pairKV( "USER", USER ) );
			_listFctn.push_back( pairKV( "JOIN", JOIN ) );
			_listFctn.push_back( pairKV( "MSG", MSG ) );
		}


		void				init( std::string port, std::string password )
		{
			size_t			lastchar;
			char			selfhost[256];
			struct hostent	*host_entry;
	
			_rejectChar = "~!@#$%&*()+=:;\"\',<.>?/";
			_port = std::stoi( port, &lastchar );
			std::cout << port[lastchar] << std::endl;
			if ( port[lastchar] || _port < 0 || _port > 65535 )
				throw( IRCErr( "Bad port value : enter port to 0 at 65 535" ) );

			_pass = password;

			if ( ( _master_socket = socket( AF_INET , SOCK_STREAM , 0 ) ) == 0 )
				throw( IRCErr( "socket failed" ) );

			if ( setsockopt( _master_socket, SOL_SOCKET, SO_REUSEADDR, ( char * )&_opt, sizeof( _opt ) ) < 0 )
				throw( IRCErr( "setsock_opt" ) );

			setAddress();

			if ( bind( _master_socket, ( struct sockaddr * )&_address, sizeof( _address ) ) < 0 )
				throw( IRCErr( "bind failed" ) );

			if ( listen( _master_socket, 3 ) < 0 )  
				throw( IRCErr( "listen" ) );

			_addrlen = sizeof( _address );

			gethostname( selfhost, sizeof( selfhost ) );
			host_entry = gethostbyname(selfhost);
			_selfIP = inet_ntoa( *( reinterpret_cast<struct in_addr*>( host_entry->h_addr_list[0] ) ) );

			std::cout << "Waiting for connections ..." << std::endl;
		}

		void				addClearedMasterSocket( void ) {
			FD_ZERO( &_readfds );
			FD_SET( _master_socket, &_readfds );
			_max_sd = _master_socket;
			for ( _clientIt = _clients.begin(); _clientIt != _clients.end(); ++_clientIt )
			{
				_sd = _clientIt->getSocket();

				//if valid socket descriptor then add to read list 
				FD_SET( _sd, &_readfds );
				 
				//highest file descriptor number, need it for the select function 
				if( _sd > _max_sd )  
					_max_sd = _sd;
			}
		}

		void				activityListener( void ) {
			_activity = select( _max_sd + 1 , &_readfds , NULL , NULL , NULL );  

			if ( ( _activity < 0 ) && ( errno != EINTR ) )  
				throw IRCErr( "select error" );
		}

		void				connectionListener( void ){
			try { connectionAcceptator(); }
			catch( IRCErr const &err )
			{ 
				if ( _new_socket != -1 )
					close( _new_socket );
				std::cerr << err.getError() << std::endl;
//				if ( _clientTmp )
//					delete _clientTmp;
			}
		}


		void				findFct( void )
		{
			listPair::iterator	listPairIt;

			for (listPairIt = _listFctn.begin(); listPairIt->first != _cmd && listPairIt != _listFctn.end(); ++listPairIt);
			if ( listPairIt != _listFctn.end() )
				listPairIt->second;
			else
				MSG();
		}

		void				IOListener( void )
		{
			for ( _clientIt = _clients.begin(); _clientIt != _clients.end(); ++_clientIt )
			{  
				_sd = _clientIt->getSocket();
				if ( FD_ISSET( _sd , &_readfds ) )  
				{
					//Check if it was for closing , and also read the 
					//incoming message
					try
					{
						receveMessage();
						if ( _request.length() == 0 )
							closeEraseDeleteClient();
						else
						{
							if ( _cmd == "PASS" )
								PASS();
							else
							{
								checkPass();
								findFct();
							}
						}
					}
					catch ( IRCErr const &err)
					{ std::cout << err.getError() << std::endl; }
				}  
			}
		}
};