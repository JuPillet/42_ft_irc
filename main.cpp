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
		server.addClearedMasterSocket(); //OK
		try
		{ server.activityListener(); }
		catch ( IRCErr const &err )
		{ std::cerr << err.getError() << std::endl; }
		try
		{
			if ( FD_ISSET( server.getMasterSocket(), const_cast<fd_set*>( server.getPTReadFds() ) ) )
			{
				server.newClient();
				std::cout << "_new_socket accepted " << std::endl;
			}
		}
		catch ( IRCErr const &err )
		{ std::cerr << err.getError() << std::endl; }
		try
		{ 		server.IOListener(); }
		catch ( IRCErr const &err )
		{ std::cerr << err.getError() << std::endl; }
	}

	return 0;
}
