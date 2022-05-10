#include <sys/select.h>
#include <unistd.h>
#include <sys/select.h>
#include "IRCData.hpp"

int main(int ac, char **av)
{
	IRCData server;

	try
	{
		server.nbArgs( ac );
		server.init( av[1], av[2] ); }
	catch ( IRCErr const &err )
	{
		std::cerr << err.getError() << std::endl;
		std::exit( EXIT_FAILURE ) ;
	}

	while ( true )
	{
		server.addClearedMasterSocket();
		try
		{ server.activityListener(); }
		catch ( IRCErr const &err )
		{ std::cerr << err.getError() << std::endl; }
		server.connectionListener();
		server.IOListener();
	}

	return 0;
}