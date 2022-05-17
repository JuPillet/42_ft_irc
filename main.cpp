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
		std::exit( EXIT_FAILURE );
	}

	while ( true )
	{
		server.addClearedMasterSocket(); //OK
		try
		{ server.activityListener(); }
		catch ( IRCErr const &e )
		{
//			std::cerr << err.getError() << std::endl;
			e.getError().size();
			continue;
		}
		try
		{
			if ( FD_ISSET( server.getMasterSocket(), server.getPtrFds() ) )
				server.newClient();
		}
		catch ( IRCErr const &err )
		{ std::cerr << err.getError() << std::endl; }
		try { server.IOListener(); }
		catch ( IRCErr const &err )
		{ std::cerr << err.getError() << std::endl; }
	}

	return 0;
}
