#include "IRCData.hpp"

int main(int ac, char **av)
{
	IRCData server;

	try
	{
		server.nbArgs( ac );
		server.init( av[1], av[2] );
	}
	catch ( IRCErr const &err )
	{
		std::cerr << err.getError() << std::endl;
		std::exit( EXIT_FAILURE ) ;
	}

	while ( true )
	{
		server.addClearedMasterSocket();
		try
		{
			server.activityListener();
			if ( FD_ISSET( server.getMasterSocket(), const_cast<fd_set*>( server.getPTReadFds() ) ) )
				server.connectionListener();
			else
				server.IOListener();
		}
		catch ( IRCErr const &err )
		{ std::cerr << err.getError() << std::endl; }
	}

	return 0;
}
