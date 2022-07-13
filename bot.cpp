#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstring>
#include <netdb.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include "IRCutils.hpp"
#include "IRCmsg.hpp"

void	initBot( std::string servIP, int port, struct sockaddr_in *servAddr, int *sockd )
{
	struct addrinfo hints, *servInfo;

	if ( ( *sockd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
		throw std::string( "Error: socket() failed" );

	servAddr->sin_family = AF_INET;
	servAddr->sin_port = htons( port );
	memset( &hints, 0, sizeof hints );

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo( servIP.c_str(), 0, &hints ,&servInfo );

	servAddr->sin_addr = ( ( struct sockaddr_in * ) servInfo->ai_addr )->sin_addr;
}

void	botExit( int socket )
{
	static int sd;

	if ( socket )
		sd = socket;
	else
	{
		std::string deconnection = "/QUIT\r\n";
		send(sd, deconnection.c_str(), deconnection.size(), 0);
		close( sd );
		std::cout << std::endl << "bot deconnection correctly and down" << std::endl;
		std::exit ( 0 );
	}
}

void	botSignal( int sig )
{
	if (!sig)
	{
		signal( SIGINT, &botSignal );
		signal( SIGQUIT, &botSignal );
	}
	else if ( sig == SIGINT || sig == SIGQUIT )
		botExit( 0 );
}

int main(int ac, char **av)
{
	int sockd = 0;
	try
	{
		char			selfhost[256];
		struct hostent	*bothost;
		if ( ac < 5 || !isdigit( av[1][0] ) )
			throw( std::string( "Please send a server pass, and channel for the bot to join." ) );
		std::string server = av[1]; // network address
		int port = atoi( av[2]); // bot port
		std::string nick = "NICK Awesomebot\r\n"; // NICK raw
		std::string user = "USER Awesomebot * * Awesomebot\r\n"; // USER raw
		std::string pass = "PASS " + std::string( av[3] ) + "\r\n";
		std::string join = "JOIN #" + std::string( av[4] ) + "\r\n";
		std::string msg = "PRIVMSG #" + std::string( av[4] ) + " :My name is Awesomebot, Here the secret to my success : https://www.youtube.com/watch?v=iik25wqIuFo \r\n";
 
		/** Structs that hold the socket information **/

		struct sockaddr_in botAddr, servAddr;
		fd_set _readfds, _writefds, _crntfds;
		int activity;
		int addrlen;
		/** Fill the members of the socket structs required to connect **/

		botAddr.sin_addr.s_addr = INADDR_ANY;
		botAddr.sin_family = AF_INET;
		botAddr.sin_port = htons( port );

		addrlen = sizeof( botAddr );

		gethostname( selfhost, sizeof( selfhost ) );
		bothost = gethostbyname(selfhost);
		std::string botIP = inet_ntoa( *( reinterpret_cast<struct in_addr*>( bothost->h_addr_list[0] ) ) );
		std::cout << botIP << std::endl;
		//close( 4 );
		initBot( server, port, &servAddr, &sockd );

		std::cout << "waiting connection..." << std::endl;
		if ( connect( sockd, ( struct sockaddr * )&servAddr, sizeof( servAddr ) ) == -1 )
			throw( std::string( "connection FAILED..." ) );
		botExit( sockd );
		botSignal( 0 );
		std::string servIP = inet_ntoa( servAddr.sin_addr );

		FD_ZERO( &_crntfds );
		FD_SET( sockd,  &_crntfds );
		int readvalue;
		char buff[1024];
		sleep( 3 );
		if( send(sockd, pass.c_str(), pass.size(), 0) == -1 )
			throw( std::string( "108 connection FAILED..." ) );
		if( send(sockd, nick.c_str(), nick.size(), 0) == -1
			|| ( readvalue = recv( sockd, buff, 1024, 0 ) ) == -1
			|| buff != ":!~@" + servIP + " NICK :Awesomebot\r\n" )
			throw( std::string( "110 connection FAILED..." ) );
		if( send(sockd, user.c_str(), user.size(), 0) == -1 )
			throw( std::string( "112 connection FAILED..." ) );
		if( sleep(3) )
			throw( std::string( "114 connection FAILED..." ) );
		if( send(sockd, join.c_str(), join.size(), 0) == -1 )
			throw( std::string( "116 connection FAILED..." ) );
		_readfds = _writefds = _crntfds;
		activity = select( sockd + 2, &_readfds, &_writefds, NULL, NULL );
		if ( ( activity < 0 ) )  
			throw( std::string( "120 connection FAILED..." ) );

		if ( readvalue == -1 )
			throw( std::string( "125 connection FAILED..." ) );
		readvalue = recv( sockd, buff, 1024, 0 );
		std::string	request( buff );
		std::string test( ":" + servIP + " 001 Awesomebot :Welcome to the IRC_QDJ_Server Awesomebot!Awesomebot@" + botIP + "\r\n" );
		
		if ( request != test )
		{
			std::cout << request <<" :" << request.length()<< std::endl;
			std::cout << test <<" :" << test.length()<< std::endl;
			throw( std::string( "132 connection FAILED..." ) );
		}

		std::cout << "connection SUCCEED!" << std::endl;
		while ( 1 )
		{
			if ( sleep(30) || send( sockd, msg.c_str(), msg.size(), 0) == -1 )
				throw( std::string( "connection LOST..." ) );
		}
	}
	catch ( std::string const &err )
	{
		if ( sockd )
			close( sockd );
		std::cout << err << std::endl;
	}
	return 0;
}