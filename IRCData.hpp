#pragma once
#include <iostream>
#include <string>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <list>
#include "IRCErr.hpp"

class User
{
	std::string		_pass;
	int				_client_socket;
	std::string		_nick;
	std::string		_channel;
	User( User const &src );
	User	&operator=( User const &src ) {
		_pass = src._pass;
		_client_socket = src._client_socket;
		_nick = src._nick;
		_channel = src._channel;
		return *this;
	}
	public:
							User( void ):_pass(), _client_socket(0), _nick(), _channel() {};
							User( int _new_socket ):_pass(), _client_socket( _new_socket ), _nick(), _channel() {};
							~User( void ) {
								delete this;
							}
		void				setPass( std::string const &pass ) { _pass = pass; }
		std::string const	&getPass( void ) const { return _pass; }
		void				setClientSocket( int new_socket ) { _client_socket = new_socket; }
		int					getClientSocket( void ) { return _client_socket; }

};

class IRCData
{
	int									_port;
	std::string							_pass;
	int									_opt;
	int									_master_socket, _addrlen, _new_socket,
										_activity, _valread, _sd, _index;
	int									_max_sd;
	std::list<User*>					_users;
	typedef	std::list<User*>::iterator	userIterator;
	struct sockaddr_in					_address;
	std::string							_buffer;
	char*								_buff[1024];
	fd_set								_readfds;

							IRCData( IRCData &src );
							IRCData	&operator=( IRCData &src );
		void				setAddress( void )
		{
			_address.sin_family = AF_INET;
			_address.sin_addr.s_addr = INADDR_ANY;
			_address.sin_port = htons( _port );
		}

		void				buffBuffer( void ) {
			_buffer.clear();
			while ( ( _valread = read( _sd , _buff, 1024) ) > 0)
				_buffer.append( reinterpret_cast<char *>(_buff), _valread );
		}

		void				socketAcceptator( void ){
			if ( _new_socket = accept( _master_socket, reinterpret_cast<struct sockaddr *>(&_address), reinterpret_cast<socklen_t *>( &_addrlen ) ) < 0 )
				throw IRCErr( "accept" );
			else
			{
				std::cout << "New connection , socket fd is " << _new_socket << ", ip is : " << inet_ntoa( _address.sin_addr ) << ", port : " << ntohs( _address.sin_port ) << std::endl;
				try
				{
					if( send( _new_socket, "ECHO Daemon v1.0 \r\n", strlen( "ECHO Daemon v1.0 \r\n" ), 0) != strlen( "ECHO Daemon v1.0 \r\n" ) )
						throw IRCErr( "send" );
				}
				catch ( IRCErr const &err )
				{ std::cout << err.getError() << std::endl; }
				std::cerr << "Welcome message sent successfully" << std::endl;
			}
		}

		void				addNewSocket( void )
		{
			_users.push_back( new User( _new_socket ) );
			std::cout << "Adding to list of sockets as " << _index << std::endl;
		}

		void				closeEraseDeleteUser( userIterator userIt )
		{
			//Somebody disconnected , get his details and print
			getpeername( _sd , (struct sockaddr*)&_address, (socklen_t*)&_addrlen ); 
			std::cout << "Host disconnected , ip " << inet_ntoa( _address.sin_addr ) << ", port " << ntohs( _address.sin_port ) << std::endl;
	
			//Close the socket and mark as 0 in list for reuse 
			close( _sd );
			(*userIt)->~User();
			_users.erase( userIt );
		}
	public:
							IRCData( void ):_port(), _pass(), _opt(), _master_socket(), _addrlen(), _new_socket(), _activity(), _valread(),
							_sd(), _max_sd(), _users(), _address(), _buffer(), _readfds() { return ; }
							~IRCData( void ) { _users.erase( _users.begin(), _users.end() ); }
		struct sockaddr_in const &getAddress( void ) const { return _address; }
		void				init( const int av, std::string port, const std::string &password )
		{
			if( av != 3 )
				throw( IRCErr( "server need 2 arguments : port and password." ) );

			size_t *lastchar;
			_port = std::stoi( port, lastchar );
			if ( *lastchar || _port < 0 || _port > 65535 )
				throw( IRCErr( "Bad port value : enter port to 0 at 65 535" ) );

			_pass = password;

			if ( ( _master_socket = socket(AF_INET , SOCK_STREAM , 0) ) == 0 )
				throw( IRCErr( "socket failed" ) );

			if ( setsockopt( _master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&_opt, sizeof(_opt) ) < 0 );
				throw( IRCErr( "setsock_opt" ) );

			setAddress();

			if ( bind( _master_socket, ( struct sockaddr *)&_address, sizeof(_address) ) < 0 )
				throw( IRCErr( "bind failed" ) );

			if ( listen( _master_socket, 3 ) < 0)  
				throw( IRCErr( "listen" ) );

			_addrlen = sizeof(_address);

			std::cout << "Waiting for connections ..." << std::endl;
		}

		void				addClearedMasterSocket( void ) {
			FD_ZERO(&_readfds);
			FD_SET( _master_socket, &_readfds);
			_max_sd = _master_socket;
			for ( userIterator userIt = _users.begin(); userIt != _users.end(); ++userIt )
			{
				_sd = (*userIt)->getClientSocket();

				//if valid socket descriptor then add to read list 
				if( _sd > 0 )
					FD_SET( _sd, &_readfds );
				 
				//highest file descriptor number, need it for the select function 
				if( _sd > _max_sd)  
					_max_sd = _sd;
			}
		}

		void				activityListener( void ) {
			_activity = select( _max_sd + 1 , &_readfds , NULL , NULL , NULL);  

			if ( ( _activity < 0 ) && ( errno != EINTR))  
				throw IRCErr( "select error" );
		}

		void				connectionListener( void ){
			if ( FD_ISSET( _master_socket, &_readfds ) )
			{
				try { socketAcceptator(); }
				catch( IRCErr const &err )
				{
					std::cerr << err.getError() << std::endl;
					exit( EXIT_FAILURE );
				}
				addNewSocket();
			}
		}


		void				IOListener( void )
		{
			for ( userIterator userIt = _users.begin(); userIt != _users.end(); ++userIt )
			{  
				_sd = (*userIt)->getClientSocket();
				if ( FD_ISSET( _sd , &_readfds ) )  
				{
					//Check if it was for closing , and also read the 
					//incoming message
					buffBuffer();
					if ( _buffer.length() == 0 )
						closeEraseDeleteUser( userIt );
					else if( (*userIt)->getPass() != _pass )
					{
						if ( _buffer.compare( 0, 6, "/pass " ) || _buffer.compare( 0, 6, "/PASS " ) )
							send( _sd, "Identify yourself with the password before do other things", 55, 0 );
					}
					//Echo back the message that came in
					else 
					{
						//set the string terminating NULL byte on the end 
						//of the data read 
						send( _sd, _buffer.c_str(), _buffer.length(), 0 );  
					}  
				}  
			}
		}
};