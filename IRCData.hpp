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
#include "IRCerr.hpp"
#include "Client.hpp"

class IRCData
{
	int										_port;
	std::string								_pass;
	int										_opt;
	int										_master_socket, _addrlen, _new_socket,
											_activity, _sd, _index;
	int										_max_sd;
	Client*									_clientTmp;
	std::list<Client*>						_clients;
	typedef	std::list<Client*>::iterator	clientIterator;
	std::string								_selfIP;
	struct sockaddr_in						_address;
	std::string								_request;
	std::string								_answer;
	char									_buff[1024];
	fd_set									_readfds;

							IRCData( IRCData &src );
							IRCData	&operator=( IRCData &src );
		void				setAddress( void )
		{
			_address.sin_family = AF_INET;
			_address.sin_addr.s_addr = INADDR_ANY;
			_address.sin_port = htons( _port );
		}
		std::string welcome( void )
		{
			return ":" + _selfIP + " 001 " + _clientTmp->getNick() + " :Welcome to the IRC_QJ_Server "
			+ _clientTmp->getNick() + "!" + _clientTmp->getUser() + "@" + inet_ntoa( _address.sin_addr ) + "\r\n";
		}
		std::string pong( void )
		{ return ":" + _selfIP + " PONG " + _selfIP + " :" + inet_ntoa( _address.sin_addr ) + "\r\n"; }
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

		void				newClient( void )
		{
			_clientTmp = 0;
			receveMessage();
			std::cout << _request << std::endl;
			_request.erase( 0 , _request.find( '\n' ) + 1 );
			if ( !_request.size() )
				receveMessage();
			std::string pass( _request, 5, _request.find( "\r\n" ) - 5 );
			_request.erase( 0 , _request.find( '\n' ) + 1 );
//			receveMessage();
			std::string nick( _request, 5, _request.find( "\r\n" ) - 5 );
			_request.erase( 0 , _request.find( '\n' ) + 1 );
//			receveMessage();
			std::string user( _request, 5, _request.find( "\r\n" ) - 5 );
			_request.erase( 0 , _request.find( '\n' ) + 1 );
			std::cout << "buffer: " << std::endl << _request;
			std::cout << "pwd: " << pass << std::endl;
			std::cout << "nick: " << nick << std::endl;
			std::cout << "user: " << user << std::endl;
			if ( pass != _pass )
			{
				send( _new_socket, "Bad password\r\n", 15, 0 );
				throw IRCErr( "bad password" );
			}
			else
				_clientTmp = new Client( _new_socket, pass, nick, user );
			std::cout << "Adding to list of sockets as " << _index << std::endl;
		}

		void				connectionAcceptator( void )
		{
			if ( ( _new_socket = accept( _master_socket, reinterpret_cast<struct sockaddr *>( &_address ), reinterpret_cast<socklen_t *>( &_addrlen ) ) ) < 0 )
				throw IRCErr( "accept" );
			_sd = _new_socket;
			std::cout << "New connection , socket fd is " << _new_socket << ", ip is : " << inet_ntoa( _address.sin_addr ) << ", port : " << ntohs( _address.sin_port ) << std::endl;
			newClient();
			_answer = welcome();
			if( send( _new_socket, _answer.c_str(), _answer.size(), 0 ) != _answer.length() )
				throw IRCErr( "send" );
			_clients.push_back( _clientTmp );
			std::cout << "Welcome message sent successfully" << std::endl;
		}

		void				closeEraseDeleteClient( clientIterator clientIt )
		{
			//Somebody disconnected , get his details and print
			getpeername( _sd , reinterpret_cast<struct sockaddr *>( &_address ), reinterpret_cast<socklen_t *>( &_addrlen ) ); 
			std::cout << "Host disconnected , ip " << inet_ntoa( _address.sin_addr ) << ", port " << ntohs( _address.sin_port ) << std::endl;
	
			//Close the socket and mark as 0 in list for reuse
			close( _sd );
			delete *clientIt;
			_clients.erase( clientIt );
		}
	public:
							IRCData( void ):_port(), _pass(), _opt(), _master_socket(), _addrlen(), _new_socket(), _activity(),
							_sd(), _max_sd(), _clients(), _address(), _request(), _readfds()
							{ _clients.clear(); }
							~IRCData( void ) {
								for ( clientIterator userIt = _clients.begin(); userIt != _clients.end(); ++userIt )
									delete ( *userIt );
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
		void				init( std::string port, std::string password )
		{
			size_t			lastchar;
			char			selfhost[256];
			struct hostent	*host_entry;

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
			for ( clientIterator clientIt = _clients.begin(); clientIt != _clients.end(); ++clientIt )
			{
				_sd = ( *clientIt )->getSocket();

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
				if ( _clientTmp )
					delete _clientTmp;
			}
		}


		void				IOListener( void )
		{
			for ( clientIterator clientIt = _clients.begin(); clientIt != _clients.end(); ++clientIt )
			{  
				_sd = ( *clientIt )->getSocket();
				if ( FD_ISSET( _sd , &_readfds ) )  
				{
					//Check if it was for closing , and also read the 
					//incoming message
					receveMessage();
					if ( _request.length() == 0 )
						closeEraseDeleteClient( clientIt );
//					else
//					{
//						_answer = pong();
//						send( _sd, reinterpret_cast<const char *>( _answer.c_str() ), _answer.length(), 0 );
//					}
				}  
			}
		}
};