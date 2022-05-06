#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include "IRCErr.hpp"

class IRCData
{
    int					_port;
    std::string			_password;
  	int					_sockfd;
    int					_new_sock;
    int					_opt;
	struct sockaddr_in	_address;
	int					_addrlen;
	char				_clients[1024];
							IRCData( IRCData &src );
							IRCData	&operator=( IRCData &src );
    public:
							IRCData( void ):_port(), _password(), _sockfd(), _new_sock(), _opt(), _address(), _addrlen(), _clients( { 0 } ) { return ; }
		void				init( const int av, const int port, const std::string &password )
		{
			if( av < 2 || av > 3 )
				throw( IRCErr( "server need 2 argument : port and password (optional)." ) );
			_port = port;
			_password = password;
			_address.sin_addr.s_addr = htonl( INADDR_ANY );
			_address.sin_family = AF_INET;
			_address.sin_port = htons(4242);
			_sockfd = socket(AF_INET, SOCK_STREAM, 0);
			setsockopt( _sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &_opt, sizeof( _opt ) );
			bind( _sockfd, ( struct sockaddr* )&_address, sizeof( _address ) );
			listen( _sockfd, 1 );
		}
		sockaddr_in const	&getAddress( void ) { return _address; }
		void				setAddressLen( void ) { _addrlen = sizeof( getAddress() ); }
		void				setNewSock( void )
		{ _new_sock = accept( _sockfd, ( struct sockaddr * )&_address, ( socklen_t * )&_addrlen); }
		void				readAndPrint( void ) {
			std::cout << _clients << std::endl;
			read( _new_sock, _clients, 1024);
			send( _new_sock, msg, strlen(msg), 0);
		}
};