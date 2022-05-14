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
	typedef std::string::iterator			strIt;
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
	int										_destSD;

	
							IRCData( IRCData &src );
							IRCData	&operator=( IRCData &src );

		void				spaceTrimer( void )
		{
			strIt	trimIt;
			for ( trimIt = _request.begin(); trimIt != _request.end() && *trimIt == ' '; ++trimIt );
			_request.erase( 0, trimIt - _request.begin() );
		}

		void				receveRequest( void ) {
			std::cout << "message start" << std::endl;
			int readvalue;
			_request.clear();
			for ( int index = 0; index != 1024; ++index )
				_buff[index] = 0;

			readvalue = recv( _sd , _buff, 1024, 0 );
			_request.append( reinterpret_cast<char *>( _buff ), readvalue );
			std::cout << readvalue << " : " << _request.length() << " : " << std::endl << _request << std::endl;
			std::cout << "message exit" << std::endl;
			spaceTrimer();
		}

		void				getCmd( void )
		{
			strIt	cmdIt;
			for ( cmdIt = _request.begin(); cmdIt != _request.end()
				&& *cmdIt != '\n' && *cmdIt != '\r' && *cmdIt != ' '; ++cmdIt );
			_cmd = std::string( _request, 0, cmdIt - _request.begin() );
			_request.erase( 0, cmdIt - _request.begin() );
			if ( _cmd != "PRIVMSG" )
				spaceTrimer();
			else if( _request.size() > 1 && _request[0] == ' ' )
				_request.erase( 0, 1 );
		}

		void sender ( void )
		{
			if ( send( _destSD, _answer.c_str(), _answer.length(), 0 ) )
			{
				_answer.clear();
				throw( IRCErr( "send" ) );
			}
			_answer.clear();
		}

		void	WELCOME( void )
		{
			if ( _clientIt->getPass() == _pass
				&& _clientIt->getNick().size() && _clientIt->getUser().size() )
			{
				_destSD = _sd;
				_answer = ":" + _selfIP + " 001 " + _clientIt->getNick() + " :Welcome to the IRC_QJ_Server "
				+ _clientIt->getNick() + "!" + _clientIt->getUser() + "@" + inet_ntoa( _address.sin_addr ) + "\r\n";
				sender();
				std::cout << "Welcome message sent successfully" << std::endl;
				_clientIt->setAutentification();
			}
		}

		void				CAPLS( void )
		{ _request.erase( 0 , _request.find( '\n' ) + 1 ); }

		void				checkPass( void )
		{
			if ( _clientIt->getPass() != _pass )
			{
				_answer = ":Bad password\r\n";
				sender();
				throw IRCErr( "Bad password" );
			}
		}

		void				PASS( void )
		{
			_destSD = _sd;
			if ( _clientIt->getAutentification() )
			{
				_answer = ":" + _selfIP + " 462 " + " " + _clientIt->getNick() + " " + ":You may not reregister";
				sender();
				throw IRCErr( _clientIt->getUser() + " try a double registration" );
			}

			strIt	passIt;
			for ( passIt = _request.begin(); passIt != _request.end()
				&& *passIt != '\n' && *passIt != '\r' && *passIt != ' '; ++passIt );
			_passTmp = std::string( _request, 0, passIt - _request.begin() );
			for ( ; passIt != _request.end() && *passIt != '\n'; ++passIt );
			_request.erase( _request.begin(), passIt + 1 );
			spaceTrimer();

			_clientIt->setPass( _passTmp );
			checkPass();
			if ( !_clientIt->getAutentification() )
				WELCOME();
		}

		void				NICK( void )
		{
			checkPass();

			strIt	nickIt;
			_nickTmp.clear();
			for ( nickIt = _request.begin(); nickIt != _request.end()
				&& *nickIt != '\n' && *nickIt != '\r' && *nickIt != ' '; ++nickIt );
			_nickTmp = std::string( _request, 0, nickIt - _request.begin() );
			for ( ; nickIt != _request.end() && *nickIt != '\n'; ++nickIt );
			_request.erase( _request.begin(), nickIt + 1 );
			spaceTrimer();

			clientIterator	tmpIt = _clients.begin();
			for ( strIt rejectIt = _rejectChar.begin(); rejectIt != _rejectChar.end(); ++rejectIt ){
				for ( strIt nickIt = _nickTmp.begin(); nickIt != _nickTmp.end(); ++nickIt ){
					if ( *nickIt == *rejectIt ){
						_destSD = _sd;
						_answer = "Nickerror\r\n";
						sender();
						throw IRCErr( "Nick format" );
			}	}	}

			for ( ; tmpIt != _clients.end(); ++tmpIt )
			{
				if ( tmpIt->getNick() == _nickTmp )
				{
					_destSD = _sd;
					_answer = "Nick already in use\r\n"; // A verifier a deux, si j essaie de prendre le nick d un autre
					sender();
					throw IRCErr( "Nick already in use" );
				}
			}

			_clientIt->setNick( _nickTmp );
			if ( !_clientIt->getAutentification() )
				WELCOME();
		}

		void				USER( void )
		{
			strIt					userIt;
			std::string				hostTmp;
			std::string				servTmp;
			std::string				realNTmp;

			for ( userIt = _request.begin(); userIt != _request.end()
				&& *userIt != '\n' && *userIt != '\r' && *userIt != ' '; ++userIt );
			_userTmp = std::string( _request, 0, userIt - _request.begin() );
			_request.erase( _request.begin(), userIt + 1 );
			for ( userIt = _request.begin(); userIt != _request.end()
				&& *userIt != '\n' && *userIt != '\r' && *userIt != ' '; ++userIt );
			
			for ( ; userIt != _request.end() && *userIt != '\n'; ++userIt );
			_request.erase( _request.begin(), userIt + 1 );

			spaceTrimer();

			clientIterator							tmpIt = _clients.begin();
			for ( strIt rejectIt = _rejectChar.begin(); rejectIt != _rejectChar.end(); ++rejectIt ){
				for ( strIt userIt = _userTmp.begin(); userIt != _userTmp.end(); ++userIt ){
					if ( *userIt == *rejectIt ){
						_destSD = _sd;
						_answer = "Usererror\r\n";
						sender();
						throw IRCErr( "User format" );
			}	}	}
			_clientIt->setUser( _userTmp );
			if ( !_clientIt->getAutentification() )
				WELCOME();
		}

		void				JOIN( void )
		{
			strIt	channelIt;
			for ( channelIt = _request.begin(); channelIt != _request.end()
				&& *channelIt != '\n' && *channelIt != '\r' && *channelIt != ' '; ++channelIt );
			_channelTmp = std::string( _request, 0, channelIt - _request.begin() );
			spaceTrimer();

			std::list<std::string>::const_iterator chanIt;
			for ( chanIt = _clientIt->getChannels().begin(); *chanIt != _channelTmp && chanIt != _clientIt->getChannels().end(); ++chanIt );
			if ( chanIt != _clientIt->getChannels().end() )
			{
				_destSD = _sd;
				_answer = "a voir!!!";
				sender();
				throw( IRCErr( "Is already in the channel" ) );
			}
			_clientIt->setChannel( _channelTmp );
		}

		void				OPENMSG( void )
		{
			clientIterator	clientIt = _clients.begin();
			for ( ; clientIt != _clients.end(); clientIt++ )
			{
				std::list<std::string>::const_iterator channel = clientIt->getChannels().begin();
				for ( ; channel != _clientIt->getChannels().end() && *channel != _dest; channel++ );
				if ( channel != _clientIt->getChannels().end() )
				{
					_destSD = _clientIt->getSocket();
					_answer = "A voir formattage message"; // A voir
					sender(); 
				}
			}
		}

		void				PRIVMSG( void )
		{
			clientIterator							clientIt = _clients.begin();
			for ( ; clientIt != _clients.end() && clientIt->getNick() != _nickTmp; ++clientIt );
			if ( clientIt != _clients.end() )
			{
				_destSD = clientIt->getSocket();
				_answer = " A VOIR formmattage "; // A VOIR
				sender();
			}
		}

		void				MSG( void )
		{
			strIt strIt;
			for ( strIt = _request.begin(); strIt != _request.end() && *strIt != ' '; strIt++ );
			_dest = _request.substr( 0, _request.begin() - strIt );
			_request.erase( 0, strIt - _request.begin() );

			if ( _dest[0] == '#' )
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

		std::string PONG( void )
		{
			_destSD = _sd;
			_answer = ":" + _selfIP + " PONG " + _selfIP + " :" + inet_ntoa( _address.sin_addr ) + "\r\n"; 
			sender();
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
			_listFctn.push_back( pairKV( "PRIVMSG", MSG ) );
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
			host_entry = gethostbyname( selfhost );
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

		void				execFct( void )
		{
			listPair::iterator	listPairIt;

			for ( listPairIt = _listFctn.begin(); listPairIt->first != _cmd && listPairIt != _listFctn.end(); ++listPairIt );
			if ( listPairIt != _listFctn.end() )
				listPairIt->second;
		}

		void				connectionSocket( void )
		{
			if ( ( _new_socket = accept( _master_socket, reinterpret_cast<struct sockaddr *>( &_address ), reinterpret_cast<socklen_t *>( &_addrlen ) ) ) < 0 )
				throw IRCErr( "accept" );
			_sd = _new_socket;
			_clients.push_back( Client( _new_socket ) );
			_clientIt = _clients.end();
			_clientIt--;
			std::cout << "New connection , socket fd is " << _new_socket << ", ip is : " << inet_ntoa( _address.sin_addr ) << ", port : " << ntohs( _address.sin_port ) << std::endl;
		}

		void				newClient( void )
		{
			connectionSocket();
			receveRequest();
			std::cout << _request << std::endl;
			strIt	CAPIt;
			for ( CAPIt = _request.begin(); CAPIt != _request.end()
				&& *CAPIt != '\n' && *CAPIt != '\r' && *CAPIt != ' '; ++CAPIt );
			std::string cap( _request, 0, CAPIt - _request.begin() );
			if ( cap == "CAP" )
			{
				for ( ; CAPIt != _request.end() && *CAPIt != '\n'
						&& *CAPIt != ' '; ++CAPIt );
				_request.erase( 0, CAPIt - _request.begin() );
			}
			spaceTrimer();
			while( _request.size() )
				execFct();
			std::cout << "Adding to list of sockets as " << _index << std::endl;
		}

		void				connectionListener( void ){
			try { newClient(); }
			catch( IRCErr const &err )
			{ 
				if ( _new_socket != -1 )
					close( _new_socket );
				std::cerr << err.getError() << std::endl;
			}
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
						receveRequest();
						if ( _request.length() == 0 )
							closeEraseDeleteClient();
						else
						{
							while ( _request.size() )
							{
								getCmd();
								if ( _cmd == "PASS" )
									PASS();
								else
								{
									checkPass();
									execFct();
								}
							}
						}
					}
					catch ( IRCErr const &err )
					{ std::cout << err.getError() << std::endl; }
				}  
			}
		}
};